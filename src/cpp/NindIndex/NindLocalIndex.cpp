//
// C++ Implementation: NindLocalIndex
//
// Description: La gestion du fichier des index locaux
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier des index locaux qui doit rester coherent pour ses lecteurs
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
#include "NindLocalIndex.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition> <identifiantDoc> <longueurDonnees> <donneesDoc>
// <flagDefinition>        ::= <Integer1>
// <identifiantDoc>        ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesDoc>            ::= { <donneesTerme> }
// <donneesTerme>          ::= <identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
// <identTermeRelatif>     ::= <IntegerSLat>
// <categorie>             ::= <Integer1>
// <nbreLocalisations>     ::= <Integer1>
// <localisations>         ::= { <localisationRelatif> <longueur> }
// <localisationRelatif>   ::= <IntegerSLat>
// <longueur>              ::= <Integer1>
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
//<flagDefinition> <identifiantDoc> <longueurDonnees> = 7
#define TETE_DEFINITION 7
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) = 5
#define TETE_DEFINITION_MAXIMUM 5
//<flagDefinition>(1) <identifiantDoc>(3) <longueurDonnees>(3) 
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) 
//<localisationRelatif>(1) <longueur>(1) = 14
#define TAILLE_DEFINITION_MINIMUM 14
//<localisationRelatif>(2) <longueur>(1) = 3
#define TAILLE_LOC_MAXIMUM 3
//<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
#define TAILLE_IDENTIFICATION 8
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileName absolute path file name
//param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon 
//param indirectionEntryNb number of entries in a single indirection block */
NindLocalIndex::NindLocalIndex(const std::string &fileName,
                               const bool isLocalIndexWriter,
                               const unsigned int lexiconWordsNb,
                               const unsigned int lexiconIdentification,
                               const unsigned int indirectionBlocSize)
    throw(NindIndexException):
    NindIndex(fileName, 
              isLocalIndexWriter, 
              lexiconWordsNb, 
              lexiconIdentification, 
              TAILLE_DEFINITION_MINIMUM, 
              indirectionBlocSize)
{
}        
////////////////////////////////////////////////////////////
NindLocalIndex::~NindLocalIndex()
{
}
////////////////////////////////////////////////////////////
//brief Read a full document as a list of terms
//param ident ident of doc
//param localIndex structure to receive all datas of the specified doc
//return true if doc was found, false otherwise */
bool NindLocalIndex::getLocalIndex(const unsigned int ident,
                                   list<struct Term> &localIndex)
    throw(NindLocalIndexException)
{
    try {
        const bool existe = getDefinition(ident);
        if (!existe) return false;
        //<flagDefinition> <identifiantDoc> <longueurDonnees> <donneesDoc>
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindLocalIndex::getLocalIndex A : " + m_fileName);
        const unsigned int identDoc = m_file.getInt3();
        if (identDoc != ident) throw InvalidFileException("NindLocalIndex::getLocalIndex B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donnees
        m_file.setEndInBuffer(longueurDonnees);
        //<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
        unsigned int identTerme = 0;      //l'ident du terme precedent
        unsigned int position = 0;        //la position de localisation precedente
        while (!m_file.endOfInBuffer()) {
            identTerme += m_file.getSIntLat();
            const unsigned char categorie = m_file.getInt1();
            const unsigned int nbreLocalisations = m_file.getInt1();
            localIndex.push_back(Term(identTerme, categorie));
            struct Term &term = localIndex.back();
            list<Localisation> &localisation = term.localisation;
            for (unsigned int it = 0; it != nbreLocalisations; it++) {
                //<localisationRelatif> <longueur>
                position += m_file.getSIntLat();
                const unsigned int longueur = m_file.getInt1();
                localisation.push_back(Localisation(position, longueur));
            }
        }
        return true;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLocalIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief Write a full termIndex as a list of structures
//param ident ident of doc
//param localIndex structure containing all datas of the specified doc 
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
void NindLocalIndex::setLocalIndex(const unsigned int ident,
                                   const std::list<struct Term> &localIndex,
                                   const unsigned int lexiconWordsNb,
                                   const unsigned int lexiconIdentification)
    throw(NindLocalIndexException)
{
    try {
        //1) verifie que le terme n'est pas en dehors du dernier bloc d'indirection
        //il faut le faire maintenant parce que le buffer d'ecriture est unique
        checkExtendIndirection(ident, lexiconWordsNb, lexiconIdentification);
        
        //2) calcule la taille maximum du buffer d'ecriture
        //<flagDefinition> <identifiantDoc> <longueurDonnees> <donneesDoc>
        unsigned int tailleMaximum = TETE_DEFINITION;   
        for (list<struct Term>::const_iterator it1 = localIndex.begin(); it1 != localIndex.end(); it1++) {
            //<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
            //<localisationRelatif> <longueur>
            //le buffer est maximise pour écrire l'identification a la fin
            tailleMaximum += TETE_DEFINITION_MAXIMUM + (*it1).localisation.size()*TAILLE_LOC_MAXIMUM + TAILLE_IDENTIFICATION;
            //il ne doit pas etre plus petit que le minimum 
            if (tailleMaximum < TAILLE_DEFINITION_MINIMUM) tailleMaximum = TAILLE_DEFINITION_MINIMUM;
        }
        
        //3) forme le buffer a ecrire sur le fichier
        m_file.createBuffer(tailleMaximum); 
        //<flagDefinition> <identifiantDoc> <longueurDonnees> <donneesDoc>
        m_file.putInt1(FLAG_DEFINITION);
        m_file.putInt3(ident);
        m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
        unsigned int identTermePrec = 0;      //l'ident du terme precedent
        unsigned int positionPrec = 0;        //la position de localisation precedente    
        for (list<struct Term>::const_iterator it1 = localIndex.begin(); it1 != localIndex.end(); it1++) {
            //<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
            m_file.putSIntLat((*it1).term - identTermePrec);
            identTermePrec = (*it1).term;
            m_file.putInt1((*it1).cg);
            const list<struct Localisation> &localisation = (*it1).localisation;
            m_file.putInt1(localisation.size());
            for (list<struct Localisation>::const_iterator it2 = localisation.begin(); it2 != localisation.end(); it2++) {
                //<localisationRelatif> <longueur>
                m_file.putSIntLat((*it2).position - positionPrec);
                positionPrec = (*it2).position;
                m_file.putInt1((*it2).length);
            }     
        }
        //ecrit la taille reelle du buffer
        const unsigned int longueurDonnees = m_file.getOutBufferSize() - TETE_DEFINITION;
        m_file.putInt3(longueurDonnees, 4);  //la taille dans la trame
        //4) ecrit la definition du terme et gere le fichier
        setDefinition(ident, lexiconWordsNb, lexiconIdentification);
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLocalIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
