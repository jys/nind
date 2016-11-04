//
// C++ Implementation: NindLexicon
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revB
// §6.6 Réduction du lexique à une version utile pour l'indexation et la recherche
// §6.7.3 Avec deux hash_maps
// Étude et maquette d'un lexique complet ANT2012.JYS.R357 revA
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: 2014-2015 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindLexicon.h"
#include <time.h>
#include <list>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief This class maintains correspondance between words and their indentifiant
////////////////////////////////////////////////////////////
//brief Creates NindLexicon. */
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  */
NindLexicon::NindLexicon(const std::string &fileName,
                         const bool isLexiconWriter)
    throw(NindLexiconException) :
    m_isLexiconWriter(isLexiconWriter),
    m_fileName(fileName),
    m_currentId(0),
    m_identification(0),
    m_lexiconSW(),
    m_lexiconCW(),
    m_lexiconFile(fileName, isLexiconWriter),
    m_nextRefreshTime(0)
{
    //cerr<<"NindLexicon::NindLexicon start"<<endl;
    //initialisation des maps depuis le fichier
    try {
      // throws EofException, ReadFileException, InvalidFileException, OutReadBufferException
      updateFromFile();
    } 
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconException(m_fileName);
    }
    m_nextRefreshTime = (time_t)time(NULL) + 10;  //temps actuel + 10s
    //cerr<<"NindLexicon::NindLexicon end"<<endl;
}
////////////////////////////////////////////////////////////
NindLexicon::~NindLexicon()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident
//if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexicon::addWord(const list<string> &componants)
    throw(NindLexiconException)
{
    try {
        if (!m_isLexiconWriter) throw BadUseException("lexicon is not writable");
        //flag pour eviter les recherches inutiles sur les mots composes quand il y a eu insertion d'un sous ensemble
        bool isNew = false;
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
        unsigned int subWordId = 0;
        for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
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
                simpleWordId = m_currentId;
                m_identification = (time_t)time(NULL);
                m_lexiconFile.writeSimpleWordDefinition(m_currentId, simpleWord, m_currentId, m_identification);
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
                    const PairHashMap::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
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
                    m_identification = (time_t)time(NULL);
                    m_lexiconFile.writeCompoundWordDefinition(m_currentId, compoundWord, m_currentId, m_identification);
                }
            }
        }
        //retourne l'id du mot specifie
        return subWordId;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexicon::getId(const list<string> &componants) 
    throw(NindLexiconException)
{
    try {
        //si lecteur, verifie s'il y a eu maj
        if (!m_isLexiconWriter && m_nextRefreshTime < (time_t)time(NULL)) {
            updateFromFile();
            m_nextRefreshTime = (time_t)time(NULL) + 10;  //temps actuel + 10s
        }
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
        unsigned int subWordId = 0;
        for(list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
            const string &simpleWord = *swIt;
            //mot dans le lexique ?
            const StringHashMap::const_iterator idSWIt = m_lexiconSW.find(simpleWord);
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
                const PairHashMap::const_iterator idCWIt = m_lexiconCW.find(compoundWord);
                if (idCWIt == m_lexiconCW.end()) return 0; //mot inconnu
                //il est dans le lexique, on prend son id
                subWordId = idCWIt->second;
            }
        }
        //retourne l'id du mot specifie
        return subWordId;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get identification of lexicon
//param wordsNb where number of words contained in lexicon is returned
//param identification where unique identification of lexicon is returned */
void NindLexicon::getIdentification(unsigned int &wordsNb, unsigned int &identification)
{
    wordsNb = m_currentId;
    identification = m_identification;
}
////////////////////////////////////////////////////////////
//brief check lexicon integrity and return counts and statistics
//param lexiconChar where lexicon characteristics are returned
//return true if integrity is ok, false elsewhere */
bool NindLexicon::integrityAndCounts(struct LexiconChar &lexiconChar)
{
    //nombre de mots simples et composes
    lexiconChar.isOk = false;
    lexiconChar.swNb = m_lexiconSW.size();
    lexiconChar.cwNb = m_lexiconCW.size();
    lexiconChar.wordsNb = m_currentId;
    lexiconChar.identification = m_identification;
    //si le lexique est trop gros, ca prend des plombes
    if (m_currentId > 1000000) return true;
    //pour verifier l'integrite, commence par construire les maps inverses
    map<unsigned int, string> retrolexiconSW;  //lexique inverse des mots simples
    for (StringHashMap::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        retrolexiconSW[it->second] = it->first;
    }
    map<unsigned int, pair<unsigned int, unsigned int> > retrolexiconCW;  //lexique inverse des mots composes
    for (PairHashMap::const_iterator it = m_lexiconCW.begin();
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
    }
    lexiconChar.isOk = true;
    return true;
}
////////////////////////////////////////////////////////////
//\brief dump full lexicon on specified ostream
//param out ostream where to dump */
void NindLexicon::dump(std::ostream &out)
{
    string sep = "";
    for (StringHashMap::const_iterator it = m_lexiconSW.begin(); it != m_lexiconSW.end(); it++) {
        out<<sep<<it->first<<":"<<it->second;
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
//met a jour le lexique lecteur avec le fichier lexique
void NindLexicon::updateFromFile()
    throw(EofException, ReadFileException, InvalidFileException, OutReadBufferException)
{
    //fait un clear du buffer du fichier, sinon, ca ne bougera jamais
    m_lexiconFile.clearBuffer();
    while (true) {
        unsigned int ident;
        bool isSimpleWord;
        string simpleWord;
        pair<unsigned int, unsigned int> compoundWord;
        const bool isWord = m_lexiconFile.readNextRecordAsWordDefinition(ident, isSimpleWord, simpleWord, compoundWord);
        if (!isWord) break;   //fin du fichier atteinte
        if (isSimpleWord) m_lexiconSW[simpleWord] = ident;
        else m_lexiconCW[compoundWord] = ident;
    }
    //recupere le max des identifiants
    const bool isIdentification = m_lexiconFile.readNextRecordAsLexiconIdentification(m_currentId, m_identification);
    if (!isIdentification) 
    {
      std::cerr << "NindLexicon::updateFromFile error reading lexicon identification. Got " 
                << m_currentId << " and " << m_identification << std::endl;
      throw InvalidFileException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
size_t NindLexicon::HashString::operator()(const string &s) const
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
#ifdef _MSC_VER
bool NindLexicon::HashString::operator()(const std::string &s1, const std::string &s2) const
{
    return (s1 < s2);
};
#else
bool NindLexicon::EqualString::operator()(const string &s1, const string &s2) const
{
    return (s1 == s2);
};
#endif
////////////////////////////////////////////////////////////
size_t NindLexicon::HashPair::operator()(const pair<unsigned int, unsigned int> &p) const
{
    unsigned int key = 0x55555555;        //0101...
    key ^= p.first;
    key ^= (p.second<<16);
    return key;
};

#ifdef _MSC_VER
bool NindLexicon::HashPair::operator()(const pair<unsigned int, unsigned int> &p1, const pair<unsigned int, unsigned int> &p2) const
{
    return (p1 < p2);
};
#else
bool NindLexicon::EqualPair::operator()(const pair<unsigned int, unsigned int> &p1, const pair<unsigned int, unsigned int> &p2) const
{
    return (p1.first == p2.first && p1.second == p2.second);
};
#endif
////////////////////////////////////////////////////////////
