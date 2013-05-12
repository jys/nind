//
// C++ Implementation: NindLexiconG
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.6 Réduction du lexique à une version utile pour l'indexation et la recherche
// §6.7.3 Avec deux hash_maps
// §6.8 Occupation mémoire sans gestion spécifique des mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconG.h"
#include <list>
//#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconG. */
NindLexiconG::NindLexiconG():
    m_currentId(0),
    m_lexiconSW()
{
}
////////////////////////////////////////////////////////////
NindLexiconG::~NindLexiconG()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconG::addWord(const list<string> &componants)
{
    //reconstitue le mot compose
    string word;
    string sep = "";
    for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
        word +=  sep + (*swIt);
        sep = "#";
    }
    const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(word);
    if (idSWIt != m_lexiconSW.end()) {
        //deja dans le lexique, on prend son id
        return idSWIt->second;
    }
    else {   
        m_lexiconSW[word] = ++m_currentId;
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
unsigned int NindLexiconG::getId(const list<string> &componants) const
{
    //reconstitue le mot compose
    string word;
    string sep = "";
    for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
        word +=  sep + (*swIt);
        sep = "#";
    }
    const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(word);
    if (idSWIt == m_lexiconSW.end()) return 0; //mot inconnu
    return idSWIt->second;
}
////////////////////////////////////////////////////////////
//brief check lexicon integrity and return counts and statistics
//param swNb where number of simple words is returned
//param cwNb where number of compound words is returned
//return true if integrity is ok, false elsewhere */
bool NindLexiconG::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
{
    //nombre de mots simples et composes
    lexiconSizes.swNb = m_lexiconSW.size();
    lexiconSizes.cwNb = 0;
    return true;
}
////////////////////////////////////////////////////////////
//\brief dump full lexicon on specified ostream
//param out ostream where to dump */
void NindLexiconG::dump(std::ostream &out)
{
    string sep = "";
    for (StringHashMap::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
        sep = ", ";
    }
    out<<endl;
}
////////////////////////////////////////////////////////////
size_t NindLexiconG::HashString::operator()(const string &s) const
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

bool NindLexiconG::EqualString::operator()(const string &s1, const string &s2) const
{
    return (s1 == s2);
};
////////////////////////////////////////////////////////////



