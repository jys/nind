//
// C++ Implantation: NindRetrolexicon
//
// Description: La gestion du lexique inverse sous forme de fichier plat
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gehre le lexique inverse qui permet de retrouver un mot ah partir de son
// identifiant gejnejrej par le lexique.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
//
// Copyright: 2014-2016 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindRetrolexicon.h"
//#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichier>               ::= { <blocDejfinition> <blocUtf8> } <blocIdentification> 
//
// <blocDejfinition>       ::= <flagDejfinition=43> <addrBlocSuivant> <nombreDejfinitions> { <dejfinitionMot> }
// <flagDejfinition=43>    ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreDejfinitions>    ::= <Integer3>
//
// <dejfinitionMot>        ::= <motComposej> | <motSimple>
// <motComposej>           ::= <flagComposej=31> <identifiantA> <identifiantS>
// <flagComposej=31>       ::= <Integer1>
// <identifiantA>          ::= <Integer3>
// <identifiantS>          ::= <Integer3>
// <motSimple>             ::= <flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
// <flagSimple=37>         ::= <Integer1>
// <longueurMotUtf8>       ::= <Integer1>
// <adresseMotUtf8>        ::= <Integer5>
//
// <blocUtf8>              ::= { <motUtf8> }
// <motUtf8>               ::= { <Octet> }
//
// <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique>
// <flagIdentification=53> ::= <Integer1>
// <maxIdentifiant>        ::= <Integer3>
// <identifieurUnique>     ::= <dateHeure>
// <dateHeure >            ::= <Integer4>
////////////////////////////////////////////////////////////
#define FLAG_DEJFINITION 43
#define FLAG_COMPOSEJ 31
#define FLAG_SIMPLE 37
#define FLAG_IDENTIFICATION 53
//<flagDejfinition=43>(1) <addrBlocSuivant>(5) <nombreDejfinitions>(3) = 9
#define TETE_DEJFINITION 9
//<flagComposej=31>(1) <identifiantA>(3) <identifiantS>(3) = 7
//<flagSimple=37>(1) <longueurMotUtf8>(1) <adresseMotUtf8>(5) = 7
#define TAILLE_DEJFINITION 7
// //<motUtf8>(255) + TAILLE_IDENTIFICATION (12) = 268
#define TAILLE_DEJFINITION_MAXIMUM 268
// //taille maximum d'un mot compose (pour detecter les bouclages)
#define TAILLE_COMPOSE_MAXIMUM 30
////////////////////////////////////////////////////////////
//brief Creates NindRetrolexicon.
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param definitionBlocSize number of entries in a single definition block */
NindRetrolexicon::NindRetrolexicon(const string &fileName,
                                   const bool isLexiconWriter,
                                   const NindIndex::Identification &lexiconIdentification,
                                   const unsigned int definitionBlocSize):
    m_file(fileName),
    m_fileName(fileName),
    m_isWriter(isLexiconWriter),
    m_dejfinitionBlocSize(definitionBlocSize),
    m_dejfinitionMapping()
{
    try {
        if (m_isWriter) {
            //si fichier ecrivain, ouvre en ecriture + lecture
            bool isOpened = m_file.open("r+b");
            if (isOpened) {
                //si le fichier existe, l'analyse pour trouver les dejfinitions et l'identification
                //ejtablit la carte des dejfinitions       
                mapDejfinition();
                //verifie l'apairage avec le lexique
                checkIdentification(lexiconIdentification); 
            }
            else {
                //si le fichier n'existe pas, le creje vide en ejcriture + lecture
                //la taille du bloc de dejfinitions doit estre spejcifieje diffejrente de 0
                if (m_dejfinitionBlocSize == 0) 
                {
                  string errorString = 
                    string("NindRetrolexicon. In write mode, definitionBlocSize "
                                "must be non null. ") + m_fileName;
                  throw BadUseException(errorString);
                }
                isOpened = m_file.open("w+b");
                if (!isOpened) throw OpenFileException(m_fileName);
                //lui colle l'identification du lexique en teste
                addIdentification(lexiconIdentification);
                //ajoute un bloc de dejfinition
                addBlocDejfinition(lexiconIdentification);
            }
        }
        else {
            //si fichier lecteur, ouvre en lecture seule
            bool isOpened = m_file.open("rb");
            if (!isOpened) throw OpenFileException(m_fileName);
            //ejtablit la carte des dejfinitions       
            mapDejfinition();
            //verifie l'apairage avec le lexique
            checkIdentification(lexiconIdentification); 
        }
    }
    catch (FileException &exc) {
        throw NindIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
NindRetrolexicon::~NindRetrolexicon()
{
}
////////////////////////////////////////////////////////////
//brief add a list of word idents in retro lexicon. If one of idents still exists, exception is raised
//param retroWords list of words definitions 
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
void NindRetrolexicon::addRetroWords(const list<struct RetroWord> &retroWords,
                                     const NindIndex::Identification &lexiconIdentification)
{
    try {
        if (!m_isWriter) throw BadUseException("retro lexicon is not writable");
        //il y a autant d'ajout au fichier qu'il y a de nouveaux identifiants
        for (list<struct RetroWord>::const_iterator it1 = retroWords.begin(); it1 != retroWords.end(); it1++) {
            const struct RetroWord &retroWord = (*it1);
            //1) rejcupehre l'adresse de la dejfinition
            unsigned long int dejfinition = getDejfinitionPos(retroWord.identifiant);
            //2) si hors limite, ajoute un bloc de dejfinitions
            if (dejfinition == 0) addBlocDejfinition(lexiconIdentification);
            //et rejcupehre l'adresse de la dejfinition
            dejfinition = getDejfinitionPos(retroWord.identifiant);
            if (dejfinition == 0) throw OutOfBoundException("NindRetrolexicon::addRetroWords : " + m_fileName);
            //3) si mot simple, ejcrit d'abord l'utf8 puis la dejfinition
            if (retroWord.identifiantA == 0) {
                //se positionne ah la fin (sur l'identification)
                m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       
                const unsigned long int utf8Pos = m_file.getPos();
                m_file.createBuffer(TAILLE_DEJFINITION_MAXIMUM);
                m_file.putStringAsBytes(retroWord.motSimple);
                //ajoute la taille de l'identification
                m_file.putPad(TAILLE_IDENTIFICATION);
                m_file.writeBuffer();
                //se positionne sur la dejfinition
                m_file.setPos(dejfinition, SEEK_SET);
                m_file.createBuffer(TAILLE_DEJFINITION);
                //<flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
                m_file.putInt1(FLAG_SIMPLE);
                m_file.putInt1(retroWord.motSimple.length());
                m_file.putInt5(utf8Pos);
                m_file.writeBuffer();
            }
            //si mot composej, ejcrit la dejfinition
            else {
                //se positionne sur la dejfinition
                m_file.setPos(dejfinition, SEEK_SET);
                m_file.createBuffer(TAILLE_DEJFINITION);
                //<flagComposej=31> <identifiantA> <identifiantS>
                m_file.putInt1(FLAG_COMPOSEJ);
                m_file.putInt3(retroWord.identifiantA);
                m_file.putInt3(retroWord.identifiantS - retroWord.identifiant);
                m_file.writeBuffer();
            }
        }
        //ejcrit l'identification
        m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END); 
        addIdentification(lexiconIdentification);
    }
    catch (FileException &exc) {
        throw NindRetrolexiconException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get word components from the specified ident
//param ident ident of word
//param components list of components of a word 
//(1 component = simple word, more components = compound word) */
//return true if word was found, false otherwise */
bool NindRetrolexicon::getComponents(const unsigned int ident,
                                     list<string> &components)
{
    //raz resultat
    components.clear();
    //lecture du mot sur le retro lexique
    struct RetroWord retroWord;
    bool existe = getRetroWord(ident, retroWord);
    //si le mot est inconnu, retour false
    if (!existe) return false;
    //le mot existe, on le decode
    while (true) {
        if (retroWord.identifiantA == 0) {
            //c'est un mot simple et c'est la fin 
            components.push_front(retroWord.motSimple);
            return true;
        }
        //c'est un mot compose
        //recupere le mot simple du couple
        struct RetroWord retroWordS;
        existe = getRetroWord(retroWord.identifiantS, retroWordS);
        if (!existe) throw InvalidFileException("NindRetrolexicon::getComponents A : " + m_fileName);
        if (retroWordS.identifiantA != 0) InvalidFileException("NindRetrolexicon::getComponents B : " + m_fileName);
        components.push_front(retroWordS.motSimple);
        //recupere l'autre mot du couple
        existe = getRetroWord(retroWord.identifiantA, retroWord);
        if (!existe) throw InvalidFileException("NindRetrolexicon::getComponents C : " + m_fileName);  
        //pour detecter les bouclages induits par un fichier bouclant
        if (components.size() == TAILLE_COMPOSE_MAXIMUM) throw InvalidFileException("NindRetrolexicon::getComponents D : " + m_fileName);  
    }    
}    
////////////////////////////////////////////////////////////
//Recupere sur le fichier retro lexique la definition d'un mot specifie par son identifiant
//ident identifiant du mot
//retroWord structure ou est ecrite la definition
//retourne true si le mot existe, sinon false
bool NindRetrolexicon::getRetroWord(const unsigned int ident,
                                    struct RetroWord &retroWord)   
{
    try {
        const bool existe = getDejfinition(ident);
        if (!existe) return false;
        const unsigned char flag = m_file.getInt1();
        if (flag == 0) return false;
        //<flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
        else if (flag == FLAG_SIMPLE) {
            const unsigned char longueurMotUtf8 = m_file.getInt1();
            const unsigned long adresseMotUtf8 = m_file.getInt5();
            //se positionne sur la definition
            m_file.setPos(adresseMotUtf8, SEEK_SET);    
            m_file.readBuffer(longueurMotUtf8);
            const string motUtf8 = m_file.getStringAsBytes(longueurMotUtf8);
            retroWord = RetroWord(ident, motUtf8);
            return true;
        }
        //<flagComposej=31> <identifiantA> <identifiantS>
        else if (flag == FLAG_COMPOSEJ) {
            const unsigned int identifiantA = m_file.getInt3();
            const unsigned int identifiantS = ident + m_file.getSInt3();
            retroWord = RetroWord(ident, identifiantA, identifiantS);
            return true;
        }
        else throw InvalidFileException("NindRetrolexicon::getRetroWord : " + m_fileName);
    }
    catch (FileException &exc) {
        throw NindRetrolexiconException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//ejtablit la carte des dejfinitions  
void NindRetrolexicon::mapDejfinition()
{
    m_dejfinitionMapping.clear();
    m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
    while (true) {
        //<flagDejfinition=43> <addrBlocSuivant> <nombreDejfinitions>
        m_file.readBuffer(TETE_DEJFINITION);
        if (m_file.getInt1() != FLAG_DEJFINITION) 
            throw InvalidFileException("NindRetrolexicon::mapDejfinition : " + m_fileName);
        const unsigned long int addrBlocSuivant = m_file.getInt5();
        const unsigned int nombreDejfinitions = m_file.getInt3();
        const unsigned long pos = m_file.getPos(); 
        const pair<unsigned int, unsigned long int> dejfinition(pos, nombreDejfinitions);
        m_dejfinitionMapping.push_back(dejfinition);
        if (addrBlocSuivant == 0) break;        //si pas d'extension, termine
        //saute au bloc d'indirection suivant
        m_file.setPos(addrBlocSuivant, SEEK_SET);    //pour aller au suivant
    }
}
////////////////////////////////////////////////////////////
//return l'offset de l'indirection de la dejfinition specifiej, 0 si hors limite
unsigned long int NindRetrolexicon::getDejfinitionPos(const unsigned int ident)
{
    //trouve la dejfinition
    unsigned int firstIdent = 0;
    list<pair<unsigned long int, unsigned int> >::const_iterator it = m_dejfinitionMapping.begin(); 
    while (it != m_dejfinitionMapping.end()) {
        if (ident < firstIdent + (*it).second) return (ident - firstIdent) * TAILLE_DEJFINITION + (*it).first;
        firstIdent += (*it).second;
        it++;
    }
    //si la dejfinition chercheje n'existe pas, retourne 0
    return 0;
}
////////////////////////////////////////////////////////////
//brief Read from file datas of a specified definition and leave result into read buffer 
//param ident ident of definition
//return true if ident was found, false otherwise */
bool NindRetrolexicon::getDejfinition(const unsigned int ident)
{
    unsigned long int dejfinitionPos = getDejfinitionPos(ident);
    if (dejfinitionPos == 0) {
        //le processus ejcrivain se dejmerde par ailleurs
        if (m_isWriter) return false;   
        //le processus lecteur met ainsi ah jour sa table d'indirection
        //ejtablit la carte des indirections  
        mapDejfinition();
        dejfinitionPos = getDejfinitionPos(ident);
    }
    if (dejfinitionPos == 0) return false;
    m_file.flush();
    //se positionne sur la definition
    m_file.setPos(dejfinitionPos, SEEK_SET);    
    m_file.readBuffer(TAILLE_DEJFINITION);
    return true;
}
////////////////////////////////////////////////////////////
//ajoute un bloc de dejfinitions vides suivi d'une identification ah la position courante du fichier
void NindRetrolexicon::addBlocDejfinition(const NindIndex::Identification &lexiconIdentification)
{
    //se positionne sur l'identification
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       
    const unsigned long int blocDejfinition = m_file.getPos();
    m_file.createBuffer(TETE_DEJFINITION);
    //<flagDejfinition=43> <addrBlocSuivant> <nombreDejfinitions>
    m_file.putInt1(FLAG_DEJFINITION);
    m_file.putInt5(0);
    m_file.putInt3(m_dejfinitionBlocSize);
    m_file.writeBuffer();                               //ecriture effective sur le fichier   
    //remplit la zone d'indirection avec des 0
    m_file.writeValue(0, m_dejfinitionBlocSize*TAILLE_DEJFINITION);
    //lui colle l'identification du lexique a suivre
    addIdentification(lexiconIdentification);
    //si ce n'est pas le premier bloc, il faut chaisner dans le fichier
    if (m_dejfinitionMapping.size() != 0) {
        pair<unsigned long int, unsigned int> &indirectionBlocPrec = m_dejfinitionMapping.back();
        //se positionne sur le <addrBlocSuivant> du dernier bloc
        //<flagDejfinition=43> <addrBlocSuivant> <nombreDejfinitions> { <dejfinitionMot> }
        m_file.setPos(indirectionBlocPrec.first -8, SEEK_SET);   //pour pointer <addrBlocSuivant>
        m_file.createBuffer(5);
        m_file.putInt5(blocDejfinition);
        m_file.writeBuffer();
    }
    //met ah jour la carte des dejfinitions
    const pair<unsigned long int, unsigned int> dejfinitions(blocDejfinition + TETE_DEJFINITION, m_dejfinitionBlocSize);
    m_dejfinitionMapping.push_back(dejfinitions);
}
////////////////////////////////////////////////////////////
//ejcrit l'identification du fichier ah l'adresse courente du fichier
void NindRetrolexicon::addIdentification(const NindIndex::Identification &lexiconIdentification)
{
    //le pointeur est censej estre au bon endroit
    m_file.createBuffer(TAILLE_IDENTIFICATION);
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    m_file.putInt1(FLAG_IDENTIFICATION);
    m_file.putInt3(lexiconIdentification.lexiconWordsNb);
    m_file.putInt4(lexiconIdentification.lexiconTime);
    m_file.putInt4(lexiconIdentification.specificFileIdent);
    m_file.writeBuffer();                               //ecriture effective sur le fichier
}
////////////////////////////////////////////////////////////
//verifie l'apairage avec le lexique
void NindRetrolexicon::checkIdentification(const NindIndex::Identification &lexiconIdentification)
{
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw InvalidFileException("NindIndex::checkIdentification : " + m_fileName);
    const unsigned int maxIdent = m_file.getInt3();
    const unsigned int identification = m_file.getInt4();
    //si c'est le fichier lexique qui est verifiej, pas de comparaison de valeurs
    if (lexiconIdentification == NindIndex::Identification(0, 0, 0)) return;
    //si ce n'est pas le fichier lexique qui est verifiej, comparaison de valeurs non spejcifiques
    if (NindIndex::Identification(maxIdent, identification, 0) != lexiconIdentification) {
        throw IncompatibleFileException(m_fileName); 
    }
}
////////////////////////////////////////////////////////////
