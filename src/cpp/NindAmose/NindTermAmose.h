//
// C++ Interface: NindTermAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme terme du fichier inverse.
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2015
//
// Copyright: See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindTermAmose_H
#define NindTermAmose_H
////////////////////////////////////////////////////////////
#include "NindIndex/NindTermIndex.h"
#include "NindLexiconAmose.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <map>
#include <set>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermAmose : public NindTermIndex {
public:

    /** \brief Creates NindTermAmose with a specified name associated with.
    *\param fileName absolute path file name
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindTermAmose(const std::string &fileName,
                  const bool isTermIndexWriter,
                  const Identification &lexiconIdentification,
                  const unsigned int indirectionBlocSize = 0);

    virtual ~NindTermAmose();
    
    /**\brief Add doc references to the specified term
    *\param ident ident of term
    *\param type type of term (0: simple term, 1: multi-term, 2: named entity) 
    *\param newDocuments list of documents ids + frequencies where term is in 
    *\param lexiconIdentification unique identification of lexicon */
    void addDocsToTerm(const unsigned int ident,
                       const AmoseTypes type,
                       const std::list<Document> &newDocuments,
                       const Identification &lexiconIdentification);
    
    /**\brief remove doc reference from the specified term
    *\param ident ident of term
    *\param type type of term (0: simple term, 1: multi-term, 2: named entity) 
    *\param documentId id of document to remove
    *\param lexiconIdentification unique identification of lexicon */
    void removeDocFromTerm(const unsigned int ident,
                           const AmoseTypes type,
                           const unsigned int documentId,
                           const Identification &lexiconIdentification);

    /**\brief read specific counts from termindex file
     * synchronization between writer and readers is up to application */
    void restoreInternalCounts();
    
// /**
// * @brief get the set of documents containing a term
// *
// * @param termId: identifier of the term
// * @return a pair of iterators pointing to 1) the first element of a set
// * of documents containing a term and to 2) past the end of this set.
// */
// virtual std::pair<DocListIterator, DocListIterator> getDocList(
//     const TermId& termId) const = 0;
// 
// La bufferisation de la liste des identifiants de documents par la classe NindTermAmose serait possible 
// o si getDocList n'est pas réentrant (pas rappellej jusqu'ah ce que son rejsultat soit complehtement utilisej)
// o s'il existait une méthode pour libérer l'espace utilisé pour la liste d'identifiants     
    /** \brief Read the list of documents where term is indexed
    * frequencies are not returned
    *\param termId ident of term
    *\param documentIds structure to receive the list of documents ids
    *\return true if term was found, false otherwise */
    bool getDocList(const unsigned int termId,
                    std::list<unsigned int> &documentIds); 
        
// /** 
// * @brief Number of documents in index that contain the given term 
// * @param termId: identifier of the term 
// */ 
// virtual uint64_t getDocFreq(
//     const TermId& termId) const = 0; 
    /** \brief Number of documents in index that contain the given term
    *\param termId: identifier of the term
    *\return number  of documents in index that contain the given term */
    unsigned int getDocFreq(const unsigned int termId);
    
// /** 
// * @brief number of unique terms 
// * @param type: type of the terms (simple term, multi-term, named entity) 
// */ 
// virtual uint64_t getUniqueTermCount (
//    Lima::Common::BagOfWords::BoWType type ) const = 0; 
    /** \brief number of unique terms  
    *\param type: type of the terms (0: simple term, 1: multi-term, 2: named entity) 
    *\return number of unique terms of specified type into the base */
    unsigned int getUniqueTermCount(const AmoseTypes type);
    
    
// @brief number of terms occurrences 
// @param type: type of the terms (simple term, multi-term, named entity) 
// virtual uint64_t getTermOccurrences (
//    Lima::Common::BagOfWords::BoWType type ) const = 0; 
    /** \brief number of terms occurrences 
    *\param type: type of the terms (0: simple term, 1: multi-term, 2: named entity) 
    *\return number  of terms occurrences of specified type into the base */
    unsigned int getTermOccurrences(const AmoseTypes type);
    
private:
    /**\brief write specific counts on termindex file
     * synchronization between writer and readers is up to application 
    *\param lexiconIdentification unique identification of lexicon */
    void saveInternalCounts(const Identification &lexiconIdentification);
    
    std::map<unsigned int, unsigned int> m_uniqueTermCount;
    std::map<unsigned int, unsigned int> m_termOccurrences;
    
    //pour utiliser la structure commune pour sauvegarder les compteurs sur le fichier termindex
    typedef Document Counts;
    typedef TermCG CountsStruct;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
