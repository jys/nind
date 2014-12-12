//
// C++ Interface: NindLexiconFile
//
// Description: La gestion du lexique en fichier
// Étude et maquette d'un lexique complet ANT2012.JYS.R357 revA
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindLexiconFile_H
#define NindLexiconFile_H
////////////////////////////////////////////////////////////
#include "NindBasics/NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindLexiconFile {
public:

    /**\brief Creates LexiconFile with a specified name associated with.
    *\param fileName absolute path file name
    *\param fromLexiconWriter true if from lexicon writer, false if from lexicon reader  */
    NindLexiconFile(const std::string &fileName,
                    const bool fromLexiconWriter)
        throw(NindLexiconException);

    virtual ~NindLexiconFile();

    /**\brief Read next record of lexicon file as word definition.
    * if next record is word definition, file pointer is advanced to the next record,
    * else, file pointer is left unchanged
    *\param ident where ident will be returned
    *\param isSimpleWord where true will be returned if it is a simple word, false otherwise
    *\param simpleWord where simple word will be returned, if it is a simple word
    *\param compoundWord where compound word will be returned, if it is a compound word
    *\return true if next record is word definition, false otherwise */
    bool readNextRecordAsWordDefinition(unsigned int &ident,
                                        bool &isSimpleWord,
                                        std::string &simpleWord,
                                        std::pair<unsigned int, unsigned int> &compoundWord)
        throw(EofException, ReadFileException, InvalidFileException);
    
    /**\brief Read next record of lexicon file as lexicon identification.
    * file pointer is left unchanged
    *\param maxIdent where max ident will be returned
    *\param identification where unique identification will be returned  
    *\return true if next record is lexicon identification, false otherwise */
    bool readNextRecordAsLexiconIdentification(unsigned int &maxIdent,
                                               unsigned int &identification)
        throw(EofException, ReadFileException, InvalidFileException, OutReadBufferException);

    /**\brief Write simple word definition on lexicon file.
    *\param ident word ident
    *\param simpleWord simple word
    *\param maxIdent max ident
    *\param identification unique identification  */
    void writeSimpleWordDefinition(const unsigned int ident,
                                   const std::string &simpleWord,
                                   const unsigned int maxIdent,
                                   const unsigned int identification)
        throw(WriteFileException, BadUseException, OutWriteBufferException);

    /**\brief Write compound word definition on lexicon file.
    *\param ident word ident
    *\param compoundWord compound word
    *\param maxIdent max ident
    *\param identification unique identification  */
    void writeCompoundWordDefinition(const unsigned int ident,
                                     const std::pair<unsigned int, unsigned int> compoundWord,
                                     const unsigned int maxIdent,
                                     const unsigned int identification)
        throw(WriteFileException, BadUseException, OutWriteBufferException);

    /**\brief Perform a clear buffer for reading the true file and not its buffer */
    inline void clearBuffer();

private:
    bool m_fromLexiconWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    NindFile m_file;
}; 
////////////////////////////////////////////////////////////
//brief Perform a clear buffer for reading the true file and not its buffer */
inline void NindLexiconFile::clearBuffer()
{
    m_file.flush();
}
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
