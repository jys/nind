//
// C++ Implementation: NindTermAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme terme du fichier inverse.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
//
// Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
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
#define NOMBRE_COMPTEURS 8
////////////////////////////////////////////////////////////
//brief Creates NindTermAmose with a specified name associated with.
//param fileNameExtensionLess absolute path file name without extension
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
//param lexiconIdentification unique identification of lexicon */
//param indirectionBlocSize number of entries in a single indirection block */
NindTermAmose::NindTermAmose(const string &fileNameExtensionLess,
                             const bool isTermIndexWriter,
                             const Identification &lexiconIdentification,
                             const unsigned int indirectionBlocSize):
    NindTermIndex(fileNameExtensionLess,
                  isTermIndexWriter,
                  lexiconIdentification,
                  NOMBRE_COMPTEURS,
                  indirectionBlocSize),
    m_uniqueTermCount({0, 0, 0, 0}),
    m_termOccurrences({0, 0, 0, 0})
{
    //commence par restaurer les compteurs s'ils existent sur le fichier termindex
    readCounts();
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
    //ejcrit le rejsultat sur le fichier
    setTermDef(ident, termDef, fileIdentification, setCountsAsList());
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
        //terminej, ejcrit la nouvelle dejfinition
        setTermDef(ident, termDef, fileIdentification, setCountsAsList());
        break;
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
//return number of documents in index that contain the given term
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
    synchronizeCounts();
    return m_uniqueTermCount[type];
}
////////////////////////////////////////////////////////////
//brief number of terms occurrences
//param type: type of the terms (ALL, SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY)
//return number  of terms of specified type into the base */
unsigned int NindTermAmose::getTermOccurrences(const AmoseTypes type)
{
    synchronizeCounts();
    return m_termOccurrences[type];
}
////////////////////////////////////////////////////////////
//brief read specific counts from termindex file if needed.
    void NindTermAmose::synchronizeCounts()
{
    //l'ejcrivain est par dejfinition dejjah synchronisej
    if (m_isWriter) return;
    //pour le lecteur, lit systejmatiquement sur le fichier
    readCounts();
}

////////////////////////////////////////////////////////////
//brief read specific counts from termindex file.
    void NindTermAmose::readCounts()
{
    //rejcupehre la liste des spejcifiques
    list<unsigned int> spejcifiques;
    getSpecificWords(spejcifiques);
    //charge les compteurs
    list<unsigned int>::const_iterator specIt = spejcifiques.begin();
    m_uniqueTermCount[ALL] = *specIt++;
    m_termOccurrences[ALL] = *specIt++;
    m_uniqueTermCount[SIMPLE_TERM] = *specIt++;
    m_termOccurrences[SIMPLE_TERM] = *specIt++;
    m_uniqueTermCount[MULTI_TERM] = *specIt++;
    m_termOccurrences[MULTI_TERM] = *specIt++;
    m_uniqueTermCount[NAMED_ENTITY] = *specIt++;
    m_termOccurrences[NAMED_ENTITY] = *specIt++;
}
////////////////////////////////////////////////////////////
//brief set specific counts as list
//return counts as a list of words */
    list<unsigned int> NindTermAmose::setCountsAsList()
{
    list<unsigned int> result;
    result.push_back(m_uniqueTermCount[ALL]);
    result.push_back(m_termOccurrences[ALL]);
    result.push_back(m_uniqueTermCount[SIMPLE_TERM]);
    result.push_back(m_termOccurrences[SIMPLE_TERM]);
    result.push_back(m_uniqueTermCount[MULTI_TERM]);
    result.push_back(m_termOccurrences[MULTI_TERM]);
    result.push_back(m_uniqueTermCount[NAMED_ENTITY]);
    result.push_back(m_termOccurrences[NAMED_ENTITY]);
    return result;
}
////////////////////////////////////////////////////////////


