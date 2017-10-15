//
// C++ Implementation: NindLexiconIndex
//
// Description: La gestion du lexique sous forme de fichier index
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
//
// Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindLexiconIndex.h"
#include "NindRetrolexicon/NindRetrolexicon.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <dejfinition>           ::= <flagDejfinition=13> <identifiantHash> <longueurDonnejes> <donnejesHash>
// <flagDejfinition=13>    ::= <Integer1>
// <identifiantHash>       ::= <Integer3>
// <longueurDonnejes>      ::= <Integer3>
// <donnejesHash>          ::= { <mot> }
// <mot>                   ::= <motSimple> <identifiantS> <nbreComposejs> <composejs>
// <motSimple>             ::= <longueurMot> <motUtf8>
// <longueurMot>           ::= <Integer1>
// <motUtf8>               ::= { <Octet> }
// <identifiantS>          ::= <Integer3>
// <nbreComposejs>         ::= <IntegerULat>
// <composejs>             ::= { <composej> } 
// <composej>              ::= <identifiantA> <identifiantRelC>
// <identifiantA>          ::= <Integer3>
// <identifiantRelC>       ::= <IntegerSLat>
////////////////////////////////////////////////////////////
// <spejcifique>           ::= <vide>
////////////////////////////////////////////////////////////
#define FLAG_DEJFINITION 13 //<flagDejfinition=13>(1) <identifiantHash>(3) = 4
#define OFFSET_LONGUEUR 4
//<flagDejfinition=13> <identifiantHash> <longueurDonnejes> = 7
#define TAILLE_TESTE_DEJFINITION 7
//<flagDejfinition=13>(1) <identifiantHash>(3) <longueurDonnejes>(3) 
//<clefA>(4) <clefB>(4) <identifiantS>(3) <nbreComposejs>(1) = 19
#define TAILLE_DEJFINITION_MINIMUM 19
//taille des spejcifiques
#define TAILLE_SPEJCIFIQUES 0
////////////////////////////////////////////////////////////
static unsigned int clef(const string &mot);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconIndex.
//param fileNameExtensionLess absolute path file name without extension
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param withRetrolexicon true if retro lexicon 
//param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
//param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
NindLexiconIndex::NindLexiconIndex(const string &fileNameExtensionLess,
                                   const bool isLexiconWriter,
                                   const bool withRetrolexicon,
                                   const unsigned int indirectionBlocSize,
                                   const unsigned int retroIndirectionBlocSize):
    NindIndex(fileNameExtensionLess + ".nindlexiconindex", 
              isLexiconWriter, 
              Identification(0, 0), 
              TAILLE_SPEJCIFIQUES,
              TAILLE_DEJFINITION_MINIMUM, 
              indirectionBlocSize),
    m_modulo(0),
    m_identification(Identification(0, 0)),
    m_withRetrolexicon(withRetrolexicon)
{
    //la taille du bloc d'indirection du fichier reel est structurante
    //(indirectionBlocSize ne sert que pour la crejation du fichier lexique)
    m_modulo = getFirstEntriesBlockSize(); 
    //l'identifiant de mot le plus eleve (pour l'ecrivain)
    getFileIdentification(m_identification);
    //initialisation du retro lexique, eventuellement
    if (m_withRetrolexicon) 
        m_nindRetrolexicon = new NindRetrolexicon(fileNameExtensionLess,
                                                  isLexiconWriter,
                                                  m_identification,
                                                  retroIndirectionBlocSize);
    else m_nindRetrolexicon = 0;
}
////////////////////////////////////////////////////////////
NindLexiconIndex::~NindLexiconIndex()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon it doesn't still exist in,
//In all cases, word ident is returned.
//param components list of components of a word 
//(1 component = simple word, more components = compound word)
//return ident of word */
unsigned int NindLexiconIndex::addWord(const list<string> &components)
{
    if (!m_isWriter) throw NindLexiconIndexException("NindLexiconIndex::addWord lexicon is not writable" + m_fileName);
    //identifiant du mot (simple ou composej) sous ensemble du mot examine
    unsigned int sousMotId = 0;
    //le compteur courant des identifiants du lexique
    unsigned int &currentId = m_identification.lexiconWordsNb;
    for (list<string>::const_iterator swIt = components.begin(); swIt != components.end(); swIt++) {
        bool estNouveau = false;
        const string &motSimple = *swIt;
        list<Mot> dejfinition;
        list<Mot>::iterator motIt;
        //cherche l'identifiant du mot sur le fichier et prepare la structure de reecriture si pas trouve
        const unsigned int motId = getDefinitionWords(motSimple, sousMotId, dejfinition, motIt);
        if (motId != 0) {
            //trouve, on passe au suivant
            sousMotId = motId;
            continue;
        }
        //pas trouve
        //structure pour eventuellement mettre a jour le retro lexique
        list<struct NindRetrolexicon::RetroWord> retroWords;
        //si mot simple inconnu, on le cree
        if ((*motIt).identifiantS == 0) {
            (*motIt).identifiantS = ++currentId;
            //pour le lexique inverse : currentId -> motSimple
            retroWords.push_back(NindRetrolexicon::RetroWord(currentId, motSimple));
        }
        //si mot composej, on le cree
        if (sousMotId != 0) {
            (*motIt).composejs.push_back(Composej(sousMotId, ++currentId));
            //pour le lexique inverse : currentId -> sousMotId, (*motIt).identifiantS
            retroWords.push_back(NindRetrolexicon::RetroWord(currentId, sousMotId, (*motIt).identifiantS));
       }
        sousMotId = currentId;
        //et on ecrit sur le fichier
        m_identification.lexiconTime = (time_t)time(NULL);
        setDefinitionWords(dejfinition, m_identification);
        //met eventuellement a jour le lexique inverse
        if (m_withRetrolexicon and retroWords.size() != 0) 
            m_nindRetrolexicon->addRetroWords(retroWords, m_identification);
    }
    //retourne l'id du mot specifie
    return sousMotId;
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param components list of components of a word (1 component = simple word, more components = compound word)
//return ident of word */
unsigned int NindLexiconIndex::getWordId(const list<string> &components)
{
    //identifiant du mot (simple ou composej) sous ensemble du mot examine
    unsigned int sousMotId = 0;
    for(list<string>::const_iterator swIt = components.begin(); swIt != components.end(); swIt++) {
        const string &motSimple = *swIt;
        const unsigned int motId = getIdentifiant(motSimple, sousMotId);
        if (motId == 0) return 0;     //mot inconnu
        sousMotId = motId;
    }
    //retourne l'id du mot specifie
    return sousMotId;
}
////////////////////////////////////////////////////////////
//brief get identification of lexicon
//return unique identification of lexicon */
NindIndex::Identification NindLexiconIndex::getIdentification()
{
    return m_identification;
}
////////////////////////////////////////////////////////////
//Recupere l'identifiant d'un mot sur le fichier lexique
//retourne l'identifiant du mot s'il existe, 0 s'il n'existe pas
unsigned int NindLexiconIndex::getIdentifiant(const string &motSimple,
                                              const unsigned int sousMotId)
{
    //l'identifiant dans le fichier est la clef calculee modulo la taille du bloc d'indirection
    const unsigned int ident =  clef(motSimple) % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (!existe) return 0;              //pas trouve de dejfinition => mot inconnu
    //<flagDejfinition=13> <identifiantHash> <longueurDonnejes> <donnejesHash>
    if (m_file.getInt1() != FLAG_DEJFINITION) 
        throw NindLexiconIndexException("NindLexiconIndex::getIdentifiant A : " + m_fileName);
    if (m_file.getInt3() != ident) 
        throw NindLexiconIndexException("NindLexiconIndex::getIdentifiant B : " + m_fileName);
    const unsigned int longueurDonnejes = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnees
    m_file.setEndInBuffer(longueurDonnejes);
    while (!m_file.endOfInBuffer()) {
        //<motSimple> <identifiantS> <nbreComposejs> <composejs>
        const string fmotSimple = m_file.getString();
        if (fmotSimple == motSimple) {
            //c'est le bon mot
            const unsigned int identifiantS = m_file.getInt3();
            if (sousMotId == 0) return identifiantS;        //retourne l'id du mot simple
            const unsigned int nbreComposejs = m_file.getUIntLat();
            unsigned int identifiantC = identifiantS;
            for (unsigned int i = 0; i != nbreComposejs; i++) {
                //<identifiantA> <identifiantRelC>
                const unsigned int identifiantA = m_file.getInt3();
                identifiantC += m_file.getSIntLat();
                if (sousMotId == identifiantA) return identifiantC;     //retourne l'id du mot composej
            }
            return 0;           //mot composej inconnu           
        }
        //ce n'est pas le bon mot, on le saute
        m_file.getInt3();
        const unsigned int nbreComposejs = m_file.getUIntLat();
        for (unsigned int i = 0; i != nbreComposejs; i++) {
            //<identifiantA> <identifiantRelC>
            m_file.getInt3();
            m_file.getSIntLat();
        }        
    }
    return 0;           //pas trouve le mot simple => mot inconnu  
}
////////////////////////////////////////////////////////////
//recupere les donnees de tous les mots qui ont la meme clef modulo 
//retourne l'identifiant du mot s'il existe, sinon retourne 0
//si le mot n'existe pas, la structure retournée est valide, sinon elle ne l'est pas
unsigned int NindLexiconIndex::getDefinitionWords(const string &motSimple,
                                                   const unsigned int sousMotId,
                                                   list<Mot> &dejfinition,
                                                   list<Mot>::iterator &motIt)
{
    bool motEstTrouve = false;
    //l'identifiant dans le fichier est la clef calculee modulo la taille du bloc d'indirection
    const unsigned int ident =  clef(motSimple) % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (existe) {
        //<flagDejfinition=13> <identifiantHash> <longueurDonnejes> <donnejesHash>
        if (m_file.getInt1() != FLAG_DEJFINITION) 
            throw NindLexiconIndexException("NindLexiconIndex::getDefinitionWords A : " + m_fileName);
        if (m_file.getInt3() != ident) 
            throw NindLexiconIndexException("NindLexiconIndex::getDefinitionWords B : " + m_fileName);
        const unsigned int longueurDonnejes = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donnees
        m_file.setEndInBuffer(longueurDonnejes);
        while (!m_file.endOfInBuffer()) {
            //cree une nouvelle structure pour un mot
            dejfinition.push_front(Mot());
            Mot &motCourant = dejfinition.front();
            //<motSimple> <identifiantS> <nbreComposejs> <composejs>
            motCourant.motSimple = m_file.getString();
            motCourant.identifiantS = m_file.getInt3();
            const unsigned int nbreComposejs = m_file.getUIntLat();
            unsigned int identifiantC = motCourant.identifiantS;
            //est-ce le mot cherche ?
            if (motCourant.motSimple == motSimple) {
                if (sousMotId == 0) return motCourant.identifiantS; //retourne id mot simple
                motEstTrouve = true;
                motIt = dejfinition.begin();
                for (unsigned int i = 0; i != nbreComposejs; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    //si mot composej trouve, on s'arrete immediatement
                    if (sousMotId == identifiantA) return identifiantC;         //retourne id mot composej
                    motCourant.composejs.push_back(Composej(identifiantA, identifiantC));
                }
            }
            else {
                for (unsigned int i = 0; i != nbreComposejs; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    motCourant.composejs.push_back(Composej(identifiantA, identifiantC));
                }
            }
        }
    }
    if (!motEstTrouve) {
        //le mot n'est pas sur le fichier, il faut le creer
        //cree une nouvelle structure pour un mot
        dejfinition.push_front(Mot());
        motIt = dejfinition.begin();
        (*motIt).motSimple = motSimple;
    }
    return 0;           //mot inconnu et structure coherente pour la reecriture
}
////////////////////////////////////////////////////////////
//Ecrit les donnees de tous les mots qui ont la meme clef modulo 
void NindLexiconIndex::setDefinitionWords(const list<Mot> &dejfinition,
                                          const Identification &lexiconIdentification)
{
    //1) calcule la taille max du buffer
    unsigned int tailleMaximum = TAILLE_TESTE_DEJFINITION + getSpecificsAndIdentificationSize();
    for (list<Mot>::const_iterator defIt = dejfinition.begin(); defIt != dejfinition.end(); defIt++) {
        //<motSimple>(256) <identifiantS>(3) <nbreComposejs>(3) <composejs> = 262
        //<identifiantA>(3) <identifiantRelC>(4) = 7
        tailleMaximum += 262 + (*defIt).composejs.size() * 7;
    }
    //2) forme le buffer a ecrire sur le fichier
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const Mot &premierMot = dejfinition.front();
    const unsigned int ident =  clef(premierMot.motSimple) % m_modulo;
    m_file.createBuffer(tailleMaximum); 
    //<flagDejfinition=13> <identifiantHash> <longueurDonnejes> 
    m_file.putInt1(FLAG_DEJFINITION);
    m_file.putInt3(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    for (list<Mot>::const_iterator defIt = dejfinition.begin(); defIt != dejfinition.end(); defIt++) {  
        //<motSimple> <identifiantS> <nbreComposejs>
        m_file.putString((*defIt).motSimple);
        m_file.putInt3((*defIt).identifiantS);
        m_file.putUIntLat((*defIt).composejs.size());
        unsigned int identifiantC = (*defIt).identifiantS;
        for (list<Composej>::const_iterator compIt = (*defIt).composejs.begin(); compIt != (*defIt).composejs.end(); compIt++) {
            //<identifiantA> <identifiantRelC>
            m_file.putInt3((*compIt).identA);
            m_file.putSIntLat((*compIt).identComp - identifiantC);
            identifiantC = (*compIt).identComp;
        }
    }
    //ecrit la taille reelle du buffer
    const unsigned int longueurDonnejes = m_file.getOutBufferSize() - TAILLE_TESTE_DEJFINITION;
    m_file.putInt3(longueurDonnejes, OFFSET_LONGUEUR);  //la taille dans la trame
    //calcule le nombre d'extra octets ah mettre dans le buffer pour atteindre la taille minimum
    int extra = m_file.getOutBufferSize() - TAILLE_DEJFINITION_MINIMUM;
    while (extra++ < 0) m_file.putInt1(0);
    //3) ejcrit l'en-teste des spejcifiques et l'identification
    writeSpecificsHeader();
    writeIdentification(lexiconIdentification);
    //4) ecrit la dejfinition du mot et gere le fichier
    setDefinition(ident);   
}
////////////////////////////////////////////////////////////
//calcule la clef de hachage
static unsigned int clef(const string &mot)
{
    unsigned int shiftB = 0;       //nombre de shifts sur la gauche
    unsigned int clefB = 0x55555555;        //0101...
    for (int i=0; i<mot.length(); i++) {
        const unsigned int oneChar = (unsigned int) ((unsigned char)mot[i]);
        clefB ^=  (oneChar << (shiftB%23));
        shiftB += 5;
    }
    return clefB;
}
////////////////////////////////////////////////////////////
//brief get word components from the specified ident
//if retro lexicon is not implanted, an exception is raised
//param ident ident of word
//param components list of components of a word 
//(1 component = simple word, more components = compound word) 
//return true if word was found, false otherwise */
bool NindLexiconIndex::getComponents(const unsigned int ident,
                                     list<string> &components)
{
    if (!m_withRetrolexicon) 
        throw NindLexiconIndexException("NindLexiconIndex::getComponents No retro lexicon " + m_fileName);
    return m_nindRetrolexicon->getComponents(ident, components);
}
////////////////////////////////////////////////////////////
//brief get retrolexicon file name
//return file name of retrolexicon */
string NindLexiconIndex::getRetrolexiconFileName()
{
    if (m_nindRetrolexicon != 0) return m_nindRetrolexicon->getFileName();
    else return "";
}
////////////////////////////////////////////////////////////
  


