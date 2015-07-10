//
// C++ Implementation: NindLocalAmose
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
#include "NindLocalAmose.h"
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindLocalAmose with a specified name associated with.
//param fileName absolute path file name
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon 
//param cacheSize size of cache for terms from indexTerm*/
NindLocalAmose::NindLocalAmose(const string &fileName,
                               const unsigned int lexiconWordsNb,
                               const unsigned int lexiconIdentification,
                               const unsigned int cacheSize)
    throw(NindIndexException):
    NindLocalIndex(fileName, 
                   false, 
                   lexiconWordsNb, 
                   lexiconIdentification),
    m_cacheSize(cacheSize),
    m_localIndexIdentCache(),
    m_localIndexCache()
{
}
////////////////////////////////////////////////////////////
NindLocalAmose::~NindLocalAmose()
{
}
////////////////////////////////////////////////////////////
//brief Read a full document as a list of terms
//param docId ident of doc
//param localIndex structure to receive all datas of the specified doc
//return true if doc was found, false otherwise */
bool NindLocalAmose::getLocalIndex(const unsigned int docId,
                                   list<struct Term> &localIndex)
    throw(NindLocalIndexException)
{
    list<NindLocalAmose::LocalIndexType>::const_reverse_iterator itLocal = getFromCache(docId); 
    if (itLocal == m_localIndexCache.crend()) return false;
    localIndex = (*itLocal);
    return true;
}
////////////////////////////////////////////////////////////
//brief get length of a document
//return  an integer, the number of occurrences of terms    */
unsigned int NindLocalAmose::getDocLength(const unsigned int docId)
    throw(NindLocalIndexException)
{
    list<NindLocalAmose::LocalIndexType>::const_reverse_iterator itLocal = getFromCache(docId); 
    if (itLocal == m_localIndexCache.crend()) return 0;
    return (*itLocal).size();
}
////////////////////////////////////////////////////////////
//brief Read a full document as a set of unique terms without frequencies
//param docId ident of doc
//param termCgSet structure to receive unique terms
//return true if doc was found, false otherwise */
bool NindLocalAmose::getUniqueTerms(const unsigned int docId,
                                    set<struct TermCg> &uniqueTermsSet)
    throw(NindLocalIndexException)
{
    list<NindLocalAmose::LocalIndexType>::const_reverse_iterator itLocal = getFromCache(docId); 
    if (itLocal == m_localIndexCache.crend()) return false;
    uniqueTermsSet.clear();
    for (list<NindLocalIndex::Term>::const_iterator it = (*itLocal).begin(); it != (*itLocal).end(); it++) {
        const NindLocalIndex::Term &term = (*it);
        const TermCg termCg(term.term, term.cg);
        uniqueTermsSet.insert(termCg);
    }
    return true;
}
////////////////////////////////////////////////////////////
//Return iterator on the cache if doc exists, end else
//Read LocalIndex if it is not in the cache, 
list<NindLocalAmose::LocalIndexType>::const_reverse_iterator NindLocalAmose::getFromCache(const unsigned int docId)
    throw(NindLocalIndexException)
{
    list<NindLocalAmose::LocalIndexType>::const_reverse_iterator itLocal = m_localIndexCache.crbegin();
    for (list<unsigned int>::const_reverse_iterator it = m_localIndexIdentCache.crbegin();
         it != m_localIndexIdentCache.crend(); it++) {
        if ((*it) == docId) return itLocal;
        itLocal++;
    }
    //not found in cache, read from file
    m_localIndexCache.push_back(LocalIndexType());
    LocalIndexType &localIndex = m_localIndexCache.back();
    if (NindLocalIndex::getLocalIndex(docId, localIndex)) {
        //found, fill cache
        m_localIndexIdentCache.push_back(docId);
        //if cache is full, dismiss the oldest
        if (m_localIndexIdentCache.size() > m_cacheSize) {
            m_localIndexIdentCache.pop_front();
            m_localIndexCache.pop_front();
        }
        return m_localIndexCache.crbegin();
    }
    else {
        //repair cache
        m_localIndexCache.pop_back();
        return m_localIndexCache.crend();
    }  
}
////////////////////////////////////////////////////////////
