//
// C++ Implementation: NindLexiconAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les spejcificitejs du lexique d'Amose, particuliehrement les types de termes.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
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
#include "NindLexiconAmose.h"
#include <sstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void splitTerm(const string &lemma,
                      const unsigned int type,
                      const string &namedEntity,
                      list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconAmose.
//param fileName absolute path file name. Lexicon is identified by its file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
//param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
NindLexiconAmose::NindLexiconAmose(const string &fileName,
                                   const bool isLexiconWriter,
                                   const unsigned int indirectionBlocSize,
                                   const unsigned int retroIndirectionBlocSize):
    NindLexiconIndex(fileName,
                     isLexiconWriter,
                     true,
                     indirectionBlocSize,
                     retroIndirectionBlocSize),
    m_semicolonIdent(0)
{
}
////////////////////////////////////////////////////////////
NindLexiconAmose::~NindLexiconAmose()
{
}
////////////////////////////////////////////////////////////
//brief add specified term in lexicon if it doesn't still exist in,
//In all cases, term ident is returned.
//param lemma word to be lexiced. Compound word is structured with "_"
//param type type of the terms (0: simple term, 1: multi-term, 2: named entity) 
//param namedEntity type of named entity, eventually
//return ident of word */
unsigned int NindLexiconAmose::addTerm(const string &lemma,
                                       const unsigned int type,
                                       const std::string &namedEntity)
{
    list<string> simpleWords;
    splitTerm(lemma, type, namedEntity, simpleWords);
    return addWord(simpleWords);
}
////////////////////////////////////////////////////////////
//brief get ident of a specified term
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param lemma word to be searched. Compound word is structured with "_"
//param type type of the terms (0: simple term, 1: multi-term, 2: named entity) 
//param namedEntity type of named entity, eventually
//return ident of word */
unsigned int NindLexiconAmose::getTermId(const string &lemma,
                                         const unsigned int type,
                                         const string &namedEntity)
{
    list<string> simpleWords;
    splitTerm(lemma, type, namedEntity, simpleWords);
    return getId(simpleWords);
}
////////////////////////////////////////////////////////////
//brief get term components from a specified term id 
//param lemma word corresponding to term id. Compound word is structured with "_"
//param type type of the terms (0: simple term, 1: multi-term, 2: named entity) 
//param namedEntity type of named entity, eventually
//return true if term exists, false otherwise */
bool NindLexiconAmose::getTerm(const unsigned int termId,
                               string &lemma,
                               unsigned int &type,
                               string &namedEntity)
{
    //rejcupehre la liste des composants
    list<string> components;
    const bool trouvej = getComponents(termId, components);
    if (!trouvej) return false;
    if (components.size() == 0) throw NindLexiconException("empty term");
    if (components.size() == 1) type = SIMPLE_TERM;
    else type = MULTI_TERM;
    list<string>::const_iterator itcomp = components.begin();
    string component = (*itcomp++);
    if (component == "§") {
        if (components.size() < 3) throw NindLexiconException("named entity error");
        type = NAMED_ENTITY;
        namedEntity = (*itcomp++);
        component = (*itcomp++);
    }
    while (itcomp != components.end()) component += '_' + (*itcomp++);
    lemma = component;
    return true;
}
////////////////////////////////////////////////////////////
//met en forme le terme pour interroger le lexique
static void splitTerm(const string &lemma,
                      const unsigned int type,
                      const string &namedEntity,
                      list<string> &simpleWords)
{
    //le premier ejlejment d'une entitej nommeje est le type de l'entitej
    if (type == NAMED_ENTITY) {
        simpleWords.push_back("§");
        simpleWords.push_back(namedEntity);
    }
    string simpleWord;
    stringstream sword(lemma);
    while (getline(sword, simpleWord, '_')) {
        if (!simpleWord.empty()) simpleWords.push_back(simpleWord);
    }
}
////////////////////////////////////////////////////////////
