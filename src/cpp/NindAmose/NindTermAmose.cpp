//
// C++ Implementation: NindTermAmose
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
#include "NindTermAmose.h"
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindTermAmose with a specified name associated with.
//param fileName absolute path file name
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon 
//param cacheSize size of cache for terms from indexTerm
NindTermAmose::NindTermAmose(const string &fileName,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification,
                             const unsigned int cacheSize)
    throw(NindIndexException):
    NindTermIndex(fileName, 
                  false, 
                  lexiconWordsNb, 
                  lexiconIdentification),
    m_cacheSize(cacheSize),
    m_termIndexIdentCache(),
    m_termIndexCache()
{
}
////////////////////////////////////////////////////////////
NindTermAmose::~NindTermAmose()
{
}
////////////////////////////////////////////////////////////
//brief Read the list of documents where term + CG is indexed
//param termId ident of term
//param cg: identifier of gramatical category
//param documents structure to receive the list of documents ids + frequencies
//return true if term was found, false otherwise */
bool NindTermAmose::getDocumentsList(const unsigned int termId,
                                     const unsigned char cg,
                                     list<Document> &documents)
    throw(NindTermIndexException)
{
    list<NindTermAmose::TermIndexType>::const_reverse_iterator itTerm = getFromCache(termId);
    if (itTerm == m_termIndexCache.crend()) return false;
    for (list<NindTermIndex::TermCG>::const_iterator it = (*itTerm).begin(); it != (*itTerm).end(); it++) {
        const NindTermIndex::TermCG &termCG = (*it);
        if (termCG.cg == cg) {
            documents = termCG.documents;
            return true;
        }
    }
    return false;   
}
////////////////////////////////////////////////////////////
//brief Number of documents in index that contain the given term + CG
//param termId: identifier of the term
//param cg: identifier of gramatical category
//return number  of documents in index that contain the given term + CG
unsigned int NindTermAmose::getDocFreq(const unsigned int termId,
                                       const unsigned char cg)
    throw(NindTermIndexException)
{
    list<NindTermAmose::TermIndexType>::const_reverse_iterator itTerm = getFromCache(termId);
    if (itTerm == m_termIndexCache.crend()) return 0;
    for (list<NindTermIndex::TermCG>::const_iterator it = (*itTerm).begin(); it != (*itTerm).end(); it++) {
        const NindTermIndex::TermCG &termCG = (*it);
        if (termCG.cg == cg) return termCG.documents.size();
    }
    return 0;      
}
////////////////////////////////////////////////////////////
//brief Number of occurences in index of the given term + CG
//param termId: identifier of the term
//param cg: identifier of gramatical category
//return number of occurences in index of the given term + CG */
unsigned int NindTermAmose::getTermFreq(const unsigned int termId,
                                        const unsigned char cg)
    throw(NindTermIndexException)
{
    list<NindTermAmose::TermIndexType>::const_reverse_iterator itTerm = getFromCache(termId);
    if (itTerm == m_termIndexCache.crend()) return 0;
    for (list<NindTermIndex::TermCG>::const_iterator it = (*itTerm).begin(); it != (*itTerm).end(); it++) {
        const NindTermIndex::TermCG &termCG = (*it);
        if (termCG.cg == cg) return termCG.frequency;
    }
    return 0;      
}
////////////////////////////////////////////////////////////
//Return iterator on the cache if term exists, end else
//Read TermIndex if it is not in the cache, 
list<NindTermAmose::TermIndexType>::const_reverse_iterator NindTermAmose::getFromCache(const unsigned int termId)
    throw(NindTermIndexException)
{
    list<NindTermAmose::TermIndexType>::const_reverse_iterator itTerm = m_termIndexCache.crbegin();
    for (list<unsigned int>::const_reverse_iterator it = m_termIndexIdentCache.crbegin();
         it != m_termIndexIdentCache.crend(); it++) {
        if ((*it) == termId) return itTerm;
        itTerm++;
    }
    //not found in cache, read from file
    m_termIndexCache.push_back(TermIndexType());
    TermIndexType &termIndex = m_termIndexCache.back();
    if (NindTermIndex::getTermIndex(termId, termIndex)) {
        //found, fill cache
        m_termIndexIdentCache.push_back(termId);
        //if cache is full, dismiss the oldest
        if (m_termIndexIdentCache.size() > m_cacheSize) {
            m_termIndexIdentCache.pop_front();
            m_termIndexCache.pop_front();
        }
        return m_termIndexCache.crbegin();
    }
    else {
        //repair cache
        m_termIndexCache.pop_back();
        return m_termIndexCache.crend();
    }  
}
////////////////////////////////////////////////////////////
