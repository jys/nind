//
// C++ Interface: NindTermIndexFile
//
// Description: La gestion du fichier inverse en fichier
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindTermIndexFile_H
#define NindTermIndexFile_H
////////////////////////////////////////////////////////////
#include "NindBasics/NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <vector>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermIndexFile {
public:

    /**\brief Creates NindTermIndexFile with a specified name associated with.
    *\param fileName absolute path file name
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
    NindTermIndexFile(const std::string &fileName,
                      const bool isTermIndexWriter)
        throw(OpenFileException, EofException, ReadFileException, WriteFileException, InvalidFileException, OutOfBoundException);

    virtual ~NindTermIndexFile();

    /**\brief Read a full termIndex as a bytes string
    *\param ident ident of term
    *\param bytes buffer address where to write read bytes
    *\param bytesNb size of buffer where to write read bytes
    *\return true if term was found, false otherwise */
    bool getTermIndex(const unsigned int ident,
                      unsigned char *bytes,
                      const unsigned int bytesNb)
        throw(EofException, ReadFileException, InvalidFileException, OutOfBoundException);

    /**\brief Read a full termIndex as a bytes string
    *\param ident ident of term
    *\param bytes address of bytes to write
    *\param bytesNb number of bytes to write 
    *\return true if success, false otherwise */
    bool setTermIndex(const unsigned int ident,
                      const unsigned char *bytes,
                      const unsigned int bytesNb)
        throw(WriteFileException);

    /**\brief set identification
     * \param wordsNb number of words contained in lexicon
     * \param identification unique identification of lexicon */
    void setIdentification(const unsigned int wordsNb,
                           const unsigned int identification)
        throw(WriteFileException);

    /**\brief Perform a clear buffer for reading the true file and not its buffer */
    void clearBuffer();

private:
    //met a jour le vecteur des locations de termes avec le fichier inverse
    void updateFromFile()
    throw(EofException, ReadFileException, InvalidFileException);

    struct RecordLocation {
        long int offset;
        unsigned int size;
        RecordLocation(const long int offs, const unsigned int sz) { offset = offs; size = sz; }
        bool operator< (const RecordLocation r1) { return size < r1.size; }
    };

    bool m_isTermIndexWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    NindFile m_file;                //pour l'indexation ou la recherche
    NindFile m_file_update;         //uniquement pour la mise a jour des localisations
    std::vector<RecordLocation> m_termLocations;  //localisation de chaque terme
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
