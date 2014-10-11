//
// C++ Interface: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindTermIndex_H
#define NindTermIndex_H
////////////////////////////////////////////////////////////
#include "NindBasics/NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermIndex {
public:

    /**\brief Creates NindTermIndex with a specified name associated with.
    *\param fileName absolute path file name
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon */
    NindTermIndex(const std::string &fileName,
                  const bool isTermIndexWriter,
                  const unsigned int lexiconWordsNb,
                  const unsigned int lexiconIdentification)
        throw(OpenFileException, EofException, ReadFileException, WriteFileException, IncompatibleFileException, InvalidFileException, OutOfBoundException);

    virtual ~NindTermIndex();
    
    /**\brief Structures to hold datas of a term */
    struct Document {
        unsigned int ident;
        unsigned int frequency;
        Document(): ident(0), frequency(0) {}
        Document(const unsigned int id, const unsigned int freq): ident(id), frequency(freq) {}
        ~Document() {}
    };
    struct TermCG {
        unsigned int cg;
        unsigned int frequency;
        std::list<Document> documents;
        TermCG(): cg(0), frequency(0), documents() {}
        TermCG(const unsigned int cat, const unsigned int freq): cg(cat), frequency(freq), documents() {}
        ~TermCG() {}
    };
    
    /**\brief Read a full termIndex as a list of structures
    *\param ident ident of term
    *\param termIndex structure to receive all datas of the specified term
    *\return true if term was found, false otherwise */
    bool getTermIndex(const unsigned int ident,
                      std::list<struct TermCG> &termIndex)
        throw(EofException, ReadFileException, InvalidFileException, OutOfBoundException);

    /**\brief Write a full termIndex as a bytes string
    *\param ident ident of term
    *\param termIndex structure containing all datas of the specified term */
    void setTermIndex(const unsigned int ident,
                      const std::list<struct TermCG> &termIndex)
        throw(WriteFileException);

    /**\brief set identification
     * \param wordsNb number of words contained in lexicon
     * \param identification unique identification of lexicon */
    void setIdentification(const unsigned int wordsNb,
                           const unsigned int identification)
        throw(WriteFileException);

private:
//recupere l'indirection du terme specifie
    void getIndirection(const unsigned int ident,
                        long int &offsetEntree,
                        unsigned int &longueurEntree)
        throw(EofException, ReadFileException, OutOfBoundException);

    bool m_isTermIndexWriter;       //true si autorise a ecrire, false sinon
    std::string m_fileName;
    NindFile m_file;                //pour l'ecrivain ou le lecteur
    std::list<std::pair<long int, unsigned int> > m_indirectionMapping;  //gestion des indirections
    std::list<std::pair<long int, unsigned int> > m_emptyAreas;         //gestion des zones libres
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
