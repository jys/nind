//
// C++ Implementation: NindLexicon
//
// Description: La gestion du lexique en memoire version hash_map
//
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconC.h"
#include <list>
//#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexicon. */
NindLexiconC::NindLexiconC():
    m_currentId(0),
    m_lexiconSW(),
    m_retrolexiconSW(),
    m_lexiconCW(),
    m_retrolexiconCW()
{
}
////////////////////////////////////////////////////////////
NindLexiconC::~NindLexiconC()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param word simple or compound word in string (compound word separator is '#') */
unsigned int NindLexiconC::addWord(const string &word)
{
    //flag pour eviter les recherches inutiles sur les mots composes quand il y a eu insertion d'un sous ensemble
    bool isNew = false;
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    //decompose le mot compose (un mot simple est un mot compose a 1 element)
    list<string> simpleWords;
    split(word, simpleWords);
    for (list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
        const string &simpleWord = *swIt;
        unsigned int simpleWordId;
        //mot deja dans le lexique ?
        const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
        if (idSWIt != m_lexiconSW.end()) {
            //deja dans le lexique, on prend son id
            simpleWordId = idSWIt->second;
        }
        else {
            //mot nouveau, on l'insere
            isNew = true;
            m_lexiconSW[simpleWord] = ++m_currentId;
            m_retrolexiconSW[m_currentId] = simpleWord;
            simpleWordId = m_currentId;
        }
        //enregistrement du mot compose eventuel
        if (subWordId == 0) {
            //c'est un mot simple, le prochain coup, il sera compose
            subWordId = simpleWordId;
        }
        else {
            //c'est bien un mot compose
            const pair<unsigned int, unsigned int> compoundWord(subWordId, simpleWordId);
            //si nouvel element dans sa structure, pas la peine de le chercher, il n'y est pas
            if (!isNew) {
                PairHashMap::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
                if (idCWIt != m_lexiconCW.end()) {
                    //deja dans le lexique, on prend son id
                    subWordId = idCWIt->second;
                }
                else {
                    isNew = true;
                }
            }
            if (isNew) {
                //mot nouveau, on l'insere
                m_lexiconCW[compoundWord] = ++m_currentId;
                m_retrolexiconCW[m_currentId] = compoundWord;
                subWordId = m_currentId;
            }
        }
    }
    //retourne l'id du mot specifie
    return subWordId;
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param word simple or compound word in string (compound word separator is '#') */
unsigned int NindLexiconC::getId(const string &word) const
{
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    //decompose le mot compose (un mot simple est un mot compose a 1 element)
    list<string> simpleWords;
    split(word, simpleWords);
    for(list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
        const string &simpleWord = *swIt;
        unsigned int simpleWordId;
        //mot dans le lexique ?
        const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
        if (idSWIt != m_lexiconSW.end()) {
            //dans le lexique, on prend son id
            simpleWordId = idSWIt->second;
        }
        else {
            //mot inconnu
            return 0;
        }
        //recherche du mot compose eventuel
        if (subWordId == 0) {
            //c'est un mot simple, le prochain coup, il sera compose
            subWordId = simpleWordId;
        }
        else {
            //c'est bien un mot compose
            const pair<unsigned int, unsigned int> compoundWord(subWordId, simpleWordId);
            PairHashMap::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
            if (idCWIt != m_lexiconCW.end()) {
                //il est dans le lexique, on prend son id
                subWordId = idCWIt->second;
            }
            else {
                //mot inconnu
                return 0;
            }
        }
    }
    //retourne l'id du mot specifie
    return subWordId;
}
////////////////////////////////////////////////////////////
//brief get word of the specified ident
//if ident exists in lexicon, corresponding word is returned (compound word separator is '#')
//else, return empty string (empty string is not a valid word !)
//param id ident as number */
string NindLexiconC::getWord(const unsigned int id) const
{
    //regarde d'abord si c'est un mot simple
    map<unsigned int, std::string>::const_iterator idSWIt = m_retrolexiconSW.find(id);
    if (idSWIt != m_retrolexiconSW.end()) {
        //dans le lexique, on retourne le mot simple
        return idSWIt->second;
    }
    //c'est peut-etre un mot compose
    map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = m_retrolexiconCW.find(id);
    if (idCWIt == m_retrolexiconCW.end()) {
        //identifiant inconnu, on retourne ""
        return string("");
    }
    //c'est un mot compose
    list<string> simpleWords;
    while (true) {
        const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
        //ajoute le mot simple a la description
        idSWIt = m_retrolexiconSW.find(compoundWord.second);
        simpleWords.push_front(idSWIt->second);
        //le composant est un mot simple ou compose ?
        idSWIt = m_retrolexiconSW.find(compoundWord.first);
        if (idSWIt != m_retrolexiconSW.end()) {
            //mot simple
            simpleWords.push_front(idSWIt->second);  //le mot simple premier
            break; //termine
        }
        //mot compose
        idCWIt = m_retrolexiconCW.find(compoundWord.first);
    }
    //sort le resultat en string, le separateur est '#'
    string word;
    string sep;
    for (list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
        word += sep + *swIt;
        sep = "#";
    }
    return word;
}
////////////////////////////////////////////////////////////
//brief check lexicon integrity and return counts and statistics
//param swNb where number of simple words is returned
//param cwNb where number of compound words is returned
//return true if integrity is ok, false elsewhere */
bool NindLexiconC::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
{
    //nombre de mots simples et composes
    lexiconSizes.swNb = m_retrolexiconSW.size();
    lexiconSizes.rswNb = m_lexiconSW.size();
    lexiconSizes.cwNb = m_retrolexiconCW.size();
    lexiconSizes.rcwNb = m_lexiconCW.size();
    //les maps directes et inverses doivent etre de meme taille
    if (lexiconSizes.swNb != lexiconSizes.rswNb) return false;
    if (lexiconSizes.cwNb != lexiconSizes.rcwNb) return false;
    //compteurs d'acces
    lexiconSizes.successCount = 0;
    lexiconSizes.failCount = 0;
    //verifie integrite
    for (map<unsigned int, pair<unsigned int, unsigned int> >::const_iterator cwIt = m_retrolexiconCW.begin();
         cwIt != m_retrolexiconCW.end(); cwIt++) {
        const unsigned int id = (*cwIt).first;  //l'id a verifier
        //verification
        map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = m_retrolexiconCW.find(id);
        if (idCWIt == m_retrolexiconCW.end()) return false;
        lexiconSizes.successCount++;
        while (true) {
            const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
            map<unsigned int, std::string>::const_iterator idSWIt = m_retrolexiconSW.find(compoundWord.second);
            if (idSWIt == m_retrolexiconSW.end()) return false;
            lexiconSizes.successCount++;
            //le composant est un mot simple ou compose ?
            idSWIt = m_retrolexiconSW.find(compoundWord.first);
            if (idSWIt != m_retrolexiconSW.end()) {
                lexiconSizes.successCount++;
                break;   //mot simple, ok pour celui la
            }
            lexiconSizes.failCount++;
            //mot compose
            idCWIt = m_retrolexiconCW.find(compoundWord.first);
            if (idCWIt == m_retrolexiconCW.end()) return false;
            lexiconSizes.successCount++;
        }
    }
    return true;
}
////////////////////////////////////////////////////////////
//\brief dump full lexicon on specified ostream
//param out ostream where to dump */
void NindLexiconC::dump(std::ostream &out)
{
    string sep = "";
    for (map<unsigned int, string>::const_iterator it = m_retrolexiconSW.begin(); it != m_retrolexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
        sep = ", ";
    }
    out<<endl;
    sep = "";
    for (StringHashMap::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
        sep = ", ";
    }
    out<<endl;
    sep = "";
    for (map<unsigned int, pair<unsigned int, unsigned int> >::const_iterator it = m_retrolexiconCW.begin(); it != m_retrolexiconCW.end(); it++) {
        out<<sep<<it->first<<":("<<it->second.first<<", "<<it->second.second<<")";
        sep = ", ";
    }
    out<<endl;
    sep = "";
    for (PairHashMap::const_iterator it = m_lexiconCW.begin(); it != m_lexiconCW.end(); it++) {
        out<<sep<<"("<<it->first.first<<", "<<it->first.second<<"):"<<it->second;
        sep = ", ";
    }
    out<<endl;
}
////////////////////////////////////////////////////////////
//decoupe le mot sur les '#' et retourne une liste ordonnee de mots simples
static void split(const string &word, list<string> &simpleWords)
{
    simpleWords.clear();
    size_t posDeb = 0;
    while (true) {
        const size_t posSep = word.find('#', posDeb);
        simpleWords.push_back(word.substr(posDeb, posSep-posDeb));
        if (posSep == string::npos) break;
        posDeb = posSep + 1;
    }
}
////////////////////////////////////////////////////////////
size_t NindLexiconC::HashString::operator()(const string &s) const
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

bool NindLexiconC::EqualString::operator()(const string &s1, const string &s2) const
{
    return (s1 == s2);
};
////////////////////////////////////////////////////////////
size_t NindLexiconC::HashPair::operator()(const pair<unsigned int, unsigned int> &p) const
{
    unsigned int key = 0x55555555;        //0101...
    key ^= p.first;
    key ^= (p.second<<16);
    return key;
};

bool NindLexiconC::EqualPair::operator()(const pair<unsigned int, unsigned int> &p1, const pair<unsigned int, unsigned int> &p2) const
{
    return (p1 == p2);
};



