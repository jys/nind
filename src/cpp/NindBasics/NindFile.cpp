//
// C++ Implementation: NindFile
//
// Description: Les acces basiques a un fichier binaire sequentiel avec possibilites de bufferisation
// Le fichier est ecrit en little-endian 
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: 2014-2015 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindFile.h"
#include <string.h>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindFile with a specified name associated with.
//param fileName absolute path file name
//param bufferSize size of writing buffer (0 if just reader) */
NindFile::NindFile(const string &fileName):
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
    m_isLittleEndian = (testLittleEndian == (*(unsigned char*)&testLittleEndian)) ; // vrai si little endian (inutile)
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
//brief Read a single byte from the file
//return byte value */
unsigned char NindFile::readInt1()
{
    unsigned char byte;
    readBytes(&byte, 1);
    return byte;
}
////////////////////////////////////////////////////////////
//brief Read a 3-bytes integer from the file
//return 4-bytes integer */
unsigned int NindFile::readInt3()
{
    //petit boutiste
    unsigned char bytes[3];
    readBytes(bytes, 3);
    return (bytes[2]*256 + bytes[1])*256 + bytes[0];
}
////////////////////////////////////////////////////////////
//brief Get an unsigned latecon integer from the file
//return 4-bytes unsigned integer */
unsigned int NindFile::readUIntLat()
{
    unsigned char bytes[5];
    readBytes(bytes, 1);
    //0-2^7-1 : 0-127
    if (!(bytes[0]&0x80)) return bytes[0];
    readBytes(bytes+1, 1);
    //2^7-2^14-1 : 128-16383
    if (!(bytes[0]&0x40)) return (bytes[0]&0x3F)*256 + bytes[1];
    readBytes(bytes+2, 1);
    //2^14-2^21-1 : 16384-2097151
    if (!(bytes[0]&0x20)) return ((bytes[0]&0x1F)*256 + bytes[1])*256 + bytes[2];
    readBytes(bytes+3, 1);
    //2^21-2^28 : 2097152-268435455
    if (!(bytes[0]&0x10)) return (((bytes[0]&0x0F)*256 + bytes[1])*256 + bytes[2])*256 + bytes[3];
    readBytes(bytes+4, 1);
    if (bytes[0]&0x08) throw FormatFileException(m_fileName);
    //2^28-2^32-1 : 268435456-4294967295
    return ((bytes[1]*256 + bytes[2])*256 + bytes[3])*256 + bytes[4];
}
////////////////////////////////////////////////////////////
//brief Get a signed latecon integer from the file
//return 4-bytes signed integer */
signed int NindFile::readSIntLat()
{
    unsigned char bytes[5];
    readBytes(bytes, 1);
    //de -2^6 a 2^6-1 : de -64 a 63
    if (!(bytes[0]&0x80)) return ((int(bytes[0]))<<25)>>25;
    readBytes(bytes+1, 1);
    //de -2^13 a -2^6-1 et de 2^6 a 2^13-1 : de -8192 a -65 et de 64 a 8191
    if (!(bytes[0]&0x40)) return ((int(bytes[0]&0x3F)*256 + bytes[1])<<18)>>18;
    readBytes(bytes+2, 1);
    //de -2^20 a -2^13-1 et de 2^13 a 2^20-1 : de -1048576 a -8193 et de 8192 a 1048575
    if (!(bytes[0]&0x20)) return ((int((bytes[0]&0x1F)*256 + bytes[1])*256 + bytes[2])<<11)>>11;
    readBytes(bytes+3, 1);
    //de -2^27 a -2^20-1 et de 2^20 a 2^27-1 : de -134217728 a -1048577 et de 1048576 a 134217727
    if (!(bytes[0]&0x10)) return ((int(((bytes[0]&0x0F)*256 + bytes[1])*256 + bytes[2])*256 + bytes[3])<<4)>>4;
    readBytes(bytes+4, 1);
    if (bytes[0]&0x08) throw FormatFileException(m_fileName);
    //de -2^31 a -2^27-1 et de 2^27 a 2^31-1 : de -2147483648 a -134217729 et de 134217728 a 2147483647
    return ((bytes[1]*256 + bytes[2])*256 + bytes[3])*256 + bytes[4];   
}       
////////////////////////////////////////////////////////////
//brief Read a string from the file
//return read string */
string NindFile::readString()
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
{
    //cree le buffer intermediaire de lecture
    if (m_rbuffer != 0) delete [] m_rbuffer;
    m_rbuffer = new (nothrow) unsigned char[bytesNb];
    if (m_rbuffer == 0) throw BadAllocException("in read buffer " + m_fileName);
    m_rbufferEnd = m_rbuffer + bytesNb;
    m_rPtr = m_rbuffer;
    //lit le fichier
    readBytes(m_rbuffer, bytesNb);
}
////////////////////////////////////////////////////////////
//brief Reduce effective buffer data specifying amount of unread datas
//param bytesNb size of effective unread datas */
void NindFile::setEndInBuffer(const unsigned int bytesNb)
{
    if (m_rPtr + bytesNb > m_rbufferEnd) throw OutReadBufferException("in read buffer (A) " + m_fileName);
    m_rbufferEnd = m_rPtr + bytesNb;
}
////////////////////////////////////////////////////////////
//brief Get a 1-bytes integer from internal buffer
//return 4-bytes integer */
unsigned int NindFile::getInt1()
{
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (B) " + m_fileName);
    return m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get a 2-bytes integer from internal buffer
//return 4-bytes integer */
unsigned int NindFile::getInt2()
{
    //gros-boutiste
    m_rPtr += 2;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (C) " + m_fileName);
    return m_rPtr[-2]*256 + m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get a 3-bytes integer from internal buffer
//return 4-bytes integer */
unsigned int NindFile::getInt3()
{
    //petit boutiste
    m_rPtr += 3;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (D) " + m_fileName);
    return (m_rPtr[-1]*256 + m_rPtr[-2])*256 + m_rPtr[-3];
}
////////////////////////////////////////////////////////////
//brief Get a 4-bytes integer from internal buffer
//return 4-bytes integer */
unsigned int NindFile::getInt4()
{
    //petit boutiste
    m_rPtr += 4;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (E) " + m_fileName);
    return ((m_rPtr[-1]*256 + m_rPtr[-2])*256 + m_rPtr[-3])*256 + m_rPtr[-4];
}
////////////////////////////////////////////////////////////
//brief Get a 5-bytes integer from internal buffer
//return 8-bytes integer */
unsigned long int NindFile::getInt5()
{
    //gros boutiste
    m_rPtr += 5;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (F) " + m_fileName);
    return (((m_rPtr[-5]*256 + m_rPtr[-4])*256 + m_rPtr[-3])*256 + m_rPtr[-2])*256 + m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get an unsigned latecon integer from internal buffer
//return 4-bytes unsigned integer */
unsigned int NindFile::getUIntLat()
{
    //il faut que ca aille vite pour les petits nombres
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (G) " + m_fileName);
    //0-2^7-1 : 0-127
    if (!(m_rPtr[-1]&0x80)) return m_rPtr[-1];
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (H) " + m_fileName);
    //2^7-2^14-1 : 128-16383
    if (!(m_rPtr[-2]&0x40)) return (m_rPtr[-2]&0x3F)*256 + m_rPtr[-1];
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (I) " + m_fileName);
    //2^14-2^21-1 : 16384-2097151
    if (!(m_rPtr[-3]&0x20)) return ((m_rPtr[-3]&0x1F)*256 + m_rPtr[-2])*256 + m_rPtr[-1];
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (J) " + m_fileName);
    //2^21-2^28 : 2097152-268435455
    if (!(m_rPtr[-4]&0x10)) return (((m_rPtr[-4]&0x0F)*256 + m_rPtr[-3])*256 + m_rPtr[-2])*256 + m_rPtr[-1];
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (K) " + m_fileName);
    if (m_rPtr[-5]&0x08) throw OutReadBufferException("in read buffer (L) " + m_fileName);
    //2^28-2^32-1 : 268435456-4294967295
    return ((m_rPtr[-4]*256 + m_rPtr[-3])*256 + m_rPtr[-2])*256 + m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get a signed latecon integer from internal buffer
//return 4-bytes signed integer */
signed int NindFile::getSIntLat()
{
    //il faut que ca aille vite pour les petits nombres
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (M) " + m_fileName);
    //de -2^6 a 2^6-1 : de -64 a 63
    if (!(m_rPtr[-1]&0x80)) return ((int(m_rPtr[-1]))<<25)>>25;
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (N) " + m_fileName);
    //de -2^13 a -2^6-1 et de 2^6 a 2^13-1 : de -8192 a -65 et de 64 a 8191
    if (!(m_rPtr[-2]&0x40)) return ((int(m_rPtr[-2]&0x3F)*256 + m_rPtr[-1])<<18)>>18;
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (O) " + m_fileName);
    //de -2^20 a -2^13-1 et de 2^13 a 2^20-1 : de -1048576 a -8193 et de 8192 a 1048575
    if (!(m_rPtr[-3]&0x20)) return ((int((m_rPtr[-3]&0x1F)*256 + m_rPtr[-2])*256 + m_rPtr[-1])<<11)>>11;
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (P) " + m_fileName);
    //de -2^27 a -2^20-1 et de 2^20 a 2^27-1 : de -134217728 a -1048577 et de 1048576 a 134217727
    if (!(m_rPtr[-4]&0x10)) return ((int(((m_rPtr[-4]&0x0F)*256 + m_rPtr[-3])*256 + m_rPtr[-2])*256 + m_rPtr[-1])<<4)>>4;
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (Q) " + m_fileName);
    if (m_rPtr[-5]&0x08) throw OutReadBufferException("in read buffer (R) " + m_fileName);
    //de -2^31 a -2^27-1 et de 2^27 a 2^31-1 : de -2147483648 a -134217729 et de 134217728 a 2147483647
    return ((m_rPtr[-4]*256 + m_rPtr[-3])*256 + m_rPtr[-2])*256 + m_rPtr[-1];
}
////////////////////////////////////////////////////////////
//brief Get a string from internal buffer
//return read string */
string NindFile::getString()
{
    //lit la longueur de la chaine
    m_rPtr += 1;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (S) " + m_fileName);
    const unsigned char len = m_rPtr[-1];
    //lit la chaine
    m_rPtr += len;
    if (m_rPtr > m_rbufferEnd) throw OutReadBufferException("in read buffer (T) " + m_fileName);
    return string((char*)(m_rPtr-len), len);
}   
////////////////////////////////////////////////////////////
//brief Create an internal buffer for writing
//param bytesNb size of buffer */
void NindFile::createBuffer(const unsigned int bytesNb)
{
    if (m_wbuffer != 0) delete [] m_wbuffer;
    m_wbuffer = new (nothrow) unsigned char[bytesNb];
    if (m_wbuffer == 0) throw BadAllocException("in write buffer" + m_fileName);
    m_wbufferEnd = m_wbuffer + bytesNb;
    m_wPtr = m_wbuffer;
}
////////////////////////////////////////////////////////////
//brief Put padding into the internal buffer
//param bytesNb number of padding bytes to write */
void NindFile::putPad(const unsigned int bytesNb)
{
    m_wPtr += bytesNb;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (A) " + m_fileName);
}
////////////////////////////////////////////////////////////
//brief Put one byte into the internal buffer
//param int1 byte value to write */
void NindFile::putInt1(const unsigned char int1)
{
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (B) " + m_fileName);
    m_wPtr[-1] = int1;
}
////////////////////////////////////////////////////////////
//brief Put a 2-bytes integer into the internal buffer
//param int4 integer to write as 2-bytes*/
void NindFile::putInt2(const unsigned int int4)
{
    //gros-boutiste
    m_wPtr += 2;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (C) " + m_fileName);
    //en big-endian quelle que soit la plate-forme
    m_wPtr[-1] = (int4)&0xFF;
    m_wPtr[-2] = (int4>>8)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a 3-bytes integer into the internal buffer
//param int4 integer to write as 3-bytes*/
void NindFile::putInt3(const unsigned int int4)
{
    //petit boutiste
    m_wPtr += 3;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (D) " + m_fileName);
    //en little-endian quelle que soit la plate-forme
    m_wPtr[-3] = (int4)&0xFF;
    m_wPtr[-2] = (int4>>8)&0xFF;
    m_wPtr[-1] = (int4>>16)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a 3-bytes integer into the internal buffer at the specified offset
//param int4 integer to write as 3-bytes
//param offset where to write the 3 bytes into the buffer*/
void NindFile::putInt3(const unsigned int int4,
                       const unsigned int offset)
{
    //petit boutiste
    unsigned char *wPtr = m_wbuffer + offset + 3;
    if (wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (E) " + m_fileName);
    //en little-endian quelle que soit la plate-forme
    wPtr[-3] = (int4)&0xFF;
    wPtr[-2] = (int4>>8)&0xFF;
    wPtr[-1] = (int4>>16)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a 4-bytes integer into the internal buffer
//param int4 integer to write as 4-bytes*/
void NindFile::putInt4(const unsigned int int4)
{
    //petit boutiste
    m_wPtr += 4;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (F) " + m_fileName);
    //en little-endian quelle que soit la plate-forme
    m_wPtr[-4] = (int4)&0xFF;
    m_wPtr[-3] = (int4>>8)&0xFF;
    m_wPtr[-2] = (int4>>16)&0xFF;
    m_wPtr[-1] = (int4>>24)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put a 5-bytes integer into the internal buffer
//param int8 long integer to write as 5-bytes*/
void NindFile::putInt5(const unsigned long int int8)
{
    //gros boutiste
    m_wPtr += 5;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (G) " + m_fileName);
    //en big-endian quelle que soit la plate-forme
    m_wPtr[-1] = (int8)&0xFF;
    m_wPtr[-2] = (int8>>8)&0xFF;
    m_wPtr[-3] = (int8>>16)&0xFF;
    m_wPtr[-4] = (int8>>24)&0xFF;
    m_wPtr[-5] = (int8>>32)&0xFF;
}
////////////////////////////////////////////////////////////
//brief Put an unsigned latecon integer into the internal buffer
//param int4 unsigned integer to write as latecon integer*/
void NindFile::putUIntLat(const unsigned int int4)
{
    //il faut que ca aille vite pour les petits nombres
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (H) " + m_fileName);
    //0-2^7-1 : 0-127
    if (int4 < 0x80) { 
        m_wPtr[-1] = int4; 
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (I) " + m_fileName);
    //2^7-2^14-1 : 128-16383
    if (int4 < 0x4000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = (int4>>8)|0x80;
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (J) " + m_fileName);
    //2^14-2^21-1 : 16384-2097151
    if (int4 < 0x200000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = (int4>>8)&0xFF;
        m_wPtr[-3] = (int4>>16)|0xC0;
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (K) " + m_fileName);
    //2^21-2^28 : 2097152-268435455
    if (int4 < 0x010000000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = (int4>>8)&0xFF;
        m_wPtr[-3] = (int4>>16)&0xFF;
        m_wPtr[-4] = (int4>>24)|0xE0;
        return; 
    }
     m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (L) " + m_fileName);
    //2^28-2^32-1 : 268435456-4294967295
    m_wPtr[-1] = (int4)&0xFF;
    m_wPtr[-2] = (int4>>8)&0xFF;
    m_wPtr[-3] = (int4>>16)&0xFF;
    m_wPtr[-4] = (int4>>24)&0xFF;
    m_wPtr[-5] = 0xF0;
}
////////////////////////////////////////////////////////////
//brief Put a signed latecon integer into the internal buffer
//param int4 signed integer to write as latecon integer*/
void NindFile::putSIntLat(const signed int int4)
{
    //il faut que ca aille vite pour les petits nombres
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (M) " + m_fileName);
    //de -2^6 a 2^6-1 : de -64 a 63
    if (int4 >= -0x40 && int4 < 0x40) { 
        m_wPtr[-1] = int4&0x7F; 
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (N) " + m_fileName);
    //de -2^13 a -2^6-1 et de 2^6 a 2^13-1 : de -8192 a -65 et de 64 a 8191
    if (int4 >= -0x2000 && int4 < 0x2000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = ((int4>>8)&0x3F)|0x80;
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (O) " + m_fileName);
    //de -2^20 a -2^13-1 et de 2^13 a 2^20-1 : de -1048576 a -8193 et de 8192 a 1048575
    if (int4 >= -0x100000 && int4 < 0x100000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = (int4>>8)&0xFF;
        m_wPtr[-3] = ((int4>>16)&0x1F)|0xC0;
        return; 
    }
    m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (P) " + m_fileName);
    //de -2^27 a -2^20-1 et de 2^20 a 2^27-1 : de -134217728 a -1048577 et de 1048576 a 134217727
    if (int4 >= -0x8000000 && int4 < 0x8000000) {
        m_wPtr[-1] = (int4)&0xFF;
        m_wPtr[-2] = (int4>>8)&0xFF;
        m_wPtr[-3] = (int4>>16)&0xFF;
        m_wPtr[-4] = ((int4>>24)&0x0F)|0xE0;
        return; 
    }
     m_wPtr += 1;
    if (m_wPtr > m_wbufferEnd) throw OutWriteBufferException("in write buffer (Q) " + m_fileName);
    //de -2^31 a -2^27-1 et de 2^27 a 2^31-1 : de -2147483648 a -134217729 et de 134217728 a 2147483647
    m_wPtr[-1] = (int4)&0xFF;
    m_wPtr[-2] = (int4>>8)&0xFF;
    m_wPtr[-3] = (int4>>16)&0xFF;
    m_wPtr[-4] = (int4>>24)&0xFF;
    m_wPtr[-5] = ((int4>>31)&0x07)|0xF0;
}
////////////////////////////////////////////////////////////
//brief Put a string into the intermediate buffer
//param str string to write  */
void NindFile::putString(const std::string &str)
{
    if (str.length() > 255) throw OutWriteBufferException("String length" + m_fileName);
    const unsigned char stringLen = str.length();
    if (m_wPtr + stringLen + 1 > m_wbufferEnd) throw OutWriteBufferException("in write buffer (R) " + m_fileName);
    //ecrit la taille de la chaine
    (*m_wPtr++) = stringLen;
    //ecrit la chaine
    memcpy(m_wPtr, (unsigned char*)str.c_str(), stringLen);
    m_wPtr += stringLen;
}
////////////////////////////////////////////////////////////
//brief Write intermediate buffer to the file */
void NindFile::writeBuffer()
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
{
    unsigned char* buffer = new (nothrow) unsigned char[count];
    if (buffer == 0) throw BadAllocException("in write value buffer" + m_fileName);
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
{
    const unsigned int readSize = fread(bytes, 1, bytesNb, m_file);
    if (readSize != bytesNb) {
        if (feof(m_file)) throw EofException(m_fileName);
        else throw ReadFileException(m_fileName);
    }
}
////////////////////////////////////////////////////////////

