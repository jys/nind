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
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
// <flagDefinition>        ::= <Integer1>
// <identifiantHash>       ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesHash>           ::= { <terme> }
// <terme>                 ::= <termeSimple> <identifiantS> <nbreComposes> <composes>
// <termeSimple>           ::= <longueurTerme> <termeUtf8>
// <longueurTerme>         ::= <Integer1>
// <termeUtf8>             ::= { <Octet> }
// <identifiantS>          ::= <Integer3>
// <nbreComposes>          ::= <IntegerULat>
// <composes>              ::= { <compose> } 
// <compose>               ::= <identifiantA> <identifiantRelC>
// <identifiantA>          ::= <Integer3>
// <identifiantRelC>       ::= <IntegerSLat>
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
//<flagDefinition> <identifiantHash> <longueurDonnees> = 7
#define TETE_DEFINITION 7
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) = 5
//#define TETE_DEFINITION_MAXIMUM 5
//<flagDefinition>(1) <identifiantHash>(3) <longueurDonnees>(3) 
//<clefA>(4) <clefB>(4) <identifiantS>(3) <nbreComposes>(1) = 19
#define TAILLE_DEFINITION_MINIMUM 19
//<localisationRelatif>(2) <longueur>(1) = 3
//#define TAILLE_LOC_MAXIMUM 3
//<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
#define TAILLE_IDENTIFICATION 8
////////////////////////////////////////////////////////////
static unsigned int clef(const string &mot);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconIndex.
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param indirectionBlocSize number of entries in a single indirection block */
NindLexiconIndex::NindLexiconIndex(const string &fileName,
                                   const bool isLexiconWriter,
                                   const unsigned int indirectionBlocSize)
    throw(NindIndexException):
    NindIndex(fileName, 
        isLexiconWriter, 
        0, 
        0, 
        TAILLE_DEFINITION_MINIMUM, 
        indirectionBlocSize),
    m_modulo(0),
    m_currentId(0),
    m_identification(0)
{
    //la taille du bloc d'indirection du fichier reel est structurante
    m_modulo = getFirstIndirectionBlockSize(); 
    //l'identifiant de terme le plus eleve (pour l'ecrivain)
    getFileIdentification(m_currentId, m_identification);
}
////////////////////////////////////////////////////////////
NindLexiconIndex::~NindLexiconIndex()
{
}
////////////////////////////////////////////////////////////
//brief add specified term in lexicon it doesn't still exist in,
//In all cases, word ident is returned.
//param componants list of componants of a word 
//(1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconIndex::addWord(const list<string> &componants)
{
    if (!m_isWriter) throw BadUseException("lexicon is not writable");
    //identifiant du mot (simple ou compose) sous ensemble du mot examine
    unsigned int sousMotId = 0;
    for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
        bool estNouveau = false;
        const string &motSimple = *swIt;
        list<Terme> definition;
        list<Terme>::iterator termeIt;
        //cherche l'identifiant du terme sur le fichier et prepare la structure de reecriture si pas trouve
        const unsigned int termeId = getDefinitionTermes(motSimple, sousMotId, definition, termeIt);
        if (termeId != 0) {
            //trouve, on passe au suivant
            sousMotId = termeId;
            continue;
        }
        //pas trouve
        //si terme simple inconnu, on le cree
        if ((*termeIt).identifiantS == 0) (*termeIt).identifiantS = ++m_currentId;
        //si terme compose, on le cree
        if (sousMotId != 0) (*termeIt).composes.push_back(Compose(sousMotId, ++m_currentId));
        sousMotId = m_currentId;
        //et on ecrit sur le fichier
        m_identification = (time_t)time(NULL);
        setDefinitionTermes(definition, m_currentId, m_identification);
    }
    //retourne l'id du mot specifie
    return sousMotId;
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconIndex::getId(const list<string> &componants)
    throw(NindLexiconIndexException)
{
    try {
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
        unsigned int sousMotId = 0;
        for(list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
            const string &motSimple = *swIt;
            const unsigned int termeId = getIdentifiant(motSimple, sousMotId);
            if (termeId == 0) return 0;     //mot inconnu
            sousMotId = termeId;
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
//param wordsNb where number of words contained in lexicon is returned
//param identification where unique identification of lexicon is returned */
void NindLexiconIndex::getIdentification(unsigned int &wordsNb, unsigned int &identification)
{
    wordsNb = m_currentId;
    identification = m_identification;
}
////////////////////////////////////////////////////////////
//Recupere l'identifiant d'un terme sur le fichier lexique
//retourne l'identifiant du terme s'il existe, 0 s'il n'existe pas
unsigned int NindLexiconIndex::getIdentifiant(const string &termeSimple,
                                              const unsigned int sousMotId)
    throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException)
{
    //l'identifiant dans le fichier est la clef calculee modulo la taille du bloc d'indirection
    const unsigned int ident =  clef(termeSimple) % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (!existe) return 0;              //pas trouve de definition => mot inconnu
    //<flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
    if (m_file.getInt1() != FLAG_DEFINITION) 
        throw InvalidFileException("NindLexiconIndex::getDefinitionTerme A : " + m_fileName);
    if (m_file.getInt3() != ident) 
        throw InvalidFileException("NindLexiconIndex::getDefinitionTerme B : " + m_fileName);
    const unsigned int longueurDonnees = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnees
    m_file.setEndInBuffer(longueurDonnees);
    while (!m_file.endOfInBuffer()) {
        //<termeSimple> <identifiantS> <nbreComposes> <composes>
        const string ftermeSimple = m_file.getString();
        if (ftermeSimple == termeSimple) {
            //c'est le bon terme
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
        //ce n'est pas le bon terme, on le saute
        m_file.getInt3();
        const unsigned int nbreComposes = m_file.getUIntLat();
        for (unsigned int i = 0; i != nbreComposes; i++) {
            //<identifiantA> <identifiantRelC>
            m_file.getInt3();
            m_file.getSIntLat();
        }        
    }
    return 0;           //pas trouve le terme simple => mot inconnu  
}
////////////////////////////////////////////////////////////
//recupere les donnees de tous les termes qui ont la meme clef modulo 
//retourne l'identifiant du terme s'il existe, sinon retourne 0
//si le terme n'existe pas, la structure retournée est valide, sinon elle ne l'est pas
unsigned int NindLexiconIndex::getDefinitionTermes(const string &termeSimple,
                                                   const unsigned int sousMotId,
                                                   list<Terme> &definition,
                                                   list<Terme>::iterator &termeIt)
    throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException)
{
    bool termeEstTrouve = false;
    //l'identifiant dans le fichier est la clef calculee modulo la taille du bloc d'indirection
    const unsigned int ident =  clef(termeSimple) % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (existe) {
        //<flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
        if (m_file.getInt1() != FLAG_DEFINITION) 
            throw InvalidFileException("NindLexiconIndex::getDefinitionTerme A : " + m_fileName);
        if (m_file.getInt3() != ident) 
            throw InvalidFileException("NindLexiconIndex::getDefinitionTerme B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donnees
        m_file.setEndInBuffer(longueurDonnees);
        while (!m_file.endOfInBuffer()) {
            //cree une nouvelle structure pour un terme
            definition.push_front(Terme());
            Terme &termeCourant = definition.front();
            //<termeSimple> <identifiantS> <nbreComposes> <composes>
            termeCourant.termeSimple = m_file.getString();
            //cerr<<"NindLexiconIndex::getDefinitionTermes  termeCourant.termeSimple="<<termeCourant.termeSimple<<endl;
            termeCourant.identifiantS = m_file.getInt3();
            const unsigned int nbreComposes = m_file.getUIntLat();
            unsigned int identifiantC = termeCourant.identifiantS;
            //est-ce le terme cherche ?
            if (termeCourant.termeSimple == termeSimple) {
                if (sousMotId == 0) return termeCourant.identifiantS; //retourne id terme simple
                termeEstTrouve = true;
                termeIt = definition.begin();
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    //si terme compose trouve, on s'arrete immediatement
                    if (sousMotId == identifiantA) return identifiantC;         //retourne id terme compose
                    termeCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
            else {
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    termeCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
        }
    }
    if (!termeEstTrouve) {
        //le terme n'est pas sur le fichier, il faut le creer
        //cree une nouvelle structure pour un terme
        definition.push_front(Terme());
        termeIt = definition.begin();
        (*termeIt).termeSimple = termeSimple;
    }
    return 0;           //terme inconnu et structure coherente pour la reecriture
}
////////////////////////////////////////////////////////////
//Ecrit les donnees de tous les termes qui ont la meme clef modulo 
void NindLexiconIndex::setDefinitionTermes(const list<Terme> &definition,
                                           const unsigned int lexiconWordsNb,
                                           const unsigned int lexiconIdentification)
{
    //1) calcule la taille max du buffer
    unsigned int tailleMaximum = TETE_DEFINITION + TAILLE_IDENTIFICATION;
    //cerr<<"NindLexiconIndex::setDefinitionTermes definition.size()="<<definition.size()<<endl;
    for (list<Terme>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {
        //<termeSimple> <identifiantS> <nbreComposes> <composes> = 262
        //<identifiantA> <identifiantRelC> = 7
        tailleMaximum += 262 + (*defIt).composes.size() * 7;
    }
    //2) forme le buffer a ecrire sur le fichier
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const Terme &premierTerme = definition.front();
    const unsigned int ident =  clef(premierTerme.termeSimple) % m_modulo;
    m_file.createBuffer(tailleMaximum); 
    //<flagDefinition> <identifiantHash> <longueurDonnees> 
    m_file.putInt1(FLAG_DEFINITION);
    m_file.putInt3(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    for (list<Terme>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {  
        //<termeSimple> <identifiantS> <nbreComposes>
        m_file.putString((*defIt).termeSimple);
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
    m_file.putInt3(longueurDonnees, 4);  //la taille dans la trame
    //3) ecrit la definition du terme et gere le fichier
    setDefinition(ident, lexiconWordsNb, lexiconIdentification);   
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
