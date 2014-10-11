//
// C++ Interface: NindFile
//
// Description: Les acces basiques a un fichier binaire sequentiel avec possibilites de bufferisation
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
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
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindFile {
public:

    /**\brief Creates NindFile with a specified name associated with.
    *\param fileName absolute path file name
    *\param isWriter true if NindFile will write on file, false otherwise */
    NindFile(const std::string &fileName,
             const bool isWriter);

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

    /**\brief Read a single byte from the file
    *\return byte value */
    unsigned char readChar()
        throw(EofException, ReadFileException);

    /**\brief Read a 4-bytes integer from the file
    *\return 4-bytes integer */
    unsigned int readInt4()
        throw(EofException, ReadFileException);

    /**\brief Read a 8-bytes integer from the file
    *\return 8-bytes integer */
    long int readInt8()
        throw(EofException, ReadFileException);

    /**\brief Read a string from the file
    *\param str string where to write read string */
    std::string readString()
        throw(EofException, ReadFileException);
        
    /**\brief Read bytes from the file into an internal buffer
    *\param bytesNb size of internal buffer to receive read datas */
    void readBuffer(const unsigned int bytesNb)
        throw(EofException, ReadFileException, BadAllocException);

    /**\brief Get a single byte from internal buffer
    *\return byte value */
    unsigned char getChar()
        throw(OutReadBufferException);

    /**\brief Get a 4-bytes integer from internal buffer
    *\return 4-bytes integer */
    unsigned int getInt4()
        throw(OutReadBufferException);

    /**\brief Get a 8-bytes integer from internal buffer
    *\return 8-bytes integer */
    long int getInt8()
        throw(OutReadBufferException);
        
    /**\brief Test if buffer was entirely read
    *\return true if buffer  was entirely read */
    inline bool endOfBuffer();
        
    /**\brief Create an internal buffer for writing
    *\param bytesNb size of buffer */
    void createBuffer(const unsigned int bytesNb)
        throw(BadAllocException);

    /**\brief Put one byte into the internal buffer
    *\param value byte value to write */
    void putChar(const unsigned char value)
        throw(OutWriteBufferException);

    /**\brief Put a 4-bytes integer into the internal buffer
    *\param int4 4-bytes integer to write */
    void putInt4(const unsigned int int4)
        throw(OutWriteBufferException);

    /**\brief Put a 8-bytes integer into the internal buffer
    *\param int8 8-bytes integer to write */
    void putInt8(const long int int8)
        throw(OutWriteBufferException);

    /**\brief Put a string into the intermediate buffer
    *\param str string to write  */
    void putString(const std::string &str)
        throw(OutWriteBufferException);

    /**\brief Write intermediate buffer to the file */
    void writeBuffer()
        throw(WriteFileException);
        
    /**\brief Write byte value to the file
    *\param value value to write
    *\param count number of bytes to write */
    void writeValue(const unsigned char value,
                    const unsigned int count)
        throw(WriteFileException, BadAllocException);
       
private:
    //Read bytes from file into specified buffer
    void readBytes(unsigned char* bytes,
                   const unsigned int bytesNb)
        throw(EofException, ReadFileException);

    bool m_isWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    FILE *m_file;
    unsigned char *m_wbuffer;        //buffer d'ecriture
    unsigned char *m_wbufferEnd;     //fin buffer d'ecriture
    unsigned char *m_wPtr;           //pointeur buffer d'ecriture
    unsigned char *m_rbuffer;        //buffer de lecture
    unsigned char *m_rbufferEnd;     //fin buffer de lecture
    unsigned char *m_rPtr;           //pointeur buffer de lecture
    long int m_fileSize;             //taille du fichier
    bool m_isLittleEndian;
};
////////////////////////////////////////////////////////////
//brief Test if buffer was entirely read
//return true if buffer  was entirely read */
inline bool NindFile::endOfBuffer()
{
    return (m_rPtr >= m_rbufferEnd);
}
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
