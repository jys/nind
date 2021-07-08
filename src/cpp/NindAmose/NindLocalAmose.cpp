//
// C++ Implementation: NindLocalAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme document du fichier des index locaux.
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
#include "NindLocalAmose.h"
#include <map>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindLocalAmose with a specified name associated with.
//param fileNameExtensionLess absolute path file name without extension
//param isLocalIndexWriter true if localIndex writer, false if localIndex reader
//param lexiconIdentification unique identification of lexicon
//param indirectionEntryNb number of entries in a single indirection block */
NindLocalAmose::NindLocalAmose(const string &fileNameExtensionLess,
                               const bool isLocalIndexWriter,
                               const Identification &lexiconIdentification,
                               const unsigned int indirectionBlocSize):
    NindLocalIndex(fileNameExtensionLess,
                   isLocalIndexWriter,
                   lexiconIdentification,
                   indirectionBlocSize),
    m_nindLexicon(fileNameExtensionLess, false)
{
}
////////////////////////////////////////////////////////////
NindLocalAmose::~NindLocalAmose()
{
}
////////////////////////////////////////////////////////////
//brief Fill the data structure positions with position of occurrences
//of terms (from @ref termIds) in documents (from @ref documents)
//param termIds vector of identifier of terms
//param documents vector of documents where to search for position of terms.
//param positions position of occurrences of terms in documents. One element foreach content id in @ref documents.
//Each element is a vector containing one element for each term in termIds.
//And each of these elements is the list of positions and lengths of the occurrences of this term in this document.  */
void NindLocalAmose::getTermPositionIndocs(const vector<unsigned int>& termIds,
                                           const vector<unsigned int>& documents,
                                           vector<vector<list<Localisation> > >& positions)
{
    //raz rejsultat
    positions.clear();
    positions.resize(documents.size());
    //crejation d'une map d'accehs inverse au vector des terms
    map<unsigned int, unsigned int> termIdsMap;
    const unsigned int termIdsSize = termIds.size();
    for (unsigned int itterm = 0; itterm != termIdsSize; itterm++) termIdsMap[termIds[itterm]] = itterm;
    //itejrateur dans structure rejsultat
    vector<vector<list<Localisation> > >::iterator itpos = positions.begin();
    //on traite les documents un ah un
    for (vector<unsigned int>::const_iterator itdoc = documents.begin(); itdoc != documents.end(); itdoc++) {
        //le rejsultat pour ce document
        vector<list<Localisation> > &docTerms = (*itpos++);
        docTerms.resize(termIds.size());
        //les index locaux du document
        list<NindLocalIndex::Term> localDef;
        const bool trouvej = NindLocalIndex::getLocalDef((*itdoc), localDef);
        //si le doc n'existe pas, laisse son rejsultat vide et passe au suivant
        if (!trouvej) continue;
        //examine chaque occurrence de terme
        for (list<struct Term>::const_iterator itoccur = localDef.begin(); itoccur != localDef.end(); itoccur++) {
            const struct Term &termOccur = (*itoccur);
            //si ce terme n'est cherchej, son occurrence n'est pas prise en compte
            const map<unsigned int, unsigned int>::const_iterator ittermIdsMap = termIdsMap.find(termOccur.term);
            if (ittermIdsMap == termIdsMap.end()) continue;
            //occurrence ah prendre en compte
            //liste des localisations du terme dans le rejsultat
            const unsigned int idxTerm = (*ittermIdsMap).second;
            list<Localisation> &Localisations = docTerms[idxTerm];
            //ajoute cette localisation aux prejcejdentes
            //(Amose n'indexe pas les localisations frectionnejes)
            Localisations.push_back(termOccur.localisation.front());
        }
    }
}
////////////////////////////////////////////////////////////
//brief get the set of unique term in a document
//param docId identifier of the document
//param termType type of terms (0: simple term, 1: multi-term, 2: named entity)
//param termsSet set de termes uniques dans le document */
bool NindLocalAmose::getDocTerms(const unsigned int docId,
                                 const AmoseTypes termType,
                                 set<string> &termsSet)
{
    //raz rejsultat
    termsSet.clear();
    //les identifiants des termes uniques du document
    set<unsigned int> termIdents;
    const bool trouvej = NindLocalIndex::getTermIdents(docId, termIdents);
    //si le doc n'existe pas, retour false
    if (!trouvej) return false;
    //examine chaque occurrence de terme
    for (set<unsigned int>::const_iterator itterm = termIdents.begin(); itterm != termIdents.end(); itterm++) {
        //cerr<<"NindLocalAmose::getDocTerms (*itterm)="<<(*itterm)<<endl;
        //rejcupehre le terme en string
        string lemma;
        AmoseTypes type;
        string namedEntity;
        const bool trouvej = m_nindLexicon.getWord((*itterm), lemma, type, namedEntity);
        if (!trouvej) throw IncompatibleFileException("Unknown term into lexicon");
        if (type == termType)
        {
            if (type == NAMED_ENTITY)
              termsSet.insert(namedEntity + ":" + lemma);
            else
              termsSet.insert(lemma);
        }
    }
    return true;
}
////////////////////////////////////////////////////////////
//brief get length of a document
//return  an integer, the number of occurrences of terms    */
unsigned int NindLocalAmose::getDocLength(const unsigned int docId)
{
    //les index locaux du document
    unsigned int localLength;
    const bool trouvej = NindLocalIndex::getLocalLength(docId, localLength);
    //si le doc n'existe pas, retour 0
    if (!trouvej) return 0;
    return localLength;
}
////////////////////////////////////////////////////////////
