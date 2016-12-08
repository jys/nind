//
// C++ Implantation: NindRetrolexiconIndex
//
// Description: La gestion du lexique et du lexique inverse sous forme de fichiers index
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
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
#include "NindRetrolexiconIndex.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition=17> <identifiantMot> <longueurDonnees> <donneesMot>
// <flagDefinition=17>     ::= <Integer1>
// <identifiantMot>        ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesMot>            ::= <motCompose> | <motSimple>
// <motCompose>            ::= <flagCompose=31> <identifiantA> <identifiantRelS>
// <flagCompose=31>        ::= <Integer1>
// <identifiantA>          ::= <Integer3>
// <identifiantRelS>       ::= <IntegerSLat>
// <motSimple>             ::= <flagSimple=37> <longueurMot> <motUtf8>
// <flagSimple=37>         ::= <Integer1>
// <longueurMot>           ::= <Integer1>
// <motUtf8>               ::= { <Octet> }
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
#define FLAG_COMPOSE 31
#define FLAG_SIMPLE 37
//<flagDefinition=17>(1) <identifiantMot>(3) = 4
#define OFFSET_LONGUEUR 4
//<flagDefinition=17>(1) <identifiantMot>(3) <longueurDonnees>(3) = 7
#define TETE_DEFINITION 7
//<flagDefinition=17>(1) <identifiantMot>(3) <longueurDonnees>(3) <flagCompose=31>(1) <identifiantA>(3) 
//<identifiantRelS>(1) = 12
//<flagDefinition=17>(1) <identifiantMot>(3) <longueurDonnees>(3) <flagSimple=37>(1) <longueurMot>(1) 
//<motUtf8>(1) = 10
#define TAILLE_DEFINITION_MINIMUM 10
//<flagDefinition=17>(1) <identifiantMot>(3) <longueurDonnees>(3) <flagSimple=37>(1) <longueurMot>(1) 
//<motUtf8>(255) = 264
#define TAILLE_DEFINITION_MAXIMUM 264
//<flagCg>(1) <categorie>(1) <frequenceMot>(3) <nbreDocs>(3) = 8
//#define TETE_DEFINITION_MAXIMUM 8
//<identDocRelatif>(3) <frequenceDoc>(2) = 5
//#define TAILLE_DOC_MAXIMUM 5
//taille maximum d'un mot compose (pour detecter les bouclages)
#define TAILLE_COMPOSE_MAXIMUM 30
////////////////////////////////////////////////////////////
//brief Creates NindRetrolexiconIndex.
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param indirectionBlocSize number of entries in a single indirection block */
NindRetrolexiconIndex::NindRetrolexiconIndex(const string &fileName,
                                             const bool isLexiconWriter,
                                             const Identification &lexiconIdentification,
                                             const unsigned int indirectionBlocSize):
    NindIndex(fileName, 
              isLexiconWriter, 
              lexiconIdentification, 
              TAILLE_DEFINITION_MINIMUM, 
              indirectionBlocSize)
{
}
////////////////////////////////////////////////////////////
NindRetrolexiconIndex::~NindRetrolexiconIndex()
{
}
////////////////////////////////////////////////////////////
//brief add a list of word idents in retro lexicon. If one of idents still exists, exception is raised
//param retroWords list of words definitions 
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
void NindRetrolexiconIndex::addRetroWords(const list<struct RetroWord> &retroWords,
                                     const Identification &lexiconIdentification)
{
    try {
        if (!m_isWriter) throw BadUseException("retro lexicon is not writable");
        //il y a autant d'ajout au fichier qu'il y a de nouveaux identifiants
        for (list<struct RetroWord>::const_iterator it1 = retroWords.begin(); it1 != retroWords.end(); it1++) {
            const struct RetroWord &retroWord = (*it1);
            //1) verifie que le mot n'est pas en dehors du dernier bloc d'indirection
            //il faut le faire maintenant parce que le buffer d'ecriture est unique
            checkExtendIndirection(retroWord.identifiant, lexiconIdentification);
            //2) forme le buffer a ecrire sur le fichier
            m_file.createBuffer(TAILLE_DEFINITION_MAXIMUM); 
            //<flagDefinition=17> <identifiantMot> <longueurDonnees> <donneesMot>
            m_file.putInt1(FLAG_DEFINITION);
            m_file.putInt3(retroWord.identifiant);
            m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
            //simple si pas compose (pas avec la chaine vide)
            if (retroWord.identifiantA == 0) {
                //<flagSimple=37> <longueurMot> <motUtf8>
                m_file.putInt1(FLAG_SIMPLE);
                m_file.putString(retroWord.motSimple);
            }
            else {
                //<flagCompose=31> <identifiantA> <identifiantRelS>
                m_file.putInt1(FLAG_COMPOSE);
                m_file.putInt3(retroWord.identifiantA);
                m_file.putSIntLat(retroWord.identifiantS - retroWord.identifiant);
            }
            //ecrit la taille reelle du buffer
            const unsigned int longueurDonnees = m_file.getOutBufferSize() - TETE_DEFINITION;
            m_file.putInt3(longueurDonnees, OFFSET_LONGUEUR);  //la taille dans la trame
            //4) ecrit la definition du mot et gere le fichier
            setDefinition(retroWord.identifiant, lexiconIdentification);           
        }
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindRetrolexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get word components from the specified ident
//param ident ident of word
//param components list of components of a word 
//(1 component = simple word, more components = compound word) */
//return true if word was found, false otherwise */
bool NindRetrolexiconIndex::getComponents(const unsigned int ident,
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
        if (!existe) throw InvalidFileException("NindRetrolexiconIndex::getComponents A : " + m_fileName);
        if (retroWordS.identifiantA != 0) InvalidFileException("NindRetrolexiconIndex::getComponents B : " + m_fileName);
        components.push_front(retroWordS.motSimple);
        //recupere l'autre mot du couple
        existe = getRetroWord(retroWord.identifiantA, retroWord);
        if (!existe) throw InvalidFileException("NindRetrolexiconIndex::getComponents C : " + m_fileName);  
        //pour detecter les bouclages induits par un fichier bouclant
        if (components.size() == TAILLE_COMPOSE_MAXIMUM) throw InvalidFileException("NindRetrolexiconIndex::getComponents D : " + m_fileName);  
    }    
}    
////////////////////////////////////////////////////////////
//Recupere sur le fichier retro lexique la definition d'un mot specifie par son identifiant
//ident identifiant du mot
//retroWord structure ou est ecrite la definition
//retourne true si le mot existe, sinon false
bool NindRetrolexiconIndex::getRetroWord(const unsigned int ident,
                                       struct RetroWord &retroWord)   
{
    try {
        const bool existe = getDefinition(ident);
        if (!existe) return false;
        //<flagDefinition=17> <identifiantMot> <longueurDonnees> <donnees>
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindRetrolexiconIndex::getRetroWord A : " + m_fileName);
        const unsigned int identifiantMot = m_file.getInt3();
        if (identifiantMot != ident) throw InvalidFileException("NindRetrolexiconIndex::getRetroWord B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donneesMot
        m_file.setEndInBuffer(longueurDonnees);
        const unsigned char flag = m_file.getInt1();
        //<flagCompose=31> <identifiantA> <identifiantRelS>
        if (flag == FLAG_COMPOSE) {
            const unsigned int identifiantA = m_file.getInt3();
            const unsigned int identifiantS = identifiantMot + m_file.getSIntLat();
            retroWord = RetroWord(ident, identifiantA, identifiantS);
        }
        //<flagSimple=37> <longueurMot> <motUtf8>
        else if (flag == FLAG_SIMPLE) {
            const string motUtf8 = m_file.getString();
            retroWord = RetroWord(ident, motUtf8);
        }
        else throw InvalidFileException("NindRetrolexiconIndex::getRetroWord C : " + m_fileName);    
        return true;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindRetrolexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
