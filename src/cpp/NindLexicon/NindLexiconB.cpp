//
// C++ Implementation: NindLexiconB
//
// Description: La gestion du lexique en memoire, solution alternative symétrique avec maps classiques
// VERSION ECARTEE
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.3 Solution alternative : identifiants différents pour les chaînes et les mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconB.h"
#include <list>
#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconB. */
NindLexiconB::NindLexiconB():
    m_SWcurrentId(0),
    m_CWcurrentId(0),
    m_lexiconSW(),
    m_retrolexiconSW(),
    m_lexiconCW(),
    m_retrolexiconCW()
{
}
////////////////////////////////////////////////////////////
NindLexiconB::~NindLexiconB()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param word simple or compound word in string (compound word separator is '#') */
unsigned int NindLexiconB::addWord(const string &word)
{
    //flag pour eviter les recherches inutiles sur les mots composes quand il y a eu insertion d'un sous ensemble
    bool isNew = false;
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    //bool firstWord = true;
    //decompose le mot compose (un mot simple est un mot compose a 1 element)
    list<string> simpleWords;
    split(word, simpleWords);
    for (list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
        const string &simpleWord = *swIt;
        unsigned int simpleWordId;
        unsigned int compoundWordId;
        //mot deja dans le lexique ?
        const map<string, unsigned int>::const_iterator idSWIt = m_retrolexiconSW.find(simpleWord);
        if (idSWIt != m_retrolexiconSW.end()) {
            //deja dans le lexique, on prend son id
            simpleWordId = idSWIt->second;
            const pair<unsigned int, unsigned int> compoundWord(0, simpleWordId);
            const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_retrolexiconCW.find(compoundWord);
            compoundWordId = idCWIt->second;
        }
        else {
            //mot nouveau, on l'insere
            isNew = true;
            m_retrolexiconSW[simpleWord] = ++m_SWcurrentId;
            m_lexiconSW[m_SWcurrentId] = simpleWord;
            simpleWordId = m_SWcurrentId;
            const pair<unsigned int, unsigned int> compoundWord(0, simpleWordId);
            m_retrolexiconCW[compoundWord] = ++m_CWcurrentId;
            m_lexiconCW[m_CWcurrentId] = compoundWord;
            compoundWordId = m_CWcurrentId;
        }
        if (subWordId == 0) {
            //c'est un mot simple, le prochain coup, il sera compose
            subWordId = compoundWordId;
        }
        else {
            //c'est bien un mot compose
            const pair<unsigned int, unsigned int> compoundWord(subWordId, simpleWordId);
            //si nouvel element dans sa structure, pas la peine de le chercher, il n'y est pas
            if (!isNew) {
                const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_retrolexiconCW.find(compoundWord);
                if (idCWIt != m_retrolexiconCW.end()) {
                    //deja dans le lexique, on prend son id
                    subWordId = idCWIt->second;
                }
                else {
                    isNew = true;
                }
            }
            if (isNew) {
                //mot nouveau, on l'insere
                m_retrolexiconCW[compoundWord] = ++m_CWcurrentId;
                m_lexiconCW[m_CWcurrentId] = compoundWord;
                subWordId = m_CWcurrentId;
            }
        }
    }
/*    cerr<<word<<endl;
    for (map<unsigned int, string>::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        cerr<<it->first<<":"<<it->second<<", ";
    }
    cerr<<endl;
    for (map<string, unsigned int>::const_iterator it = m_retrolexiconSW.begin(); it != m_retrolexiconSW.end(); it++) {
        cerr<<it->first<<":"<<it->second<<", ";
    }
    cerr<<endl;
    for (map<unsigned int, pair<unsigned int, unsigned int> >::const_iterator it = m_lexiconCW.begin(); it != m_lexiconCW.end(); it++) {
        cerr<<it->first<<":("<<it->second.first<<", "<<it->second.second<<"), ";
    }
    cerr<<endl;
    for (map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator it = m_retrolexiconCW.begin(); it != m_retrolexiconCW.end(); it++) {
        cerr<<"("<<it->first.first<<", "<<it->first.second<<"):"<<it->second<<", ";
    }
    cerr<<endl;*/
    
    //retourne l'id du mot specifie
    return subWordId;
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param word simple or compound word in string (compound word separator is '#') */
unsigned int NindLexiconB::getId(const string &word) const
{
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    //decompose le mot compose (un mot simple est un mot compose a 1 element)
    list<string> simpleWords;
    split(word, simpleWords);
    for(list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
        const string &simpleWord = *swIt;
        unsigned int simpleWordId;
        unsigned int compoundWordId;
        //mot dans le lexique ?
        const map<string, unsigned int>::const_iterator idSWIt = m_retrolexiconSW.find(simpleWord);
        if (idSWIt != m_retrolexiconSW.end()) {
            //dans le lexique, on prend son id
            simpleWordId = idSWIt->second;
            const pair<unsigned int, unsigned int> compoundWord(0, simpleWordId);
            const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_retrolexiconCW.find(compoundWord);
            compoundWordId = idCWIt->second;
        }
        else {
            //mot inconnu
            return 0;
        }
        //recherche du mot compose eventuel
        if (subWordId == 0) {
            //c'est un mot simple, le prochain coup, il sera compose
            subWordId = compoundWordId;
        }
        else {
            //c'est bien un mot compose
            const pair<unsigned int, unsigned int> compoundWord(subWordId, simpleWordId);
            const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_retrolexiconCW.find(compoundWord);
            if (idCWIt != m_retrolexiconCW.end()) {
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
string NindLexiconB::getWord(const unsigned int id) const
{
    //ce ne peut etre qu'un mot compose
    map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = m_lexiconCW.find(id);
    if (idCWIt == m_lexiconCW.end()) {
        //identifiant inconnu, on retourne ""
        return string("");
    }
    //c'est un mot compose
    list<string> simpleWords;
    while (true) {
        const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
        //ajoute le mot simple a la description
        map<unsigned int, std::string>::const_iterator idSWIt = m_lexiconSW.find(compoundWord.second);
        simpleWords.push_front(idSWIt->second);
        //le composant est un mot simple ou compose ?
        if (compoundWord.first == 0) {
            //mot simple
            break; //termine
        }
        //mot compose
        idCWIt = m_lexiconCW.find(compoundWord.first);
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
bool NindLexiconB::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
{
    //nombre de mots simples et composes
    lexiconSizes.swNb = m_lexiconSW.size();
    lexiconSizes.rswNb = m_retrolexiconSW.size();
    lexiconSizes.cwNb = m_lexiconCW.size();
    lexiconSizes.rcwNb = m_retrolexiconCW.size();
    //les maps directes et inverses doivent etre de meme taille
    if (lexiconSizes.swNb != lexiconSizes.rswNb) return false;
    if (lexiconSizes.cwNb != lexiconSizes.rcwNb) return false;
    //compteurs d'acces
    lexiconSizes.successCount = 0;
    lexiconSizes.failCount = 0;
    //verifie integrite
    for (map<unsigned int, pair<unsigned int, unsigned int> >::const_iterator cwIt = m_lexiconCW.begin();
         cwIt != m_lexiconCW.end(); cwIt++) {
        const unsigned int id = (*cwIt).first;  //l'id a verifier
        //verification
        map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = m_lexiconCW.find(id);
        if (idCWIt == m_lexiconCW.end()) return false;
        lexiconSizes.successCount++;
        while (true) {
            const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
            map<unsigned int, std::string>::const_iterator idSWIt = m_lexiconSW.find(compoundWord.second);
            if (idSWIt == m_lexiconSW.end()) return false;
            lexiconSizes.successCount++;
            //le composant est un mot simple ou compose ?
            if (compoundWord.first == 0) break;   //mot simple, ok pour celui la
            //mot compose
            idCWIt = m_lexiconCW.find(compoundWord.first);
            if (idCWIt == m_lexiconCW.end()) return false;
            lexiconSizes.successCount++;
        }
    }
    return true;
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



