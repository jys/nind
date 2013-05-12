//
// C++ Implementation: NindLexiconA
//
// Description: La gestion du lexique en memoire : solution 1 symétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.2 Solution : mêmes identifiants pour les chaînes et les mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconA.h"
#include <list>
//#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
//brief Creates NindLexiconA. */
NindLexiconA::NindLexiconA():
    m_currentId(0),
    m_lexiconSW(),
    m_retrolexiconSW(),
    m_lexiconCW(),
    m_retrolexiconCW()
{
}
////////////////////////////////////////////////////////////
NindLexiconA::~NindLexiconA()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconA::addWord(const list<string> &componants)
{
    //flag pour eviter les recherches inutiles sur les mots composes quand il y a eu insertion d'un sous ensemble
    bool isNew = false;
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
        const string &simpleWord = *swIt;
        unsigned int simpleWordId;
        //mot deja dans le lexique ?
        const map<string, unsigned int>::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
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
                const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
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
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconA::getId(const list<string> &componants) const
{
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int subWordId = 0;
    for(list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
        const string &simpleWord = *swIt;
        //mot dans le lexique ?
        const map<string, unsigned int>::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
        if (idSWIt == m_lexiconSW.end()) return 0; //mot inconnu
        //dans le lexique, on prend son id
        const unsigned int simpleWordId = idSWIt->second;
        //recherche du mot compose eventuel
        if (subWordId == 0) {
            //c'est un mot simple, le prochain coup, il sera compose
            subWordId = simpleWordId;
        }
        else {
            //c'est bien un mot compose
            const pair<unsigned int, unsigned int> compoundWord(subWordId, simpleWordId);
            const map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
            if (idCWIt == m_lexiconCW.end()) return 0; //mot inconnu
            //il est dans le lexique, on prend son id
            subWordId = idCWIt->second;
        }
    }
    //retourne l'id du mot specifie
    return subWordId;    
}
////////////////////////////////////////////////////////////
//brief get word of the specified ident
//if ident exists in lexicon, corresponding word is returned (compound word separator is '#')
//else, return empty string (empty string is not a valid word !)
//param id ident as number 
//param componants list of componants of the word (list size means 0 : unknown id, 1 : simple word, more : compound word) */
void NindLexiconA::getWord(const unsigned int id, list<string> &componants) const
{
    componants.clear();
    //regarde d'abord si c'est un mot simple
    map<unsigned int, std::string>::const_iterator idSWIt = m_retrolexiconSW.find(id);
    if (idSWIt != m_retrolexiconSW.end()) {
        //dans le lexique, on retourne le mot simple
        componants.push_front(idSWIt->second);
        return; //termine
    }
    //c'est peut-etre un mot compose
    map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = m_retrolexiconCW.find(id);
    if (idCWIt == m_retrolexiconCW.end()) {
        //identifiant inconnu, on retourne ""
        return; //termine, retourne liste vide
    }
    //c'est un mot compose
    while (true) {
        const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
        //ajoute le mot simple a la description
        idSWIt = m_retrolexiconSW.find(compoundWord.second);
        componants.push_front(idSWIt->second);
        //le composant est un mot simple ou compose ?
        idSWIt = m_retrolexiconSW.find(compoundWord.first);
        if (idSWIt != m_retrolexiconSW.end()) {
            //mot simple
            componants.push_front(idSWIt->second);  //le mot simple premier
            break; //termine
        }
        //mot compose
        idCWIt = m_retrolexiconCW.find(compoundWord.first);
    }
}
////////////////////////////////////////////////////////////
//brief check lexicon integrity and return counts and statistics
//param swNb where number of simple words is returned
//param cwNb where number of compound words is returned
//return true if integrity is ok, false elsewhere */
bool NindLexiconA::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
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
void NindLexiconA::dump(std::ostream &out)
{
    string sep = "";
    for (map<unsigned int, string>::const_iterator it = m_retrolexiconSW.begin(); it != m_retrolexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
        sep = ", ";
    }
    out<<endl;
    sep = "";
    for (map<string, unsigned int>::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
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
    for (map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator it = m_lexiconCW.begin(); it != m_lexiconCW.end(); it++) {
        out<<sep<<"("<<it->first.first<<", "<<it->first.second<<"):"<<it->second;
        sep = ", ";
    }
    out<<endl;
}
////////////////////////////////////////////////////////////



