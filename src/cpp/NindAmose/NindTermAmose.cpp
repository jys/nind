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
//#include <iostream>
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
    m_identification(),
    m_uniqueTermCount({0, 0, 0, 0}),
    m_termOccurrences({0, 0, 0, 0})
{
    //mejmorise l'identification du fichier
    getFileIdentification(m_identification);
    //commence par restaurer les compteurs s'ils existent sur le fichier termindex
    readInternalCounts();
}
////////////////////////////////////////////////////////////
NindTermAmose::~NindTermAmose()
{
}
////////////////////////////////////////////////////////////
//brief Add doc references to the specified term
//param ident ident of term
//param type type of term (SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY) 
//param newDocuments list of documents ids + frequencies where term is in 
//param fileIdentification unique identification of file */
void NindTermAmose::addDocsToTerm(const unsigned int ident,
                                  const AmoseTypes type,
                                  const list<Document> &newDocuments,
                                  const Identification &fileIdentification)
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termDef;
    getTermDef(ident, termDef);
    //si le terme n'existe pas encore, la liste est crejeje avec un ejlejment
    if (termDef.size() == 0) {
        //increjmente le nombre de termes pour ce type
        m_uniqueTermCount[type] +=1;
        m_uniqueTermCount[ALL] +=1;
        //creje un ejlejment vide
        termDef.push_back(TermCG());
    }
    //travaille sur l'unique ejlejment
    TermCG &termcg = termDef.front();
    list<Document> &documents = termcg.documents;       //documents dejjah lah
    //ajoute tous les documents
    for (list<Document>::const_iterator itdoc = newDocuments.begin(); itdoc != newDocuments.end(); itdoc++) {
        const Document &document = (*itdoc);            //document ah ajouter
        unsigned int frequency = document.frequency;    //sa frejquence
        //trouve la place dans la liste ordonnee
        list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
        while (it2 != documents.end()) {
            //deja dans la liste, met ah jour la frejquence
            if ((*it2).ident == document.ident) {
                (*it2).frequency += frequency;
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
        //increjmente les occurrences pour ce type
        m_termOccurrences[type] += frequency;
        m_termOccurrences[ALL] += frequency;
        //increjmente la frejquence globale de ce terme
        termcg.frequency += frequency;
    }
    //ejtablit la nouvelle identification
    m_identification = fileIdentification;
    m_identification.specificFileIdent = m_termOccurrences[ALL];
    //ejcrit le rejsultat sur le fichier
    setTermDef(ident, termDef, m_identification);  
    //ejcrit les novelles valeurs des compteurs
    saveInternalCounts(m_identification);
}
////////////////////////////////////////////////////////////
//brief remove doc reference from the specified term
//param ident ident of term
//param type type of term (SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY) 
//param documentId id of document to remove
//param fileIdentification unique identification of file */
void NindTermAmose::removeDocFromTerm(const unsigned int ident,
                                      const AmoseTypes type,
                                      const unsigned int documentId,
                                      const Identification &fileIdentification)
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termDef;
    getTermDef(ident, termDef);
    //si le terme n'existe pas, on ne fait rien
    if (termDef.size() == 0) return;
    //travaille sur l'unique ejlejment
    TermCG &termcg = termDef.front();
    list<Document> &documents = termcg.documents;
    //vire le document de la liste des documents
    for (list<Document>::iterator itdoc = documents.begin(); itdoc != documents.end(); itdoc++) {
        Document &document = (*itdoc);
        if (document.ident != documentId) continue;
        //dejcrejmente les occurrences pour ce type
        m_termOccurrences[type] -= document.frequency;
        m_termOccurrences[ALL] -= document.frequency;
        //dejcrejmente la frejquence globale de ce terme
        termcg.frequency -= document.frequency;
        //enlehve le doc de la liste
        itdoc = documents.erase(itdoc);
        //si c'ejtait le dernier, efface le terme
        if (itdoc == documents.end()) {
            //dejcrejmente le nombre de termes pour ce type
            m_uniqueTermCount[type] -=1;
            m_uniqueTermCount[ALL] -=1;
            termDef.clear();        
        }
        //ejtablit la nouvelle identification
        m_identification = fileIdentification;
        m_identification.specificFileIdent = m_termOccurrences[ALL];
        //terminej, ejcrit la nouvelle dejfinition
        setTermDef(ident, termDef, m_identification);  
        //ejcrit les nouvelles valeurs des compteurs
        saveInternalCounts(m_identification);
        return;
    }
    //si document pas trouvej, rien n'est fait
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
    list<struct TermCG> termDef;
    const bool trouvej = getTermDef(termId, termDef);
    //si terme inconnu, retourne false
    if (!trouvej) return false;
    const TermCG &termCG = termDef.front();
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
    list<struct TermCG> termDef;
    const bool trouvej = getTermDef(termId, termDef);
    //si terme inconnu, retourne 0
    if (!trouvej) return 0;
    const TermCG &termCG = termDef.front();
    return termCG.documents.size();
}
////////////////////////////////////////////////////////////
//brief number of unique terms  
//param type: type of the terms (ALL, SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY) 
//return number of unique terms of specified type into the base */
unsigned int NindTermAmose::getUniqueTermCount(const AmoseTypes type)
{
    synchronizeInternalCounts();
    return m_uniqueTermCount[type];
}
////////////////////////////////////////////////////////////
//brief number of terms occurrences 
//param type: type of the terms (ALL, SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY) 
//return number  of terms of specified type into the base */
unsigned int NindTermAmose::getTermOccurrences(const AmoseTypes type)
{
    synchronizeInternalCounts();
    return m_termOccurrences[type];
}
////////////////////////////////////////////////////////////
//brief read specific counts from termindex file if needed. 
    void NindTermAmose::synchronizeInternalCounts()
{
    //l'ejcrivain est par dejfinition dejjah synchronisej
    if (m_isWriter) return;
    //le lecteur est ah synchroniser que s'il y a eu changement
    //regarde si le fichier a changej depuis la derniehre mise ah jour
    Identification identification;
    getFileIdentification(identification);
//     cerr<<"m_identification.specificFileIdent="<<m_identification.specificFileIdent;
//     cerr<<" identification.specificFileIdent="<<identification.specificFileIdent<<endl;
    //si pas de changement, raf
    if (identification == m_identification) return;
    //met ah jour l'identification
    m_identification = identification; 
    readInternalCounts();
}

////////////////////////////////////////////////////////////
//brief read specific counts from termindex file. 
    void NindTermAmose::readInternalCounts()
{
    //rejcupehre la dejfinition de ce terme
    list<TermCG> termDef;
    getTermDef(0, termDef);
    //si le terme n'existe pas, on ne fait rien
    if (termDef.size() == 0) return;
    //travaille sur l'unique ejlejment
    CountsStruct &countsStruct = termDef.front();
    list<Counts> &counts = countsStruct.documents;
    //remplit les compteurs avec la structure
    list<Counts>::const_iterator itcount = counts.begin();
    m_uniqueTermCount[ALL] = (*itcount).ident;
    m_termOccurrences[ALL] = (*itcount++).frequency;
    m_uniqueTermCount[SIMPLE_TERM] = (*itcount).ident;
    m_termOccurrences[SIMPLE_TERM] = (*itcount++).frequency;
    m_uniqueTermCount[MULTI_TERM] = (*itcount).ident;
    m_termOccurrences[MULTI_TERM] = (*itcount++).frequency;
    m_uniqueTermCount[NAMED_ENTITY] = (*itcount).ident;
    m_termOccurrences[NAMED_ENTITY] = (*itcount++).frequency;
}
////////////////////////////////////////////////////////////
//brief write specific counts on termindex file
//synchronization between writer and readers is up to application */
    void NindTermAmose::saveInternalCounts(const Identification &identification)
{
    //les compteurs sont sauvegardejs sur le fichier termindex comme des termes
    list<CountsStruct> termDef;
    //creje un ejlejment vide
    termDef.push_back(CountsStruct());
    //travaille sur l'unique ejlejment
    CountsStruct &countsStruct = termDef.front();
    list<Counts> &counts = countsStruct.documents;
    //remplit la structure avec les compteurs
    counts.push_back(Counts(m_uniqueTermCount[ALL], m_termOccurrences[ALL]));
    counts.push_back(Counts(m_uniqueTermCount[SIMPLE_TERM], m_termOccurrences[SIMPLE_TERM]));
    counts.push_back(Counts(m_uniqueTermCount[MULTI_TERM], m_termOccurrences[MULTI_TERM]));
    counts.push_back(Counts(m_uniqueTermCount[NAMED_ENTITY], m_termOccurrences[NAMED_ENTITY]));
    //ejcrit comme term 0
    setTermDef(0, termDef, identification);
}
////////////////////////////////////////////////////////////
