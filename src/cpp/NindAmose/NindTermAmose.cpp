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
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
//param lexiconIdentification unique identification of lexicon */
//param indirectionBlocSize number of entries in a single indirection block */
NindTermAmose::NindTermAmose(const string &fileName,
                             const bool isTermIndexWriter,
                             const Identification &lexiconIdentification,
                             const unsigned int indirectionBlocSize):
    NindTermIndex(fileName, 
                  isTermIndexWriter, 
                  lexiconIdentification,
                  indirectionBlocSize),
    m_uniqueTermCount({{SIMPLE_TERM, 0},{MULTI_TERM, 0},{NAMED_ENTITY, 0}}),
    m_termOccurrences({{SIMPLE_TERM, 0},{MULTI_TERM, 0},{NAMED_ENTITY, 0}})
{
    //commence par restaurer les compteurs s'ils existent sur le fichier termindex
    restoreInternalCounts();
}
////////////////////////////////////////////////////////////
NindTermAmose::~NindTermAmose()
{
}
////////////////////////////////////////////////////////////
//brief Add doc references to the specified term
//param ident ident of term
//param type type of term (0: simple term, 1: multi-term, 2: named entity) 
//param newDocuments list of documents ids + frequencies where term is in 
//param lexiconIdentification unique identification of lexicon */
void NindTermAmose::addDocsToTerm(const unsigned int ident,
                                  const AmoseTypes type,
                                  const list<Document> &newDocuments,
                                  const Identification &lexiconIdentification)
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termIndex;
    getTermIndex(ident, termIndex);
    //si le terme n'existe pas encore, la liste est crejeje avec un ejlejment
    if (termIndex.size() == 0) {
        //increjmente le nombre de termes pour ce type
        m_uniqueTermCount[type] +=1;
        //creje un ejlejment vide
        termIndex.push_back(TermCG());
    }
    //travaille sur l'unique ejlejment
    TermCG &termcg = termIndex.front();
    list<Document> &documents = termcg.documents;
    //ajoute tous les documents
    for (list<Document>::const_iterator itdoc = newDocuments.begin(); itdoc != newDocuments.end(); itdoc++) {
        const Document &document = (*itdoc);
        //increjmente les occurrences pour ce type
        m_termOccurrences[type] += document.frequency;
        //increjmente la frejquence globale de ce terme
        termcg.frequency += document.frequency;
        //trouve la place dans la liste ordonnee
        list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
        while (it2 != documents.end()) {
            //deja dans la liste, incremente la frequence
            if ((*it2).ident == document.ident) {
                (*it2).frequency += document.frequency;
                break;
            }
            //insere a l'interieur de la liste
            if ((*it2).ident > document.ident) {
                documents.insert(it2, document);
                break;
            }
            it2++;
        }
        //si fin de liste, insere en fin
        if (it2 == documents.end()) documents.push_back(document);
    }
    //ejcrit le rejsultat sur le fichier
    setTermIndex(ident, termIndex, lexiconIdentification);  
    //ejcrit les novelles valeurs des compteurs
    saveInternalCounts(lexiconIdentification);
}
////////////////////////////////////////////////////////////
//brief remove doc reference from the specified term
//param ident ident of term
//param type type of term (0: simple term, 1: multi-term, 2: named entity) 
//param documentId id of document to remove
//param lexiconIdentification unique identification of lexicon */
void NindTermAmose::removeDocFromTerm(const unsigned int ident,
                                      const AmoseTypes type,
                                      const unsigned int documentId,
                                      const Identification &lexiconIdentification)
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termIndex;
    getTermIndex(ident, termIndex);
    //si le terme n'existe pas, on ne fait rien
    if (termIndex.size() == 0) return;
    //travaille sur l'unique ejlejment
    TermCG &termcg = termIndex.front();
    list<Document> &documents = termcg.documents;
    //vire le document de la liste des documents
    for (list<Document>::iterator itdoc = documents.begin(); itdoc != documents.end(); itdoc++) {
        Document &document = (*itdoc);
        if (document.ident != documentId) continue;
        //dejcrejmente les occurrences pour ce type
        m_termOccurrences[type] -= document.frequency;
        //dejcrejmente la frejquence globale de ce terme
        termcg.frequency -= document.frequency;
        //enlehve le doc de la liste
        documents.erase(itdoc);
        //si c'ejtait le dernier, efface le terme
        if (documents.size() == 0) {
            //dejcrejmente le nombre de termes pour ce type
            m_uniqueTermCount[type] -=1;
            termIndex.clear();        
        }
        //terminej, ejcrit la nouvelle dejfinition
        setTermIndex(ident, termIndex, lexiconIdentification);  
        return;
    }
    //si document pas trouvej, rien n'est fait
}   
////////////////////////////////////////////////////////////
//brief read specific counts from termindex file
//synchronization between writer and readers is up to application */
    void NindTermAmose::restoreInternalCounts()
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termIndex;
    getTermIndex(0, termIndex);
    //si le terme n'existe pas, on ne fait rien
    if (termIndex.size() == 0) return;
    //travaille sur l'unique ejlejment
    CountsStruct &countsStruct = termIndex.front();
    list<Counts> &counts = countsStruct.documents;
    //remplit les compteurs avec la structure
    list<Counts>::const_iterator itcount = counts.begin();
    m_uniqueTermCount[SIMPLE_TERM] = (*itcount).ident;
    m_termOccurrences[SIMPLE_TERM] = (*itcount++).frequency;
    m_uniqueTermCount[MULTI_TERM] = (*itcount).ident;
    m_termOccurrences[MULTI_TERM] = (*itcount++).frequency;
    m_uniqueTermCount[NAMED_ENTITY] = (*itcount).ident;
    m_termOccurrences[NAMED_ENTITY] = (*itcount++).frequency;
}
////////////////////////////////////////////////////////////
//brief Read the list of documents where term is indexed
//frequencies are not returned
//param termId ident of term
//param documentIds structure to receive the list of documents ids
//return true if term was found, false otherwise */
bool NindTermAmose::getDocList(const unsigned int termId,
                               list<unsigned int> &documentIds)
{
    //raz resultat
    documentIds.clear();
    list<struct TermCG> termIndex;
    const bool trouvej = getTermIndex(termId, termIndex);
    //si terme inconnu, retourne false
    if (!trouvej) return false;
    const TermCG &termCG = termIndex.front();
    const list<Document> &documents = termCG.documents;
    for (list<Document>::const_iterator it = documents.begin(); it != documents.end(); it++) { 
        documentIds.push_back((*it).ident);
    }
    return true;
}
////////////////////////////////////////////////////////////
//brief Number of documents in index that contain the given term
//param termId: identifier of the term
//return number  of documents in index that contain the given term
unsigned int NindTermAmose::getDocFreq(const unsigned int termId)
{
    list<struct TermCG> termIndex;
    const bool trouvej = getTermIndex(termId, termIndex);
    //si terme inconnu, retourne 0
    if (!trouvej) return 0;
    const TermCG &termCG = termIndex.front();
    return termCG.documents.size();
}
////////////////////////////////////////////////////////////
//brief number of unique terms  
//param type: type of the terms (0: simple term, 1: multi-term, 2: named entity) 
//return number of unique terms of specified type into the base */
unsigned int NindTermAmose::getUniqueTermCount(const AmoseTypes type)
{
    map<unsigned int, unsigned int>::const_iterator itcount = m_uniqueTermCount.find(type);
    if (itcount == m_uniqueTermCount.end()) return 0;
    return ((*itcount).second); 
}
////////////////////////////////////////////////////////////
//brief number of terms occurrences 
//param type: type of the terms (0: simple term, 1: multi-term, 2: named entity) 
//return number  of terms of specified type into the base */
unsigned int NindTermAmose::getTermOccurrences(const AmoseTypes type)
{
    map<unsigned int, unsigned int>::const_iterator itcount = m_termOccurrences.find(type);
    if (itcount == m_termOccurrences.end()) return 0;
    return ((*itcount).second); 
}
////////////////////////////////////////////////////////////
//brief write specific counts on termindex file
//synchronization between writer and readers is up to application */
    void NindTermAmose::saveInternalCounts(const Identification &lexiconIdentification)
{
    //les compteurs sont sauvegardejs sur le fichier termindex comme des termes
    list<CountsStruct> termIndex;
    //creje un ejlejment vide
    termIndex.push_back(CountsStruct());
    //travaille sur l'unique ejlejment
    CountsStruct &countsStruct = termIndex.front();
    list<Counts> &counts = countsStruct.documents;
    //remplit la structure avec les compteurs
    counts.push_back(Counts(m_uniqueTermCount[SIMPLE_TERM], m_termOccurrences[SIMPLE_TERM]));
    counts.push_back(Counts(m_uniqueTermCount[MULTI_TERM], m_termOccurrences[MULTI_TERM]));
    counts.push_back(Counts(m_uniqueTermCount[NAMED_ENTITY], m_termOccurrences[NAMED_ENTITY]));
    //ejcrit comme term 0
    setTermIndex(0, termIndex, lexiconIdentification);
}
////////////////////////////////////////////////////////////
