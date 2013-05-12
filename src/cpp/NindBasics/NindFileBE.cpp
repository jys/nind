//
// C++ Implementation: NindFile
//
// Description: Les acces basiques a un fichier binaire sequentiel avec possibilites de bufferisation
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindFile.h"
#include <string.h>
//#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindFile with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if NindFile will write on file, false otherwise
//param bufferSize size of writing buffer (0 if just reader) */
NindFile::NindFile(const string &fileName,
                   const bool isWriter,
                   const unsigned int bufferSize):
    m_isWriter(isWriter),
    m_fileName(fileName),
    m_file(0),
    m_buffer(0),
    m_bufferSize(bufferSize),
    m_sizeInBuffer(0),
    m_isLittleEndian(false)
{
    //teste si la plateforme est little endian ou big endian
    //(pour le moment, on s'en fout, le code est independant de la plateforme)
    const unsigned int testLittleEndian = 13;
    m_isLittleEndian = (testLittleEndian == (*(unsigned char*)&testLittleEndian)) ; // vrai si little endian
    if (m_bufferSize != 0 && m_isWriter) {
        //initialise le buffer d'ecriture
        m_buffer = new unsigned char[m_bufferSize];
    }
}
////////////////////////////////////////////////////////////
NindFile::~NindFile()
{
    close();
    if (m_buffer) {
        delete m_buffer;
        m_buffer = 0;
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
    return (m_file != 0);
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
//brief Read bytes from the file
//param bytes buffer address where to write read bytes
//param bytesNb size of buffer where to write read bytes */
void NindFile::readBytes(unsigned char *bytes,
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
//brief Read a 4-bytes integer from the file
//param int4 4-bytes integer where to write read integer */
void NindFile::readInt4(unsigned int &int4)
    throw(EofException, ReadFileException)
{
    //un entier, c'est 4 octets sur le fichier lexique
    unsigned char bytes[4];
    readBytes(bytes, 4);
    //en big-endian quelle que soit la plate-forme
    int4 = ((bytes[0]*256 + bytes[1])*256 + bytes[2])*256 + bytes[3];
}
////////////////////////////////////////////////////////////
//brief Read a string from the file
//param str string where to write read string */
void NindFile::readString(std::string &str)
    throw(EofException, ReadFileException)
{
    //lit la longueur de la chaine
    unsigned char len;
    readBytes(&len, 1);
    //lit la chaine
    unsigned char bytes[256];
    readBytes(bytes, (unsigned int)len);
    str = string((char*)bytes, len);
}
////////////////////////////////////////////////////////////
//brief Write bytes into the intermediate buffer
//param bytes address of bytes to write
//param bytesNb number of bytes to write */
void NindFile::writeBytes(const unsigned char *bytes,
                          const unsigned int bytesNb)
    throw(WriteFileException)
{
    //ecrit dans le buffer afin d'ecrire en une seule fois sur le fichier pour qu'il reste toujours coherent
    if (m_sizeInBuffer + bytesNb >= m_bufferSize) throw WriteFileException("in lexicon buffer");
    memcpy(&m_buffer[m_sizeInBuffer], bytes, bytesNb);
    m_sizeInBuffer += bytesNb;
}
////////////////////////////////////////////////////////////
//brief Write a 4-bytes integer into the intermediate buffer
//param int4 4-bytes integer to write */
void NindFile::writeInt4(const unsigned int int4)
    throw(WriteFileException)
{
    //un entier, c'est 4 octets sur le fichier lexique
    unsigned char bytes[4];
    //en big-endian quelle que soit la plate-forme
    bytes[0]=(int4>>24)&0xFF;
    bytes[1]=(int4>>16)&0xFF;
    bytes[2]=(int4>>8)&0xFF;
    bytes[3]=(int4)&0xFF;
    writeBytes(bytes, 4);
}
////////////////////////////////////////////////////////////
//brief Write a string into the intermediate buffer
//param str string to write */
void NindFile::writeString(const std::string &str)
    throw(WriteFileException, OutOfBoundException)
{
    //ecrit la taille de la chaine
    if (str.length() > 255) throw OutOfBoundException("String length in NindLexiconFile");
    const unsigned char stringLen = str.length();
    writeBytes(&stringLen, 1);
    //ecrit la chaine elle-meme
    writeBytes((unsigned char*)str.c_str(), stringLen);
}
////////////////////////////////////////////////////////////
//brief Write intermediate buffer to the file */
void NindFile::write()
    throw(WriteFileException)
{
    const unsigned int writeSize =  fwrite(m_buffer, 1, m_sizeInBuffer, m_file);
    if (writeSize != m_sizeInBuffer) throw WriteFileException(m_fileName);
    m_sizeInBuffer = 0;
}
////////////////////////////////////////////////////////////
