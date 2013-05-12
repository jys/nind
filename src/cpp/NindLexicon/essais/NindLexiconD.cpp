//
// C++ Implementation: NindLexiconD
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.6 Réduction du lexique à une version utile pour l'indexation et la recherche
// §6.7.1 Avec maps classiques
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconD.h"
#include <list>
//#include <iostream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconD. */
NindLexiconD::NindLexiconD():
    m_currentId(0),
    m_lexiconSW(),
    m_lexiconCW()
{
}
////////////////////////////////////////////////////////////
NindLexiconD::~NindLexiconD()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconD::addWord(const list<string> &componants)
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
unsigned int NindLexiconD::getId(const list<string> &componants) const
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
//brief check lexicon integrity and return counts and statistics
//param swNb where number of simple words is returned
//param cwNb where number of compound words is returned
//return true if integrity is ok, false elsewhere */
bool NindLexiconD::integrityAndCounts(struct LexiconSizes &lexiconSizes) const
{
    //nombre de mots simples et composes
    lexiconSizes.swNb = m_lexiconSW.size();
    lexiconSizes.cwNb = m_lexiconCW.size();
    //pour verifier l'integrite, commence par construire les maps inverses
    map<unsigned int, string> retrolexiconSW;  //lexique inverse des mots simples
    for (map<string, unsigned int>::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        retrolexiconSW[it->second] = it->first;
    }
    map<unsigned int, pair<unsigned int, unsigned int> > retrolexiconCW;  //lexique inverse des mots composes
    for (map<pair<unsigned int, unsigned int>, unsigned int >::const_iterator it = m_lexiconCW.begin();
         it != m_lexiconCW.end(); it++) {
        retrolexiconCW[it->second] = it->first;
    }
    //reconstitue chaque mot compose, en partant de l'id, retrouve les composants en chaine
    for (map<unsigned int, pair<unsigned int, unsigned int> >::const_iterator cwIt = retrolexiconCW.begin();
         cwIt != retrolexiconCW.end(); cwIt++) {
        const unsigned int id = (*cwIt).first;  //l'id a verifier
        list<string> simpleWords; //les composants ordonnes du mot a elaborer
        //verification
        map<unsigned int, std::pair<unsigned int, unsigned int> >::const_iterator idCWIt = retrolexiconCW.find(id);
        if (idCWIt == retrolexiconCW.end()) return false;
        while (true) {
            const pair<unsigned int, unsigned int> &compoundWord = idCWIt->second;
            map<unsigned int, string>::const_iterator idSWIt = retrolexiconSW.find(compoundWord.second);
            if (idSWIt == retrolexiconSW.end()) return false;   //mot simple inconnu, erreur integrite
            simpleWords.push_front(idSWIt->second);  //le mot simple en tete
            //le composant est un mot simple ou compose ?
            idSWIt = retrolexiconSW.find(compoundWord.first);
            if (idSWIt != retrolexiconSW.end()) {
                simpleWords.push_front(idSWIt->second);  //le mot simple en tete
                break;   //mot simple, ok pour celui la
            }
            //mot compose
            idCWIt = retrolexiconCW.find(compoundWord.first);
            if (idCWIt == retrolexiconCW.end()) return false;   //mot compose inconnu, erreur integrite
        }
        //nous avons maintenant l'id et le mot, en partant des composants, retrouve l'id et compte les acces
        if (id != getId(simpleWords)) return false;   //erreur integrite
/*        unsigned int subWordId = 0;
        for(list<string>::const_iterator swIt = simpleWords.begin(); swIt != simpleWords.end(); swIt++) {
            const string &simpleWord = *swIt;
            //mot dans le lexique ?
            const map<string, unsigned int>::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
            if (idSWIt == m_lexiconSW.end()) return false;   //mot simple inconnu, erreur integrite
            lexiconSizes.successCount++;
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
                if (idCWIt == m_lexiconCW.end()) return false;   //mot compose inconnu, erreur integrite
                //il est dans le lexique, on prend son id
                subWordId = idCWIt->second;
            }
        }
        //compare l'id de depart et celui d'arrivee
        if (id != subWordId) return false;   //erreur integrite*/
    }
    return true;
}
////////////////////////////////////////////////////////////
//\brief dump full lexicon on specified ostream
//param out ostream where to dump */
void NindLexiconD::dump(std::ostream &out)
{
    string sep = "";
    for (map<string, unsigned int>::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
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



