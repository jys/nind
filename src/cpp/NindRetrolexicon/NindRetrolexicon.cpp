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
// <fichier>               ::= { <blocIndexej> <blocEnVrac> } <blocIdentification> 
//
// <blocIndexej>           ::= <flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
// <flagIndexej=47>        ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreIndex>           ::= <Integer3>
//
// <donnejesIndexejes>     ::= { <Octet> }
// <blocEnVrac>            ::= { <Octet> }
//
// <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
// <flagIdentification=53> ::= <Integer1>
// <maxIdentifiant>        ::= <Integer3>
// <identifieurUnique>     ::= <dateHeure>
// <identifieurSpecifique> ::= <Integer4>
// <dateHeure >            ::= <Integer4>
////////////////////////////////////////////////////////////
// <donnejesIndexejes>     ::= <dejfinitionMot>
// <blocEnVrac>            ::= <blocUtf8>
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

// <blocEnVrac>            ::= { <Octet> }
////////////////////////////////////////////////////////////
#define FLAG_COMPOSEJ 31
#define FLAG_SIMPLE 37
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
                                   const Identification &lexiconIdentification,
                                   const unsigned int definitionBlocSize):
    NindPadFile(fileName, isLexiconWriter, lexiconIdentification, TAILLE_DEJFINITION, definitionBlocSize)
{
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
                                     const Identification &lexiconIdentification)
{
    try {
        if (!m_isWriter) throw BadUseException("retro lexicon is not writable");
        //il y a autant d'ajout au fichier qu'il y a de nouveaux identifiants
        for (list<struct RetroWord>::const_iterator it1 = retroWords.begin(); it1 != retroWords.end(); it1++) {
            const struct RetroWord &retroWord = (*it1);
            //1) rejcupehre l'adresse de la dejfinition
            unsigned long int dejfinition = getEntryPos(retroWord.identifiant);
            //2) si hors limite, ajoute un bloc de dejfinitions
            if (dejfinition == 0) addEntriesBlock(lexiconIdentification);
            //et rejcupehre l'adresse de la dejfinition
            dejfinition = getEntryPos(retroWord.identifiant);
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
//brief Read from file datas of a specified definition and leave result into read buffer 
//param ident ident of definition
//return true if ident was found, false otherwise */
bool NindRetrolexicon::getDejfinition(const unsigned int ident)
{
    unsigned long int dejfinitionPos = getEntryPos(ident);
    if (dejfinitionPos == 0) return false;
    m_file.flush();
    //se positionne sur la definition
    m_file.setPos(dejfinitionPos, SEEK_SET);    
    m_file.readBuffer(TAILLE_DEJFINITION);
    return true;
}
////////////////////////////////////////////////////////////
