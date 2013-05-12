//
// C++ Interface: NindTermIndex
//
// Description: La gestion du fichier inversé
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358 revA
// N'importe quel jargonneux de la syntaxe C++ verra une invitation a mettre un niveau
// d'abstraction pour indexer n'importe quoi. Arriere, manants, seule compte l'algorithmique,
// la syntaxe C++, on s'en cogne.
//
// Cette classe ajoute une nouvelle correspondance entre un identifiant de terme et
// un identifiant de document. Elle donne aussi tous les identifiants de documents
// associés à un terme spécifique
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindTermIndex_H
#define NindTermIndex_H
////////////////////////////////////////////////////////////
#include "NindTerm.h"
#include "NindBasics/NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <ostream>
#include <string>
#include <vector>
#include <list>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between term idents and document idents
*/
class DLLExportLexicon NindTermIndex {
public:
    /**\brief Creates NindTermIndex.
    *\param fileName absolute path file name
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
    NindTermIndex(const std::string &fileName,
                  const bool isTermIndexWriter)
        throw(OpenFileException, EofException, ReadFileException, WriteFileException, InvalidFileException);

    virtual ~NindTermIndex();

    /**\brief Add document ident to term ident
    *\param termId term ident where to add specified document ident
    *\param category grammatical category
    *\param documentId document ident to add to specified term ident  */
    void addDocumentId(const unsigned int termId,
                       const unsigned int category,
                       const unsigned int documentId)
        throw (EofException, ReadFileException, WriteFileException, OutOfBoundException, DecodeErrorException, EncodeErrorException);

    /**\brief Suppress document ident to term ident
    *\param termId term ident where to suppress specified document ident
    *\param documentId document ident to suppress to specified term ident  */
    void suppressDocumentId(const unsigned int termId,
                            const unsigned int documentId)
        throw (EofException, ReadFileException, WriteFileException, OutOfBoundException, DecodeErrorException, EncodeErrorException);

    /**\brief set identification 
     * \param wordsNb number of words contained in lexicon 
     * \param identification unique identification of lexicon */
    void setIdentification(const unsigned int wordsNb,
                           const unsigned int identification)
        throw(WriteFileException);

    /**\brief get all informations about the specified term
    *\param termId term ident to get all indexed informations
    *\param termCatResultList list of all results for the specified term  */
    void getTermInfos(const unsigned int termId,
                      std::list<NindTerm::TermCatResult> &termCatResultList)
        throw (EofException, ReadFileException, OutOfBoundException, DecodeErrorException);
private:
    //Met a jour la gestion des espaces libres et retourne l'offset ou ecrire
    long int getFreeSpace(const unsigned int size);

    struct RecordLocation {
        long int offset;
        unsigned int size;
        RecordLocation(const long int offs, const unsigned int sz) { offset = offs; size = sz; }
        bool operator< (const RecordLocation r1) { return size < r1.size; }
    };
    bool m_isTermIndexWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    NindFile m_file;
    std::vector<RecordLocation> m_termLocations;  //localisation de chaque terme
    std::list<RecordLocation> m_freeLocations;    //gestion des espaces libres
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////
