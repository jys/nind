//
// C++ Interface: NindLocalAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme document du fichier des index locaux.
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
#ifndef NindLocalAmose_H
#define NindLocalAmose_H
////////////////////////////////////////////////////////////
#include "NindIndex/NindLocalIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <set>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindLocalAmose : public NindLocalIndex {
public:

    /** \brief Creates NindLocalAmose with a specified name associated with.
    *\param fileName absolute path file name
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon 
    *\param cacheSize size of cache for terms from indexTerm*/
    NindLocalAmose(const std::string &fileName,
                  const unsigned int lexiconWordsNb,
                  const unsigned int lexiconIdentification,
                  const unsigned int cacheSize = 5)
        throw(NindIndexException);

    virtual ~NindLocalAmose();
    
    /** \brief Read a full document as a list of terms
    *\param docId ident of doc
    *\param localIndex structure to receive all datas of the specified doc
    *\return true if doc was found, false otherwise */
    bool getLocalIndex(const unsigned int docId,
                       std::list<struct Term> &localIndex)
        throw(NindLocalIndexException);
        
    /** \brief get length of a document
    *\return  an integer, the number of occurrences of terms    */
    unsigned int getDocLength(const unsigned int docId)
        throw(NindLocalIndexException);
        
    struct TermCg {
        unsigned int term;
        unsigned char cg;
        TermCg(): term(0), cg(0) {}
        TermCg(const unsigned int ter, const unsigned char cat): term(ter), cg(cat) {}
        ~TermCg() {}
        bool operator<(const TermCg tcg) const {return this->term < tcg.term; }
    };

    /** \brief Read a full document as a set of unique terms without frequencies
    *\param docId ident of doc
    *\param termCgSet structure to receive unique terms
    *\return true if doc was found, false otherwise */
    bool getUniqueTerms(const unsigned int docId,
                       std::set<struct TermCg> &uniqueTermsSet)
        throw(NindLocalIndexException);
        
private:
    typedef std::list<struct Term> LocalIndexType;
    //Return iterator on the cache if doc exists, end else
    //Read LocalIndex if it is not in the cache, 
    std::list<LocalIndexType>::const_reverse_iterator getFromCache(const unsigned int docId)
        throw(NindLocalIndexException);
        
    unsigned int m_cacheSize;
    std::list<unsigned int> m_localIndexIdentCache;
    std::list<LocalIndexType> m_localIndexCache;    
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
