//
// C++ Implementation: NindLexiconH
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.8 Occupation mémoire sans gestion spécifique des mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconH.h"
#include <list>
//#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconH. */
NindLexiconH::NindLexiconH():
    m_currentId(0),
    m_lexiconSW(),
    m_retrolexiconSW()
{
}
////////////////////////////////////////////////////////////
NindLexiconH::~NindLexiconH()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconH::addWord(const string &word)
{
    const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(word);
    if (idSWIt != m_lexiconSW.end()) {
        //deja dans le lexique, on prend son id
        return idSWIt->second;
    }
    else {
        m_lexiconSW[word] = ++m_currentId;
        m_retrolexiconSW[m_currentId] = word;
        //retourne l'id du mot specifie
        return m_currentId;
    }
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconH::getId(const string &word) const
{
    const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(word);
    if (idSWIt == m_lexiconSW.end()) return 0; //mot inconnu
    return idSWIt->second;
}
////////////////////////////////////////////////////////////
//brief get word of the specified ident
//if ident exists in lexicon, corresponding word is returned (compound word separator is '#')
//else, return empty string (empty string is not a valid word !)
//param id ident as number */
string NindLexiconH::getWord(const unsigned int id) const
{
    //regarde d'abord si c'est un mot simple
    map<unsigned int, std::string>::const_iterator idSWIt = m_retrolexiconSW.find(id);
    if (idSWIt != m_retrolexiconSW.end()) {
        //dans le lexique, on retourne le mot simple
        return idSWIt->second;
    }
    return string("");
}
////////////////////////////////////////////////////////////
//brief check lexicon integrity and return counts and statistics
//param swNb where number of simple words is returned
//param cwNb where number of compound words is returned
//return true if integrity is ok, false elsewhere */
bool NindLexiconH::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
{
    //nombre de mots simples et composes
    lexiconSizes.swNb = m_lexiconSW.size();
    lexiconSizes.cwNb = 0;
    return true;
}
////////////////////////////////////////////////////////////
//\brief dump full lexicon on specified ostream
//param out ostream where to dump */
void NindLexiconH::dump(std::ostream &out)
{
    string sep = "";
    for (StringHashMap::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
        sep = ", ";
    }
    out<<endl;
}
////////////////////////////////////////////////////////////
size_t NindLexiconH::HashString::operator()(const string &s) const
{
    const int shift[] = {0,8,16,24};    // 4 shifts to "occupy" 32 bits
    unsigned int key = 0x55555555;        //0101...
    unsigned int oneChar;
    int depth = s.length();
    if (depth > 25)
        depth = 25;
    for (int i=0; i<depth; i++) {
        oneChar = ((unsigned int) (s[i]) )<<shift[i%4];
        key^=oneChar;                    // exclusive or
    }
    return key;
};

bool NindLexiconH::EqualString::operator()(const string &s1, const string &s2) const
{
    return (s1 == s2);
};
////////////////////////////////////////////////////////////



