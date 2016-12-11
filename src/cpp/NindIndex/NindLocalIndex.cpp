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
// <definition>            ::= <flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees> <donneesDoc>
// <flagDefinition=19>     ::= <Integer1>
// <identifiantDoc>        ::= <Integer3>
// <identifiantExterne>    ::= <Integer4>
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
#define FLAG_DEFINITION 19
//<flagDefinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) = 8
#define TETE_IDENT_EXTERNE 8
//<flagDefinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) = 8
#define OFFSET_LONGUEUR 8
//<flagDefinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) <longueurDonnees>(3) = 11
#define TETE_DEFINITION 11
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) = 5
#define TETE_DEFINITION_MAXIMUM 5
//<flagDefinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) <longueurDonnees>(3) 
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) 
//<localisationRelatif>(1) <longueur>(1) = 18
#define TAILLE_DEFINITION_MINIMUM 18
//<localisationRelatif>(2) <longueur>(1) = 3
#define TAILLE_LOC_MAXIMUM 3
//taille minimum du buffer d'ecriture
#define TAILLE_BUFFER_MINIMUM 128
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileName absolute path file name
//param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
//param lexiconIdentification unique identification of lexicon 
//param indirectionEntryNb number of entries in a single indirection block */
NindLocalIndex::NindLocalIndex(const std::string &fileName,
                               const bool isLocalIndexWriter,
                               const Identification &lexiconIdentification,
                               const unsigned int indirectionBlocSize):
    NindIndex(fileName, 
              isLocalIndexWriter, 
              lexiconIdentification, 
              TAILLE_DEFINITION_MINIMUM, 
              indirectionBlocSize),
    m_identification(),
    m_docIdTradExtInt(),
    m_currIdent(0)
{
    try {
        //mejmorise l'identification du fichier
        getFileIdentification(m_identification);
        //initialise la map de traduction des id externes -> id internes
        fillDocIdTradExtInt(1, m_identification.specificFileIdent +1);
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLocalIndexException(m_fileName);
    }
}        
////////////////////////////////////////////////////////////
NindLocalIndex::~NindLocalIndex()
{
}
////////////////////////////////////////////////////////////
//brief Return a full document as a list of terms whith their localisations
//param ident ident of doc
//param localDef structure to receive all datas of the specified doc
//return true if doc was found, false otherwise */
bool NindLocalIndex::getLocalDef(const unsigned int ident,
                                 list<struct Term> &localDef)
{
    try {
        //raz rejsultat
        localDef.clear();
        //trouve l'identifiant interne du doc 
        const unsigned int identInt = getInternalIdent(ident);
        //si pas trouvej retourne faux
        if (identInt == 0) return false;
        //rejcupehre la dejfinition
        const bool existe = getDefinition(identInt);
        if (!existe) return false;
        //<flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees> <donneesDoc>
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindLocalIndex::getLocalIndex A : " + m_fileName);
        const unsigned int identDoc = m_file.getInt3();
        if (identDoc != identInt) throw InvalidFileException("NindLocalIndex::getLocalIndex B : " + m_fileName);
        const unsigned int identExt = m_file.getInt4();
        if (identExt != ident) throw InvalidFileException("NindLocalIndex::getLocalIndex C : " + m_fileName);
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
            localDef.push_back(Term(identTerme, categorie));
            struct Term &term = localDef.back();
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
//brief Return a full document as a list of unique terms ids
//param ident ident of doc
//param termIdents structure to receive all datas of the specified doc
//return true if doc was found, false otherwise */
bool NindLocalIndex::getTermIdents(const unsigned int ident,
                                   set<unsigned int> &termIdents)
{
    //raz rejsultat
    termIdents.clear();
    //rejcupehre la structure du document
    list<struct Term> localDef;
    const bool existe = getLocalDef(ident, localDef);
    if (!existe) return false;
    //trouve tous les identifiants de termes de ce document
    for (list<struct Term>::const_iterator it = localDef.begin(); it != localDef.end(); it++) {
        const NindLocalIndex::Term &term = (*it);
        termIdents.insert(term.term);
    } 
    return true;
}
////////////////////////////////////////////////////////////
//brief Write a full document as a list of terms whith their localisations
//param ident ident of doc
//param localDef structure containing all datas of the specified doc. empty when deletion
//param lexiconIdentification unique identification of lexicon */
void NindLocalIndex::setLocalDef(const unsigned int ident,
                                 const std::list<struct Term> &localDef,
                                 const Identification &lexiconIdentification)
{
    try {
        //est-ce que ce document est dejah connu ?
        map<unsigned int, unsigned int>::const_iterator itident = m_docIdTradExtInt.find(ident);
        //si non, on le creje
        if (itident == m_docIdTradExtInt.end()) {
            m_docIdTradExtInt[ident] = ++m_currIdent;
            itident = m_docIdTradExtInt.find(ident);
        }
        const unsigned int identInt = (*itident).second;
        //ejtablit la nouvelle identification
        m_identification = lexiconIdentification;
        m_identification.specificFileIdent = m_currIdent;
        //1) verifie que l'identInt n'est pas en dehors du dernier bloc d'indirection
        //il faut le faire maintenant parce que le buffer d'ecriture est unique
        checkExtendIndirection(identInt, m_identification);
        //effacement ?
        if (localDef.size() == 0) {
            deleteLocalIndex(ident, m_identification);
            return;
        }
        //2) calcule la taille maximum du buffer d'ecriture
        //<flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees> <donneesDoc>
        unsigned int tailleMaximum = TETE_DEFINITION;   
        for (list<struct Term>::const_iterator it1 = localDef.begin(); it1 != localDef.end(); it1++) {
            //<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
            //<localisationRelatif> <longueur>
            //le buffer est maximise pour écrire l'identification a la fin
            tailleMaximum += TETE_DEFINITION_MAXIMUM + (*it1).localisation.size()*TAILLE_LOC_MAXIMUM + TAILLE_IDENTIFICATION;
            //il ne doit pas etre plus petit que le minimum 
            if (tailleMaximum < TAILLE_BUFFER_MINIMUM) tailleMaximum = TAILLE_BUFFER_MINIMUM;
        }
        
        //3) forme le buffer a ecrire sur le fichier
        m_file.createBuffer(tailleMaximum); 
        //<flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees> <donneesDoc>
        m_file.putInt1(FLAG_DEFINITION);
        m_file.putInt3(identInt);
        m_file.putInt4(ident);
        m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
        unsigned int identTermePrec = 0;      //l'ident du terme precedent
        unsigned int positionPrec = 0;        //la position de localisation precedente    
        for (list<struct Term>::const_iterator it1 = localDef.begin(); it1 != localDef.end(); it1++) {
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
        m_file.putInt3(longueurDonnees, OFFSET_LONGUEUR);  //la taille dans la trame
        //4) ecrit la definition du terme et gere le fichier
        setDefinition(identInt, m_identification);
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLocalIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief number of documents in the collection 
//return number of documents in the collection */
unsigned int NindLocalIndex::getDocCount() const
{
    return m_docIdTradExtInt.size();
}
////////////////////////////////////////////////////////////
void NindLocalIndex::deleteLocalIndex(const unsigned int ident,
                                      const Identification &identification)
{
    map<unsigned int, unsigned int>::const_iterator itident = m_docIdTradExtInt.find(ident);
    //si effacement de pas connu, raf
    if (itident == m_docIdTradExtInt.end()) return;
    const unsigned int identInt = (*itident).second;
    //efface dans le fichier
    m_file.createBuffer(0); 
    setDefinition(identInt, identification);
    //n'efface pas dans la map de traduction des identifiants, le fichier sera trouvej vide
    //m_docIdTradExtInt.erase(itident);
}
////////////////////////////////////////////////////////////
//Rejcupehre l'identifiant interne 
unsigned int NindLocalIndex::getInternalIdent(const unsigned int ident)
{
    //cherche l'identifiant dans la map courante
    map<unsigned int, unsigned int>::const_iterator itident = m_docIdTradExtInt.find(ident);
    //si l'identifiant externe est connu, retourne l'identifiant interne
    if (itident != m_docIdTradExtInt.end()) return (*itident).second;
    //l'identifiant externe n'est pas connu
    //regarde si le fichier a changej depuis le dernier calcul de traduction des identifiants
    Identification identification;
    getFileIdentification(identification);
    //si pas de changement, identifiant inconnu, retourne 0
    if (identification == m_identification) return 0;
    //met ah jour l'identification
    m_identification = identification;
    //met ah jour la map
    //initialise la map de traduction des id externes -> id internes
    fillDocIdTradExtInt(m_currIdent +1, m_identification.specificFileIdent +1);
    //cherche l'identifiant dans la map courante
    itident = m_docIdTradExtInt.find(ident);
    //si l'identifiant externe est connu, retourne l'identifiant interne
    if (itident != m_docIdTradExtInt.end()) return (*itident).second;
    //sinon retourne 0
    return 0;
}
////////////////////////////////////////////////////////////
//met ah jour la map de traduction des id externes -> id internes
void NindLocalIndex::fillDocIdTradExtInt(const unsigned int intIdMin,
                                         const unsigned int intIdMax) 
{
    for (unsigned int ident = intIdMin; ident != intIdMax; ident++) {
        const bool existe = getDefinition(ident, TETE_IDENT_EXTERNE);
        if (!existe) continue;
        //<flagDefinition=19> <identifiantDoc> <identifiantExterne> 
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindLocalIndex::fillDocIdTradExtInt A : " + m_fileName);
        const unsigned int identDoc = m_file.getInt3();
        if (identDoc != ident) throw InvalidFileException("NindLocalIndex::fillDocIdTradExtInt B : " + m_fileName);
        const unsigned int identExt = m_file.getInt4();       //<identifiantExterne>
        m_docIdTradExtInt[identExt] = identDoc;
        m_currIdent = identDoc;
    }
}
////////////////////////////////////////////////////////////
