//
// C++ Implementation: NindLexiconAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les spejcificitejs du lexique d'Amose, particuliehrement les types de mots.
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
#include "NindLexiconAmose.h"
#include <sstream>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void splitWord(const string &lemma,
                      const unsigned int type,
                      const string &namedEntity,
                      list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconAmose.
//param fileNameExtensionLess absolute path file name without extension
//param isLexiconWriter true if lexicon writer, false if lexicon reader
//param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
//param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
NindLexiconAmose::NindLexiconAmose(const string &fileNameExtensionLess,
                                   const bool isLexiconWriter,
                                   const unsigned int indirectionBlocSize,
                                   const unsigned int retroIndirectionBlocSize):
    NindLexiconIndex(fileNameExtensionLess,
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
//brief add specified word in lexicon if it doesn't still exist in,
//In all cases, word ident is returned.
//param lemma word to be lexiced. Compound word is structured with "_"
//param type type of the words (SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY)
//param namedEntity type of named entity, eventually
//return ident of word */
unsigned int NindLexiconAmose::addWord(const string &lemma,
                                       const AmoseTypes type,
                                       const std::string &namedEntity)
{
    if (type == ALL) return 0;
    list<string> simpleWords;
    splitWord(lemma, type, namedEntity, simpleWords);
    return NindLexiconIndex::addWord(simpleWords);
}
////////////////////////////////////////////////////////////
//brief get ident of a specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param lemma word to be searched. Compound word is structured with "_"
//param type type of the words (SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY)
//param namedEntity type of named entity, eventually
//return ident of word */
unsigned int NindLexiconAmose::getWordId(const string &lemma,
                                         const AmoseTypes type,
                                         const string &namedEntity)
{
    if (type == ALL) return 0;
    list<string> simpleWords;
    splitWord(lemma, type, namedEntity, simpleWords);
    return NindLexiconIndex::getWordId(simpleWords);
}
////////////////////////////////////////////////////////////
//brief get word components from a specified word id
//param lemma word corresponding to word id. Compound word is structured with "_"
//param type type of the words (SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY)
//param namedEntity type of named entity, eventually
//return true if word exists, false otherwise */
bool NindLexiconAmose::getWord(const unsigned int wordId,
                               string &lemma,
                               AmoseTypes &type,
                               string &namedEntity)
{
    //rejcupehre la liste des composants
    list<string> components;
    const bool trouvej = getComponents(wordId, components);
    if (!trouvej) return false;
    if (components.size() == 0) throw NindLexiconException("empty word");
    if (components.size() == 1) type = SIMPLE_TERM;
    else type = MULTI_TERM;
    list<string>::const_iterator itcomp = components.begin();
    string component = (*itcomp++);
    namedEntity = "";
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
//met en forme le mot pour interroger le lexique
static void splitWord(const string &lemma,
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
