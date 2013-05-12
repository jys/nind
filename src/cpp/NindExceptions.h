//
// C++ Interface: NindExceptions
//
// Description: les exceptions du projet NIND (nouvel index)
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindExceptions_H
#define NindExceptions_H
////////////////////////////////////////////////////////////
#include <stdexcept>
#include <string>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief when something on file fails  */
class FileException : public std::runtime_error {
public:
    std::string m_fileName;
    FileException(const char * name, const std::string fileName) :
        runtime_error(name),
        m_fileName(fileName) {}
    virtual ~FileException() throw() {}
};
/**\brief when EOF occurs (it can be a valid test of EOF)  */
class EofException : public FileException {
    public:
    EofException() :
        FileException("Eof", "") {}
    EofException(const std::string fileName) :
        FileException("Eof", fileName) {}
};
/**\brief when decode file fails  */
class FormatFileException : public FileException {
public:
    FormatFileException() :
        FileException("Format file error", "") {}
    FormatFileException(const std::string fileName) :
        FileException("Format file error", fileName) {}
};
/**\brief when an invalid file is used  */
class IncompatibleFileException : public FileException {
    public:
    IncompatibleFileException() :
        FileException("Incompatible file", "") {}
    IncompatibleFileException(const std::string fileName) :
        FileException("Incompatible file", fileName) {}
};
/**\brief when an invalid file is used  */
class InvalidFileException : public FileException {
    public:
    InvalidFileException() :
        FileException("Invalid file", "") {}
    InvalidFileException(const std::string fileName) :
        FileException("Invalid file", fileName) {}
};
/**\brief when open file fails  */
class OpenFileException : public FileException {
public:
    OpenFileException() :
        FileException("Open file error", "") {}
    OpenFileException(const std::string fileName) :
        FileException("Open file error", fileName) {}
};
/**\brief when read error occurs on a file  */
class ReadFileException : public FileException {
    public:
    ReadFileException() :
        FileException("Read file error", "") {}
    ReadFileException(const std::string fileName) :
        FileException("Read file error", fileName) {}
};
/**\brief when seek error occurs on a file  */
class SeekFileException : public FileException {
    public:
    SeekFileException() :
        FileException("Seek file error", "") {}
    SeekFileException(const std::string fileName) :
        FileException("Seek file error", fileName) {}
};
/**\brief when write error occurs on a file  */
class WriteFileException : public FileException {
    public:
    WriteFileException() :
        FileException("Write file error", "") {}
    WriteFileException(const std::string fileName) :
        FileException("Write file error", fileName) {}
};
////////////////////////////////////////////////////////////
/**\brief when something on lexicon fails  */
class LexiconException : public std::runtime_error {
public:
    std::string m_word;
    LexiconException(const char * name, const std::string word) :
        runtime_error(name),
        m_word(word) {}
    virtual ~LexiconException() throw() {}
};
/**\brief when bad use of lexicon is attempted  */
class BadUseException : public LexiconException {
public:
    BadUseException() :
        LexiconException("Bad use", "") {}
    BadUseException(const std::string word) :
        LexiconException("Bad use", word) {}
};
/**\brief when lexicon is broken  */
class IntegrityException : public LexiconException {
public:
    IntegrityException() :
        LexiconException("Lexicon integrity error", "") {}
    IntegrityException(const std::string word) :
        LexiconException("Lexicon integrity error", word) {}
};
/**\brief when an out of bound parameter is gotten   */
class OutOfBoundException : public LexiconException {
    public:
    OutOfBoundException() :
        LexiconException("Out of bound error", "") {}
    OutOfBoundException(const std::string error) :
        LexiconException("Out of bound error", error) {}
};
////////////////////////////////////////////////////////////
/**\brief when something on terms index fails  */
class TermIndexException : public std::runtime_error {
public:
    std::string m_word;
    TermIndexException(const char * name, const std::string word) :
        runtime_error(name),
        m_word(word) {}
    virtual ~TermIndexException() throw() {}
};
/**\brief when a decode error occurs    */
class DecodeErrorException : public TermIndexException {
    public:
    DecodeErrorException() :
        TermIndexException("Out of bound error", "") {}
    DecodeErrorException(const std::string error) :
        TermIndexException("Out of bound error", error) {}
};
/**\brief when a encode error occurs    */
class EncodeErrorException : public TermIndexException {
    public:
    EncodeErrorException() :
        TermIndexException("Out of bound error", "") {}
    EncodeErrorException(const std::string error) :
        TermIndexException("Out of bound error", error) {}
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
