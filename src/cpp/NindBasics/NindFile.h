//
// C++ Interface: NindFile
//
// Description: Les acces basiques a un fichier binaire sequentiel avec possibilites de bufferisation
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindFile_H
#define NindFile_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindFile {
public:

    /**\brief Creates NindFile with a specified name associated with.
    *\param fileName absolute path file name
    *\param isWriter true if NindFile will write on file, false otherwise
    *\param bufferSize size of writing buffer (0 if just reader) */
    NindFile(const std::string &fileName,
             const bool isWriter,
             const unsigned int bufferSize = 0);

    virtual ~NindFile();

    /**\brief Open file associated with. No exceptions are raised.
    *\param accessMode authorized access modes are "rb", "wb", "ab", "r+b", "w+b", "a+b", "rb+", "wb+", "ab+"
    *\return true if success, false otherwise */
    bool open(const std::string &accessMode);

    /**\brief Close file associated with. Success in any case. */
    void close();

    /**\brief get current position in file
    *\return current position in file */
    long int getPos() const;

    /**\brief get current size of file
    *\return current size of file */
    long int getFileSize() const;

    /**\brief Set file on relative position
    *\param offset Number of bytes to offset from origin
    *\param origin Position from where offset is added :
    * SEEK_SET : Beginning of file, SEEK_CUR : Current position of the file pointer, SEEK_END : End of file
    *\return true if success, false otherwise */
    bool setPos(const long int offset, const int origin);

    /**brief clear input buffer in reading mode and write output buffer in writing mode
    *\return true if success, false otherwise */
    bool flush();

    /**\brief Read bytes from the file
    *\param bytes buffer address where to write read bytes
    *\param bytesNb size of buffer where to write read bytes */
    void readBytes(unsigned char *bytes,
                   const unsigned int bytesNb)
        throw(EofException, ReadFileException);

    /**\brief Read a 4-bytes integer from the file
    *\param int4 4-bytes integer where to write read integer */
    void readInt4(unsigned int &int4)
        throw(EofException, ReadFileException);

    /**\brief Read a 4-bytes integer from a buffer
    *\param int4 4-bytes integer where to write read integer
    *\param bytes buffer address where to read integer*/
    void readInt4(unsigned int &int4,
                  const unsigned char *bytes);

    /**\brief Read a string from the file
    *\param str string where to write read string */
    void readString(std::string &str)
        throw(EofException, ReadFileException);

    /**\brief Write bytes into the intermediate buffer
    *\param bytes address of bytes to write
    *\param bytesNb number of bytes to write */
    void writeBytes(const unsigned char *bytes,
                    const unsigned int bytesNb)
        throw(WriteFileException);

    /**\brief Write a 4-bytes integer into the intermediate buffer
    *\param int4 4-bytes integer to write */
    void writeInt4(const unsigned int int4)
        throw(WriteFileException);

    /**\brief Write a string into the intermediate buffer
    *\param str string to write */
    void writeString(const std::string &str)
        throw(WriteFileException, OutOfBoundException);

    /**\brief Write intermediate buffer to the file */
    void write()
        throw(WriteFileException);

    /**\brief Perform a clear buffer for reading the true file and not its buffer */
    void clearBuffer();
    
private:

    bool m_isWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    FILE *m_file;
    unsigned char* m_buffer;        //buffer d'ecriture pour que le fichier ne soit jamais incoherent
    unsigned int m_bufferSize;      //taille du buffer d'ecriture
    unsigned int m_sizeInBuffer;    //taille des donnes en attente d'ecriture
    long int m_fileSize;            //taille du fichier
    bool m_isLittleEndian;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
