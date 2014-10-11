//
// C++ Implementation: NindFile
//
// Description: Les acces basiques a un fichier binaire sequentiel avec possibilites de bufferisation
// Le fichier est ecrit en little-endian 
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindFile.h"
#include <string.h>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindFile with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if NindFile will write on file, false otherwise
//param bufferSize size of writing buffer (0 if just reader) */
NindFile::NindFile(const string &fileName,
                   const bool isWriter):
    m_isWriter(isWriter),
    m_fileName(fileName),
    m_file(0),
    m_wbuffer(0),
    m_wbufferEnd(0),
    m_wPtr(0),
    m_rbuffer(0),
    m_rbufferEnd(0),
    m_rPtr(0),
    m_fileSize(0),
    m_isLittleEndian(false)  
{
    //teste si la plateforme est little endian ou big endian
    const unsigned int testLittleEndian = 13;
    m_isLittleEndian = (testLittleEndian == (*(unsigned char*)&testLittleEndian)) ; // vrai si little endian
    //cout<<"m_isLittleEndian="<<m_isLittleEndian<<endl;
}
////////////////////////////////////////////////////////////
NindFile::~NindFile()
{
    close();
    if (m_wbuffer != 0) {
        delete [] m_wbuffer;
        m_wbuffer = 0;
    }
    if (m_rbuffer != 0) {
        delete [] m_rbuffer;
        m_rbuffer = 0;
    }
}
////////////////////////////////////////////////////////////
//brief Open file associated with. No exceptions are raised.
//param accessMode authorized access modes are "rb", "wb", "ab", "r+b", "w+b", "a+b", "rb+", "wb+", "ab+"
//return true if success, false otherwise */
bool NindFile::open(const string &accessMode)
{
    //ouvre le fichier
    m_file =  fopen(m_fileName.c_str(), accessMode.c_str());
    if (!m_file) return false;
    //memorise sa taille
    fseek(m_file, 0, SEEK_END);
    m_fileSize = ftell(m_file);
    return true;
}
////////////////////////////////////////////////////////////
//brief Close file associated with. Success in any case. */
void NindFile::close()
{
    if (m_file) {
        fclose(m_file);
        m_file = 0;
    }
}
////////////////////////////////////////////////////////////
//brief get current position in file
//return current position in file */
long int NindFile::getPos() const
{
    return ftell(m_file);
}
////////////////////////////////////////////////////////////
//brief get current size of file
//return current size of file */
long int NindFile::getFileSize() const
{
    return m_fileSize;
}
////////////////////////////////////////////////////////////
//brief Set file on relative position
//param offset Number of bytes to offset from origin
//param origin Position from where offset is added :
//SEEK_SET : Beginning of file, SEEK_CUR : Current position of the file pointer, SEEK_END : End of file
//return true if success, false otherwise */
bool NindFile::setPos(const long int offset,
                      const int origin)
{
    return fseek(m_file, offset, origin);
}
////////////////////////////////////////////////////////////
//brief clear input buffer in reading mode and write output buffer in writing mode
//return true if success, false otherwise */
bool NindFile::flush()
{
    return fflush(m_file);
}
////////////////////////////////////////////////////////////
//brief Read a single byte from the file
//return byte value */
unsigned char NindFile::readChar()
    throw(EofException, ReadFileException)
{
    unsigned char byte;
    readBytes(&byte, 1);
    return byte;
}
////////////////////////////////////////////////////////////
//brief Read a 4-bytes integer from the file
//return 4-bytes integer */
unsigned int NindFile::readInt4()
    throw(EofException, ReadFileException)
{
    //un entier, c'est 4 octets sur le fichier
    unsigned char bytes[4];
    readBytes(bytes, 4);
    if (m_isLittleEndian) return *(unsigned int*)bytes;
    return ((bytes[3]*256 + bytes[2])*256 + bytes[1])*256 + bytes[0];
}
////////////////////////////////////////////////////////////
//brief Read a 8-bytes integer from the file
//return 8-bytes integer */
long int NindFile::readInt8()
    throw(EofException, ReadFileException)
{
    //un entier long, c'est 8 octets sur le fichier
    unsigned char bytes[8];
    readBytes(bytes, 8);
    if (m_isLittleEndian) return *(long int*)bytes;
    return ((((((bytes[7]*256 + bytes[6])*256 + bytes[5])*256 + bytes[4])*256 + bytes[3])*256 + bytes[2])*256 + bytes[1])*256 + bytes[0];
}
////////////////////////////////////////////////////////////
//brief Read a string from the file
//param str string where to write read string */
std::string NindFile::readString()
        throw(EofException, ReadFileException)
{
    //lit la longueur de la chaine
    unsigned char len;
    readBytes(&len, 1);
    //lit la chaine
    unsigned char bytes[256];
    readBytes(bytes, (unsigned int)len);
    return string((char*)bytes, len);
}   
////////////////////////////////////////////////////////////
//brief Read bytes from the file into an internal buffer
//param bytesNb size of internal buffer to receive read datas */
void NindFile::readBuffer(const unsigned int bytesNb)
    throw(EofException, ReadFileException, BadAllocException)
{
    //cree le buffer intermediaire de lecture
    if (m_rbuffer != 0) delete [] m_rbuffer;
    m_rbuffer = new (nothrow) unsigned char[bytesNb];
    if (m_rbuffer == 0) throw BadAllocException("in read buffer");
    m_rbufferEnd = m_rbuffer + bytesNb;
    m_rPtr = m_rbuffer;
    //lit le fichier
    readBytes(m_rbuffer, bytesNb);
}
////////////////////////////////////////////////////////////
//brief Get a single byte from internal buffer
//return byte value */
unsigned char NindFile::getChar()
    throw(OutReadBufferException)
{
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (A)");
    return m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get a 4-bytes integer from internal buffer
//return 4-bytes integer */
unsigned int NindFile::getInt4()
    throw(OutReadBufferException)
{
    m_rPtr += 4;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (B)");
    if (m_isLittleEndian) return *(unsigned int*)(m_rPtr-4);
    return ((m_rPtr[-1]*256 + m_rPtr[-2])*256 + m_rPtr[-3])*256 + m_rPtr[-4];
}
////////////////////////////////////////////////////////////
//brief Get a 8-bytes integer from internal buffer
//return 8-bytes integer */
long int NindFile::getInt8()
    throw(OutReadBufferException)
{
    m_rPtr += 8;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (C)");
    if (m_isLittleEndian) return *(long int*)(m_rPtr-8);
    return ((((((m_rPtr[-1]*256 + m_rPtr[-2])*256 + m_rPtr[-3])*256 + m_rPtr[-4])*256 + m_rPtr[-5])*256 + m_rPtr[-6])*256 + m_rPtr[-7])*256 + m_rPtr[-8];
}
////////////////////////////////////////////////////////////
//brief Create an internal buffer for writing
//param bytesNb size of buffer */
void NindFile::createBuffer(const unsigned int bytesNb)
    throw(BadAllocException)
{
    if (m_wbuffer != 0) delete [] m_wbuffer;
    m_wbuffer = new (nothrow) unsigned char[bytesNb];
    if (m_wbuffer == 0) throw BadAllocException("in write buffer");
    m_wbufferEnd = m_wbuffer + bytesNb;
    m_wPtr = m_wbuffer;
}
////////////////////////////////////////////////////////////
//brief Put one byte into the internal buffer
//param value byte value to write */
void NindFile::putChar(const unsigned char value)
    throw(OutWriteBufferException)
{
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (A)");
    m_wPtr[-1] = value;
}
////////////////////////////////////////////////////////////
//brief Put a 4-bytes integer into the internal buffer
//param int4 4-bytes integer to write */
void NindFile::putInt4(const unsigned int int4)
    throw(OutWriteBufferException)
{
    m_wPtr += 4;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (B)");
    //en little-endian quelle que soit la plate-forme
    m_wPtr[-4] = (int4)&0xFF;
    m_wPtr[-3] = (int4>>8)&0xFF;
    m_wPtr[-2] = (int4>>16)&0xFF;
    m_wPtr[-1] = (int4>>24)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a 8-bytes integer into the internal buffer
//param int8 8-bytes integer to write */
void NindFile::putInt8(const long int int8)
    throw(OutWriteBufferException)
{
    m_wPtr += 8;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (C)");
    //en little-endian quelle que soit la plate-forme
    m_wPtr[-8] = (int8)&0xFF;
    m_wPtr[-7] = (int8>>8)&0xFF;
    m_wPtr[-6] = (int8>>16)&0xFF;
    m_wPtr[-5] = (int8>>24)&0xFF;
    m_wPtr[-4] = (int8>>32)&0xFF;
    m_wPtr[-3] = (int8>>40)&0xFF;
    m_wPtr[-2] = (int8>>48)&0xFF;
    m_wPtr[-1] = (int8>>56)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a string into the intermediate buffer
//param str string to write  */
void NindFile::putString(const std::string &str)
    throw(OutWriteBufferException)
{
    if (str.length() > 255) throw OutWriteBufferException("String length");
    const unsigned char stringLen = str.length();
    if (m_wPtr + stringLen + 1 > m_wbufferEnd) throw OutWriteBufferException("in write buffer (D)");
    //ecrit la taille de la chaine
    (*m_wPtr++) = stringLen;
    //ecrit la chaine
    memcpy(m_wPtr, (unsigned char*)str.c_str(), stringLen);
    m_wPtr += stringLen;
}
////////////////////////////////////////////////////////////
//brief Write intermediate buffer to the file */
void NindFile::writeBuffer()
    throw(WriteFileException)
{
    const unsigned int size = m_wPtr - m_wbuffer;
    const unsigned int writeSize =  fwrite(m_wbuffer, 1, size, m_file);
    if (writeSize != size) throw WriteFileException(m_fileName);
    m_wPtr = m_wbuffer;
    //memorise la taille du fichier
    fseek(m_file, 0, SEEK_END);
    m_fileSize = ftell(m_file);
}
////////////////////////////////////////////////////////////
//brief Write byte value to the file
//param value value to write
//param count number of bytes to write*/
void NindFile::writeValue(const unsigned char value,
                          const unsigned int count)
    throw(WriteFileException, BadAllocException)
{
    unsigned char* buffer = new (nothrow) unsigned char[count];
    if (buffer == 0) throw BadAllocException("in write value buffer");
    for (unsigned int it = 0; it != count; it++) buffer[it] = value;
    const unsigned int writeSize =  fwrite(buffer, 1, count, m_file);
    delete [] buffer;
    if (writeSize != count) throw WriteFileException(m_fileName);
    //memorise la taille du fichier
    fseek(m_file, 0, SEEK_END);
    m_fileSize = ftell(m_file);    
}
////////////////////////////////////////////////////////////
//Read bytes from file into specified buffer
void NindFile::readBytes(unsigned char* bytes,
                         const unsigned int bytesNb)
    throw(EofException, ReadFileException)
{
    const unsigned int readSize = fread(bytes, 1, bytesNb, m_file);
    if (readSize != bytesNb) {
        if (feof(m_file)) throw EofException(m_fileName);
        else throw ReadFileException(m_fileName);
    }
}
////////////////////////////////////////////////////////////

