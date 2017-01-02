//
// C++ Implementation: NindLexiconIndex
//
// Description: La gestion du lexique sous forme de fichier index
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
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
#include "NindLexiconIndex.h"
#include "NindRetrolexiconIndex.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition=17> <identifiantHash> <longueurDonnees> <donneesHash>
// <flagDefinition=17>     ::= <Integer1>
// <identifiantHash>       ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesHash>           ::= { <mot> }
// <mot>                   ::= <motSimple> <identifiantS> <nbreComposes> <composes>
// <motSimple>             ::= <longueurMot> <motUtf8>
// <longueurMot>           ::= <Integer1>
// <motUtf8>               ::= { <Octet> }
// <identifiantS>          ::= <Integer3>
// <nbreComposes>          ::= <IntegerULat>
// <composes>              ::= { <compose> } 
// <compose>               ::= <identifiantA> <identifiantRelC>
// <identifiantA>          ::= <Integer3>
// <identifiantRelC>       ::= <IntegerSLat>
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
//<flagDefinition=17>(1) <identifiantHash>(3) = 4
#define OFFSET_LONGUEUR 4
//<flagDefinition=17> <identifiantHash> <longueurDonnees> = 7
#define TETE_DEFINITION 7
//<flagDefinition=17>(1) <identifiantHash>(3) <longueurDonnees>(3) 
//<clefA>(4) <clefB>(4) <identifiantS>(3) <nbreComposes>(1) = 19
#define TAILLE_DEFINITION_MINIMUM 19
//<localisationRelatif>(2) <longueur>(1) = 3
//#define TAILLE_LOC_MAXIMUM 3
////////////////////////////////////////////////////////////
static unsigned int clef(const string &mot);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconIndex.
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param withRetrolexicon true if retro lexicon 
//param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
//param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
NindLexiconIndex::NindLexiconIndex(const string &fileName,
                                   const bool isLexiconWriter,
                                   const bool withRetrolexicon,
                                   const unsigned int indirectionBlocSize,
                                   const unsigned int retroIndirectionBlocSize):
    NindIndex(fileName, 
        isLexiconWriter, 
        Identification(0, 0, 0),
        TAILLE_DEFINITION_MINIMUM, 
        indirectionBlocSize),
    m_modulo(0),
    m_identification(Identification(0, 0, 0)),
    m_withRetrolexicon(withRetrolexicon)
{
    //la taille du bloc d'indirection du fichier reel est structurante
    m_modulo = getFirstIndirectionBlockSize(); 
    //l'identifiant de mot le plus eleve (pour l'ecrivain)
    getFileIdentification(m_identification);
    //initialisation du retro lexique, eventuellement
    if (m_withRetrolexicon) {
        const size_t pos = fileName.find('.');
        m_nindRetrolexiconIndex = new NindRetrolexiconIndex(fileName.substr(0, pos) + ".retrolexiconindex",
                                                            isLexiconWriter,
                                                            m_identification,
                                                            retroIndirectionBlocSize);
    }
    else m_nindRetrolexiconIndex = 0;
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
    if (!m_isWriter) throw BadUseException("lexicon is not writable");
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int sousMotId = 0;
    //le compteur courant des identifiants du lexique
    unsigned int &currentId = m_identification.lexiconWordsNb;
    for (list<string>::const_iterator swIt = components.begin(); swIt != components.end(); swIt++) {
        bool estNouveau = false;
        const string &motSimple = *swIt;
        list<Mot> definition;
        list<Mot>::iterator motIt;
        //cherche l'identifiant du mot sur le fichier et prepare la structure de reecriture si pas trouve
        const unsigned int motId = getDefinitionWords(motSimple, sousMotId, definition, motIt);
        if (motId != 0) {
            //trouve, on passe au suivant
            sousMotId = motId;
            continue;
        }
        //pas trouve
        //structure pour eventuellement mettre a jour le retro lexique
        list<struct NindRetrolexiconIndex::RetroWord> retroWords;
        //si mot simple inconnu, on le cree
        if ((*motIt).identifiantS == 0) {
            (*motIt).identifiantS = ++currentId;
            //pour le lexique inverse : currentId -> motSimple
            retroWords.push_back(NindRetrolexiconIndex::RetroWord(currentId, motSimple));
        }
        //si mot compose, on le cree
        if (sousMotId != 0) {
            (*motIt).composes.push_back(Compose(sousMotId, ++currentId));
            //pour le lexique inverse : currentId -> sousMotId, (*motIt).identifiantS
            retroWords.push_back(NindRetrolexiconIndex::RetroWord(currentId, sousMotId, (*motIt).identifiantS));
       }
        sousMotId = currentId;
        //et on ecrit sur le fichier
        m_identification.lexiconTime = (time_t)time(NULL);
        setDefinitionWords(definition, m_identification);
        //met eventuellement a jour le lexique inverse
        if (m_withRetrolexicon and retroWords.size() != 0) 
            m_nindRetrolexiconIndex->addRetroWords(retroWords, m_identification);
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
    try {
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
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
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconIndexException(m_fileName);
    }
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
    if (!existe) return 0;              //pas trouve de definition => mot inconnu
    //<flagDefinition=17> <identifiantHash> <longueurDonnees> <donneesHash>
    if (m_file.getInt1() != FLAG_DEFINITION) 
        throw InvalidFileException("NindLexiconIndex::getIdentifiant A : " + m_fileName);
    if (m_file.getInt3() != ident) 
        throw InvalidFileException("NindLexiconIndex::getIdentifiant B : " + m_fileName);
    const unsigned int longueurDonnees = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnees
    m_file.setEndInBuffer(longueurDonnees);
    while (!m_file.endOfInBuffer()) {
        //<motSimple> <identifiantS> <nbreComposes> <composes>
        const string fmotSimple = m_file.getString();
        if (fmotSimple == motSimple) {
            //c'est le bon mot
            const unsigned int identifiantS = m_file.getInt3();
            if (sousMotId == 0) return identifiantS;        //retourne l'id du mot simple
            const unsigned int nbreComposes = m_file.getUIntLat();
            unsigned int identifiantC = identifiantS;
            for (unsigned int i = 0; i != nbreComposes; i++) {
                //<identifiantA> <identifiantRelC>
                const unsigned int identifiantA = m_file.getInt3();
                identifiantC += m_file.getSIntLat();
                if (sousMotId == identifiantA) return identifiantC;     //retourne l'id du mot compose
            }
            return 0;           //mot compose inconnu           
        }
        //ce n'est pas le bon mot, on le saute
        m_file.getInt3();
        const unsigned int nbreComposes = m_file.getUIntLat();
        for (unsigned int i = 0; i != nbreComposes; i++) {
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
                                                   list<Mot> &definition,
                                                   list<Mot>::iterator &motIt)
{
    bool motEstTrouve = false;
    //l'identifiant dans le fichier est la clef calculee modulo la taille du bloc d'indirection
    const unsigned int ident =  clef(motSimple) % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (existe) {
        //<flagDefinition=17> <identifiantHash> <longueurDonnees> <donneesHash>
        if (m_file.getInt1() != FLAG_DEFINITION) 
            throw InvalidFileException("NindLexiconIndex::getDefinitionWords A : " + m_fileName);
        if (m_file.getInt3() != ident) 
            throw InvalidFileException("NindLexiconIndex::getDefinitionWords B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donnees
        m_file.setEndInBuffer(longueurDonnees);
        while (!m_file.endOfInBuffer()) {
            //cree une nouvelle structure pour un mot
            definition.push_front(Mot());
            Mot &motCourant = definition.front();
            //<motSimple> <identifiantS> <nbreComposes> <composes>
            motCourant.motSimple = m_file.getString();
            motCourant.identifiantS = m_file.getInt3();
            const unsigned int nbreComposes = m_file.getUIntLat();
            unsigned int identifiantC = motCourant.identifiantS;
            //est-ce le mot cherche ?
            if (motCourant.motSimple == motSimple) {
                if (sousMotId == 0) return motCourant.identifiantS; //retourne id mot simple
                motEstTrouve = true;
                motIt = definition.begin();
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    //si mot compose trouve, on s'arrete immediatement
                    if (sousMotId == identifiantA) return identifiantC;         //retourne id mot compose
                    motCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
            else {
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    motCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
        }
    }
    if (!motEstTrouve) {
        //le mot n'est pas sur le fichier, il faut le creer
        //cree une nouvelle structure pour un mot
        definition.push_front(Mot());
        motIt = definition.begin();
        (*motIt).motSimple = motSimple;
    }
    return 0;           //mot inconnu et structure coherente pour la reecriture
}
////////////////////////////////////////////////////////////
//Ecrit les donnees de tous les mots qui ont la meme clef modulo 
void NindLexiconIndex::setDefinitionWords(const list<Mot> &definition,
                                           const Identification &lexiconIdentification)
{
    //1) calcule la taille max du buffer
    unsigned int tailleMaximum = TETE_DEFINITION + TAILLE_IDENTIFICATION;
    for (list<Mot>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {
        //<motSimple>(256) <identifiantS>(3) <nbreComposes>(3) <composes> = 262
        //<identifiantA>(3) <identifiantRelC>(4) = 7
        tailleMaximum += 262 + (*defIt).composes.size() * 7;
    }
    //2) forme le buffer a ecrire sur le fichier
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const Mot &premierMot = definition.front();
    const unsigned int ident =  clef(premierMot.motSimple) % m_modulo;
    m_file.createBuffer(tailleMaximum); 
    //<flagDefinition=17> <identifiantHash> <longueurDonnees> 
    m_file.putInt1(FLAG_DEFINITION);
    m_file.putInt3(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    for (list<Mot>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {  
        //<motSimple> <identifiantS> <nbreComposes>
        m_file.putString((*defIt).motSimple);
        m_file.putInt3((*defIt).identifiantS);
        m_file.putUIntLat((*defIt).composes.size());
        unsigned int identifiantC = (*defIt).identifiantS;
        for (list<Compose>::const_iterator compIt = (*defIt).composes.begin(); compIt != (*defIt).composes.end(); compIt++) {
            //<identifiantA> <identifiantRelC>
            m_file.putInt3((*compIt).identA);
            m_file.putSIntLat((*compIt).identComp - identifiantC);
            identifiantC = (*compIt).identComp;
        }
    }
    //ecrit la taille reelle du buffer
    const unsigned int longueurDonnees = m_file.getOutBufferSize() - TETE_DEFINITION;
    m_file.putInt3(longueurDonnees, OFFSET_LONGUEUR);  //la taille dans la trame
    //3) ecrit la definition du mot et gere le fichier
    setDefinition(ident, lexiconIdentification);   
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
    if (!m_withRetrolexicon) throw BadUseException("there is no retro lexicon");
    return m_nindRetrolexiconIndex->getComponents(ident, components);
}
////////////////////////////////////////////////////////////

