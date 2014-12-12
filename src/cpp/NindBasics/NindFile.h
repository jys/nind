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
    *\param fileName absolute path file name */
    NindFile(const std::string &fileName);

    virtual ~NindFile();

    /**\brief Open file associated with. No exceptions are raised.
    *\param accessMode authorized access modes are "rb", "wb", "ab", "r+b", "w+b", "a+b", "rb+", "wb+", "ab+"
    *\return true if success, false otherwise */
    bool open(const std::string &accessMode);

    /**\brief Close file associated with. Success in any case. */
    void close();

    /**\brief get current position in file
    *\return current position in file */
    inline long int getPos() const;

    /**\brief get current size of file
    *\return current size of file */
    inline long int getFileSize() const;

    /**\brief Set file on relative position
    *\param offset Number of bytes to offset from origin
    *\param origin Position from where offset is added :
    * SEEK_SET : Beginning of file, SEEK_CUR : Current position of the file pointer, SEEK_END : End of file
    *\return true if success, false otherwise */
    inline bool setPos(const long int offset, const int origin);

    /**brief clear input buffer in reading mode and write output buffer in writing mode
    *\return true if success, false otherwise */
    inline bool flush();

    /**\brief Read a single byte from the file
    *\return byte value */
    unsigned char readInt1()
        throw(EofException, ReadFileException);

    /**\brief Read a 3-bytes integer from the file
    *\return 4-bytes integer */
    unsigned int readInt3()
        throw(EofException, ReadFileException);

    /**\brief Get an unsigned latecon integer from the file
    *\return 4-bytes unsigned integer */
    unsigned int readUIntLat()
        throw(EofException, ReadFileException, FormatFileException);
        
    /**\brief Get a signed latecon integer from the file
    *\return 4-bytes signed integer */
    signed int readSIntLat()
        throw(EofException, ReadFileException, FormatFileException);
        
    /**\brief Read a string from the file
    *\return read string */
    std::string readString()
        throw(EofException, ReadFileException);
        
    /**\brief Read bytes from the file into an internal buffer
    *\param bytesNb size of internal buffer to receive read datas */
    void readBuffer(const unsigned int bytesNb)
        throw(EofException, ReadFileException, BadAllocException);
        
    /**\brief Reduce effective buffer data specifying amount of unread datas
    *\param bytesNb size of effective unread datas */
    void setEndInBuffer(const unsigned int bytesNb)
        throw(OutReadBufferException);

    /**\brief Test if effective buffer was entirely read
    *\return true if effective buffer was entirely read */
    inline bool endOfInBuffer();
        
    /**\brief Get a 1-bytes integer from internal buffer
    *\return 4-bytes integer */
    unsigned int getInt1()
        throw(OutReadBufferException);

    /**\brief Get a 2-bytes integer from internal buffer
    *\return 4-bytes integer */
    unsigned int getInt2()
        throw(OutReadBufferException);

    /**\brief Get a 3-bytes integer from internal buffer
    *\return 4-bytes integer */
    unsigned int getInt3()
        throw(OutReadBufferException);

    /**\brief Get a 4-bytes integer from internal buffer
    *\return 4-bytes integer */
    unsigned int getInt4()
        throw(OutReadBufferException);

    /**\brief Get a 5-bytes integer from internal buffer
    *\return 8-bytes integer */
    unsigned long int getInt5()
        throw(OutReadBufferException);
        
    /**\brief Get an unsigned latecon integer from internal buffer
    *\return 4-bytes unsigned integer */
    unsigned int getUIntLat()
        throw(OutReadBufferException);
        
    /**\brief Get a signed latecon integer from internal buffer
    *\return 4-bytes signed integer */
    signed int getSIntLat()
        throw(OutReadBufferException);

    /**\brief Get a string from internal buffer
    *\return read string */
    std::string getString()
        throw(EofException, ReadFileException);
        
    /**\brief Create an internal buffer for writing
    *\param bytesNb size of buffer */
    void createBuffer(const unsigned int bytesNb)
        throw(BadAllocException);
        
    /**\brief Return the effective size of buffer for writing
     * \return the effective actual size */
    inline unsigned int getOutBufferSize();

    /**\brief Put padding into the internal buffer
    *\param bytesNb number of padding bytes to write */
    void putPad(const unsigned int bytesNb)
        throw(OutWriteBufferException);

    /**\brief Put one byte into the internal buffer
    *\param int1 byte value to write */
    void putInt1(const unsigned char int1)
        throw(OutWriteBufferException);

    /**\brief Put a 2-bytes integer into the internal buffer
    *\param int4 integer to write as 2-bytes*/
    void putInt2(const unsigned int int4)
        throw(OutWriteBufferException);

    /**\brief Put a 3-bytes integer into the internal buffer
    *\param int4 integer to write as 3-bytes*/
    void putInt3(const unsigned int int4)
        throw(OutWriteBufferException);

    /**\brief Put a 3-bytes integer into the internal buffer at the specified offset
    *\param int4 integer to write as 3-bytes
    *\param offset where to write the 3 bytes into the buffer*/
    void putInt3(const unsigned int int4,
                 const unsigned int offset)
        throw(OutWriteBufferException);

    /**\brief Put a 4-bytes integer into the internal buffer
    *\param int4 integer to write as 4-bytes*/
    void putInt4(const unsigned int int4)
        throw(OutWriteBufferException);

    /**\brief Put a 5-bytes integer into the internal buffer
    *\param int8 long integer to write as 5-bytes*/
    void putInt5(const unsigned long int int8)
        throw(OutWriteBufferException);

    /**\brief Put an unsigned latecon integer into the internal buffer
    *\param int4 unsigned integer to write as latecon integer*/
    void putUIntLat(const unsigned int int4)
        throw(OutWriteBufferException);

    /**\brief Put a signed latecon integer into the internal buffer
    *\param int4 signed integer to write as latecon integer*/
    void putSIntLat(const signed int int4)
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
//brief get current position in file
//return current position in file */
inline long int NindFile::getPos() const
{
    return ftell(m_file);
}
////////////////////////////////////////////////////////////
//brief get current size of file
//return current size of file */
inline long int NindFile::getFileSize() const
{
    return m_fileSize;
}
////////////////////////////////////////////////////////////
//brief Set file on relative position
//param offset Number of bytes to offset from origin
//param origin Position from where offset is added :
//SEEK_SET : Beginning of file, SEEK_CUR : Current position of the file pointer, SEEK_END : End of file
//return true if success, false otherwise */
inline bool NindFile::setPos(const long int offset,
                      const int origin)
{
    return fseek(m_file, offset, origin);
}
////////////////////////////////////////////////////////////
//brief clear input buffer in reading mode and write output buffer in writing mode
//return true if success, false otherwise */
inline bool NindFile::flush()
{
    return fflush(m_file);
}
////////////////////////////////////////////////////////////
//brief Test if effective buffer was entirely read
//return true if effective buffer was entirely read */
inline bool NindFile::endOfInBuffer()
{
    return (m_rPtr >= m_rbufferEnd);
}
////////////////////////////////////////////////////////////
//brief Return the effective size of buffer for writing
//return the effective actual size */
inline unsigned int NindFile::getOutBufferSize()
{
    return (m_wPtr - m_wbuffer);
}
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
