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
// <definition>            ::= <flagDefinition> <identifiantTerme>
//                                                 <longueurDonnees> <donneesTerme>
// <flagDefinition>        ::= <Integer1>
// <identifiantTerme>      ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesTerme>          ::= <termeCompose> | <termeSimple>
// <termeCompose>          ::= <flagCompose> <identifiantA> <identifiantRelS>
// <flagCompose>           ::= <Integer1>
// <identifiantA>          ::= <Integer3>
// <identifiantRelS>       ::= <IntegerSLat>
// <termeSimple>           ::= <flagSimple> <longueurTerme> <termeUtf8>
// <flagSimple>            ::= <Integer1>
// <longueurTerme>         ::= <Integer1>
// <termeUtf8>             ::= { <Octet> }
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
#define FLAG_COMPOSE 31
#define FLAG_SIMPLE 37
//<flagDefinition>(1) <identifiantTerme>(3) <longueurDonnees>(3) = 7
#define TETE_DEFINITION 7
//<flagDefinition>(1) <identifiantTerme>(3) <longueurDonnees>(3) <flagCompose>(1) <identifiantA>(3) 
//<identifiantRelS>(1) = 12
//<flagDefinition>(1) <identifiantTerme>(3) <longueurDonnees>(3) <flagSimple>(1) <longueurTerme>(1) 
//<termeUtf8>(1) = 10
#define TAILLE_DEFINITION_MINIMUM 10
//<flagDefinition>(1) <identifiantTerme>(3) <longueurDonnees>(3) <flagSimple>(1) <longueurTerme>(1) 
//<termeUtf8>(255) = 264
#define TAILLE_DEFINITION_MAXIMUM 264
//<flagCg>(1) <categorie>(1) <frequenceTerme>(3) <nbreDocs>(3) = 8
//#define TETE_DEFINITION_MAXIMUM 8
//<identDocRelatif>(3) <frequenceDoc>(2) = 5
//#define TAILLE_DOC_MAXIMUM 5
//<flagIdentification>(1) <maxIdentifiant>(3) <identifieurUnique>(4) = 8
#define TAILLE_IDENTIFICATION 8
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
//brief add a list of term idents in retro lexicon. If one of idents still exists, exception is raised
//param termDefs list of terms definitions 
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
void NindRetrolexiconIndex::addTerms(const list<struct TermDef> &termDefs,
                                     const Identification &lexiconIdentification)
{
    try {
        if (!m_isWriter) throw BadUseException("retro lexicon is not writable");
        //il y a autant d'ajout au fichier qu'il y a de nouveaux identifiants
        for (list<struct TermDef>::const_iterator it1 = termDefs.begin(); it1 != termDefs.end(); it1++) {
            const struct TermDef &termDef = (*it1);
            //1) verifie que le terme n'est pas en dehors du dernier bloc d'indirection
            //il faut le faire maintenant parce que le buffer d'ecriture est unique
            checkExtendIndirection(termDef.identifiant, lexiconIdentification);
            //2) forme le buffer a ecrire sur le fichier
            m_file.createBuffer(TAILLE_DEFINITION_MAXIMUM); 
            //<flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
            m_file.putInt1(FLAG_DEFINITION);
            m_file.putInt3(termDef.identifiant);
            m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
            //simple si pas compose (pas avec la chaine vide)
            if (termDef.identifiantA == 0) {
                //<flagSimple> <longueurTerme> <termeUtf8>
                m_file.putInt1(FLAG_SIMPLE);
                m_file.putString(termDef.termeSimple);
            }
            else {
                //<flagCompose> <identifiantA> <identifiantRelS>
                m_file.putInt1(FLAG_COMPOSE);
                m_file.putInt3(termDef.identifiantA);
                m_file.putSIntLat(termDef.identifiantS - termDef.identifiant);
            }
            //ecrit la taille reelle du buffer
            const unsigned int longueurDonnees = m_file.getOutBufferSize() - TETE_DEFINITION;
            m_file.putInt3(longueurDonnees, 4);  //la taille dans la trame
            //4) ecrit la definition du terme et gere le fichier
            setDefinition(termDef.identifiant, lexiconIdentification);           
        }
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindRetrolexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get word components from the specified ident
//param ident ident of term
//param components list of components of a word 
//(1 component = simple word, more components = compound word) */
//return true if term was found, false otherwise */
bool NindRetrolexiconIndex::getComponents(const unsigned int ident,
                                          list<string> &components)
{
    //raz resultat
    components.clear();
    //lecture du terme sur le retro lexique
    struct TermDef termDef;
    bool existe = getTermDef(ident, termDef);
    //si le terme est inconnu, retour false
    if (!existe) return false;
    //le terme existe, on le decode
    while (true) {
        if (termDef.identifiantA == 0) {
            //c'est un terme simple et c'est la fin 
            components.push_front(termDef.termeSimple);
            return true;
        }
        //c'est un terme compose
        //recupere le terme simple du couple
        struct TermDef termDefS;
        existe = getTermDef(termDef.identifiantS, termDefS);
        if (!existe) throw InvalidFileException("NindRetrolexiconIndex::getComponents A : " + m_fileName);
        if (termDefS.identifiantA != 0) InvalidFileException("NindRetrolexiconIndex::getComponents B : " + m_fileName);
        components.push_front(termDefS.termeSimple);
        //recupere l'autre terme du couple
        existe = getTermDef(termDef.identifiantA, termDef);
        if (!existe) throw InvalidFileException("NindRetrolexiconIndex::getComponents C : " + m_fileName);  
        //pour detecter les bouclages induits par un fichier bouclant
        if (components.size() == TAILLE_COMPOSE_MAXIMUM) throw InvalidFileException("NindRetrolexiconIndex::getComponents D : " + m_fileName);  
    }    
}    
////////////////////////////////////////////////////////////
//Recupere sur le fichier retro lexique la definition d'un terme specifie par son identifiant
//ident identifiant du terme
//termDef structure ou est ecrite la definition
//retourne true si le terme existe, sinon false
bool NindRetrolexiconIndex::getTermDef(const unsigned int ident,
                                       struct TermDef &termDef)   
{
    try {
        const bool existe = getDefinition(ident);
        if (!existe) return false;
        //<flagDefinition> <identifiantTerme> <longueurDonnees> <donnees>
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindRetrolexiconIndex::getTermDef A : " + m_fileName);
        const unsigned int identTerme = m_file.getInt3();
        if (identTerme != ident) throw InvalidFileException("NindRetrolexiconIndex::getTermDef B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donneesTerme
        m_file.setEndInBuffer(longueurDonnees);
        const unsigned char flag = m_file.getInt1();
        //<flagCompose> <identifiantA> <identifiantRelS>
        if (flag == FLAG_COMPOSE) termDef = TermDef(ident, m_file.getInt3(), ident + m_file.getSIntLat());
        //<flagSimple> <longueurTerme> <termeUtf8>
        else if (flag == FLAG_SIMPLE) termDef = TermDef(ident, m_file.getString());
        else throw InvalidFileException("NindRetrolexiconIndex::getTermDef C : " + m_fileName);    
        return true;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindRetrolexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
