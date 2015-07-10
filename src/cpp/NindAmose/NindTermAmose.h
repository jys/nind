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
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermAmose : public NindTermIndex {
public:

    /** \brief Creates NindTermAmose with a specified name associated with.
    *\param fileName absolute path file name
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon 
    *\param cacheSize size of cache for terms from indexTerm*/
    NindTermAmose(const std::string &fileName,
                  const unsigned int lexiconWordsNb,
                  const unsigned int lexiconIdentification,
                  const unsigned int cacheSize = 5)
        throw(NindIndexException);

    virtual ~NindTermAmose();
    
    /** \brief Read the list of documents where term + CG is indexed
    *\param termId ident of term
    *\param cg: identifier of gramatical category
    *\param documents structure to receive the list of documents ids + frequencies
    *\return true if term was found, false otherwise */
    bool getDocumentsList(const unsigned int termId,
                          const unsigned char cg,
                          std::list<Document> &documents)
        throw(NindTermIndexException);
        
    /** \brief Number of documents in index that contain the given term + CG
    *\param termId: identifier of the term
    *\param cg: identifier of gramatical category
    *\return number  of documents in index that contain the given term + CG */
    unsigned int getDocFreq(const unsigned int termId,
                            const unsigned char cg)
        throw(NindTermIndexException);
 
    /** \brief Number of occurences in index of the given term + CG
    *\param termId: identifier of the term
    *\param cg: identifier of gramatical category
    *\return number of occurences in index of the given term + CG */
    unsigned int getTermFreq(const unsigned int termId,
                            const unsigned char cg)
        throw(NindTermIndexException);
 
private:
    typedef std::list<struct TermCG> TermIndexType;
    //Return iterator on the cache if term exists, end else
    //Read TermIndex if it is not in the cache, 
    std::list<TermIndexType>::const_reverse_iterator getFromCache(const unsigned int termId)
        throw(NindTermIndexException);
        
    unsigned int m_cacheSize;
    std::list<unsigned int> m_termIndexIdentCache;
    std::list<TermIndexType> m_termIndexCache;    
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
