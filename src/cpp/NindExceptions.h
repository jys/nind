//
// C++ Interface: NindExceptions
//
// Description: les exceptions du projet NIND (nouvel index)
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
#ifndef NindExceptions_H
#define NindExceptions_H
////////////////////////////////////////////////////////////
#include <stdexcept>
#include <string>
////////////////////////////////////////////////////////////
namespace latecon {
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
/**\brief when an incompatible file is used  */
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
/**\brief when attemp to read over buffer  */
class OutReadBufferException : public FileException {
    public:
    OutReadBufferException() :
        FileException("Out read buffer error", "") {}
    OutReadBufferException(const std::string fileName) :
        FileException("Out read buffer error", fileName) {}
};
/**\brief when attemp to write over buffer  */
class OutWriteBufferException : public FileException {
    public:
    OutWriteBufferException() :
        FileException("Out write buffer error", "") {}
    OutWriteBufferException(const std::string fileName) :
        FileException("Out write buffer error", fileName) {}
};
/**\brief when a bad alloc occurs in allocating buffer  */
class BadAllocException : public FileException {
    public:
    BadAllocException() :
        FileException("Bad alloc error", "") {}
    BadAllocException(const std::string fileName) :
        FileException("Bad alloc error", fileName) {}
};
/**\brief when an out of bound parameter is gotten   */
class OutOfBoundException : public FileException {
    public:
    OutOfBoundException() :
        FileException("Out of bound error", "") {}
    OutOfBoundException(const std::string error) :
        FileException("Out of bound error", error) {}
};
/**\brief when bad use of lexicon is attempted  */
class BadUseException : public FileException {
public:
    BadUseException() :
        FileException("Bad use", "") {}
    BadUseException(const std::string word) :
        FileException("Bad use", word) {}
};
/**\brief when an error occurs on file index  */
class NindIndexException : public FileException {
    public:
    NindIndexException() :
        FileException("Nind Index error", "") {}
    NindIndexException(const std::string fileName) :
        FileException("Nind Index error", fileName) {}
};
/**\brief when an error occurs on term file index  */
class NindTermIndexException : public FileException {
    public:
    NindTermIndexException() :
        FileException("Nind Termindex error", "") {}
    NindTermIndexException(const std::string fileName) :
        FileException("Nind Termindex error", fileName) {}
};
/**\brief when an error occurs on local file index  */
class NindLocalIndexException : public FileException {
    public:
    NindLocalIndexException() :
        FileException("Nind Localindex error", "") {}
    NindLocalIndexException(const std::string fileName) :
        FileException("Nind Localindex error", fileName) {}
};
/**\brief when an error occurs on lexicon file index  */
class NindLexiconIndexException : public FileException {
    public:
    NindLexiconIndexException() :
        FileException("Nind Lexiconindex error", "") {}
    NindLexiconIndexException(const std::string fileName) :
        FileException("Nind Lexiconindex error", fileName) {}
};
/**\brief when an error occurs on lexicon  */
class NindLexiconException : public FileException {
    public:
    NindLexiconException() :
        FileException("Nind Lexicon error", "") {}
    NindLexiconException(const std::string fileName) :
        FileException("Nind Lexicon error", fileName) {}
};
/**\brief when a decode error occurs    */
class DecodeErrorException : public FileException {
    public:
    DecodeErrorException() :
        FileException("Decode error", "") {}
    DecodeErrorException(const std::string error) :
        FileException("Decode error", error) {}
};
/**\brief when a encode error occurs    */
class EncodeErrorException : public FileException {
    public:
    EncodeErrorException() :
        FileException("Encode error", "") {}
    EncodeErrorException(const std::string error) :
        FileException("Encode error", error) {}
};
/**\brief when lexicon is broken  */
class IntegrityException : public FileException {
public:
    IntegrityException() :
        FileException("Lexicon integrity error", "") {}
    IntegrityException(const std::string word) :
        FileException("Lexicon integrity error", word) {}
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
