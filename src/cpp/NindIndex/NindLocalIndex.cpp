//
// C++ Implementation: NindLocalIndex
//
// Description: La gestion du fichier des index locaux
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier des index locaux qui doit rester coherent pour ses lecteurs
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
#include "NindLocalIndex.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <dejfinition>           ::= <flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> <donnejesDoc>
// <flagDejfinition=19>    ::= <Integer1>
// <identifiantDoc>        ::= <Integer3>
// <identifiantExterne>    ::= <Integer4>
// <longueurDonnejes>      ::= <Integer3>
// <donnejesDoc>           ::= { <donnejesTerme> }
// <donnejesTerme>         ::= <identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
// <identTermeRelatif>     ::= <IntegerSLat>
// <catejgorie>            ::= <Integer1>
// <nbreLocalisations>     ::= <Integer1>
// <localisations>         ::= { <localisationRelatif> <longueur> }
// <localisationRelatif>   ::= <IntegerSLat>
// <longueur>              ::= <Integer1>
////////////////////////////////////////////////////////////
// <spejcifique>           ::= <maxIdentifiantInterne> <nombreDocuments>
// <maxIdentifiantInterne> ::= <Integer4>
// <nombreDocuments>       ::= <Integer4>
////////////////////////////////////////////////////////////
#define FLAG_DEJFINITION 19
//<flagDejfinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) = 8
#define TAILLE_TESTE_IDENT_EXTERNE 8
//<flagDejfinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) = 8
#define OFFSET_LONGUEUR 8
//<flagDejfinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) <longueurDonnejes>(3) = 11
#define TAILLE_TESTE_DEJFINITION 11
//<identTermeRelatif>(3) <catejgorie>(1) <nbreLocalisations>(1) = 5
#define TAILLE_TESTE_DEJFINITION_MAXIMUM 5
//<flagDejfinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) <longueurDonnejes>(3) 
//<identTermeRelatif>(3) <catejgorie>(1) <nbreLocalisations>(1) 
//<localisationRelatif>(1) <longueur>(1) = 18
#define TAILLE_DEJFINITION_MINIMUM 18
//<localisationRelatif>(2) <longueur>(1) = 3
#define TAILLE_LOC_MAXIMUM 3
//taille minimum du buffer d'ecriture
#define TAILLE_BUFFER_MINIMUM 128
//taille des spejcifiques
#define TAILLE_SPEJCIFIQUES 8
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileNameExtensionLess absolute path file name without extension
//param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
//param lexiconIdentification unique identification of lexicon 
//param indirectionBlocSize number of entries in a single indirection block */
NindLocalIndex::NindLocalIndex(const std::string &fileNameExtensionLess,
                               const bool isLocalIndexWriter,
                               const Identification &lexiconIdentification,
                               const unsigned int indirectionBlocSize):
    NindIndex(fileNameExtensionLess + ".nindlocalindex", 
              isLocalIndexWriter, 
              lexiconIdentification, 
              TAILLE_SPEJCIFIQUES,
              TAILLE_DEJFINITION_MINIMUM, 
              indirectionBlocSize),
    m_docIdTradExtInt(),
    m_currIdent(0)
{
    //lit le plus grand identifiant interne utilisej
    getSpecifics();
    //initialise la map de traduction des id externes -> id internes
    fillDocIdTradExtInt(1, m_file.getInt4() +1);
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
    //raz rejsultat
    localDef.clear();
    //trouve l'identifiant interne du doc 
    const unsigned int identInt = getInternalIdent(ident);
    //si pas trouvej retourne faux
    if (identInt == 0) return false;
    //rejcupehre la dejfinition
    const bool existe = getDefinition(identInt);
    if (!existe) return false;
    //<flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> <donnejesDoc>
    if (m_file.getInt1() != FLAG_DEJFINITION) 
        throw NindLocalIndexException("NindLocalIndex::getLocalIndex A : " + m_fileName);
    const unsigned int identDoc = m_file.getInt3();
    if (identDoc != identInt) 
        throw NindLocalIndexException("NindLocalIndex::getLocalIndex B : " + m_fileName);
    const unsigned int identExt = m_file.getInt4();
    if (identExt != ident) 
        throw NindLocalIndexException("NindLocalIndex::getLocalIndex C : " + m_fileName);
    const unsigned int longueurDonnejes = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnees
    m_file.setEndInBuffer(longueurDonnejes);
    //<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
    unsigned int identTerme = 0;      //l'ident du terme precedent
    unsigned int position = 0;        //la position de localisation precedente
    while (!m_file.endOfInBuffer()) {
        identTerme += m_file.getSIntLat();
        const unsigned char catejgorie = m_file.getInt1();
        const unsigned int nbreLocalisations = m_file.getInt1();
        localDef.push_back(Term(identTerme, catejgorie));
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
//param fileIdentification unique identification of lexicon */
void NindLocalIndex::setLocalDef(const unsigned int ident,
                                 const std::list<struct Term> &localDef,
                                 const Identification &fileIdentification)
{
    //est-ce que ce document est dejah connu ?
    map<unsigned int, unsigned int>::const_iterator itident = m_docIdTradExtInt.find(ident);
    //si non, on le creje
    if (itident == m_docIdTradExtInt.end()) {
        //sauf si c'est pour un effacement, auquel cas, rien n'est fait
        if (localDef.size() == 0) return;
        m_docIdTradExtInt[ident] = ++m_currIdent;
        itident = m_docIdTradExtInt.find(ident);
    }
    const unsigned int identInt = (*itident).second;
    //1) verifie que l'identInt n'est pas en dehors du dernier bloc d'indirection
    //il faut le faire maintenant parce que le buffer d'ecriture est unique
    checkExtendIndirection(identInt, fileIdentification);
    //effacement ?
    if (localDef.size() == 0) {
        //efface dans la map de traduction des ids
        m_docIdTradExtInt.erase(itident);
        //efface dans le fichier en l'ejcrivant vide
        m_file.createBuffer(getSpecificsAndIdentificationSize()); 
        //ejcrit les spejcifiques et l'identification
        writeSpecificsAndIdentification(fileIdentification);
        setDefinition(identInt);
        return;
    }
    //2) calcule la taille maximum du buffer d'ecriture
    //<flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> <donnejesDoc>
    unsigned int tailleMaximum = TAILLE_TESTE_DEJFINITION  + getSpecificsAndIdentificationSize();   
    for (list<struct Term>::const_iterator it1 = localDef.begin(); it1 != localDef.end(); it1++) {
        //<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
        //<localisationRelatif> <longueur>
        //le buffer est maximise pour écrire l'identification a la fin
        tailleMaximum += TAILLE_TESTE_DEJFINITION_MAXIMUM + 
                            (*it1).localisation.size()*TAILLE_LOC_MAXIMUM;
        //il ne doit pas etre plus petit que le minimum 
        if (tailleMaximum < TAILLE_BUFFER_MINIMUM) tailleMaximum = TAILLE_BUFFER_MINIMUM;
    }
    
    //3) forme le buffer a ecrire sur le fichier
    m_file.createBuffer(tailleMaximum); 
    //<flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> <donnejesDoc>
    m_file.putInt1(FLAG_DEJFINITION);
    m_file.putInt3(identInt);
    m_file.putInt4(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    unsigned int identTermePrec = 0;      //l'ident du terme precedent
    unsigned int positionPrec = 0;        //la position de localisation precedente    
    for (list<struct Term>::const_iterator it1 = localDef.begin(); it1 != localDef.end(); it1++) {
        //<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
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
    const unsigned int longueurDonnejes = m_file.getOutBufferSize() - TAILLE_TESTE_DEJFINITION;
    m_file.putInt3(longueurDonnejes, OFFSET_LONGUEUR);  //la taille dans la trame
    //calcule le nombre d'extra octets ah mettre dans le buffer pour atteindre la taille minimum
    int extra = m_file.getOutBufferSize() - TAILLE_DEJFINITION_MINIMUM;
    while (extra++ < 0) m_file.putInt1(0);
    //4) ejcrit les spejcifiques et l'identification
    writeSpecificsAndIdentification(fileIdentification);
    setDefinition(identInt);
}
////////////////////////////////////////////////////////////
//brief number of documents in the collection 
//return number of documents in the collection */
unsigned int NindLocalIndex::getDocCount() 
{
    //lit le nombre de documents sur le fichier
    getSpecifics();
    m_file.getInt4();
    return m_file.getInt4();
}
////////////////////////////////////////////////////////////
//brief write specifics footer and identification into write buffer
//param fileIdentification unique identification of file */
void NindLocalIndex::writeSpecificsAndIdentification(const Identification &fileIdentification)
{
    writeSpecificsHeader();
    //<maxIdentifiantInterne> <nombreDocuments>
    m_file.putInt4(m_currIdent);
    m_file.putInt4(m_docIdTradExtInt.size());
    writeIdentification(fileIdentification);
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
    //lit le plus grand identifiant interne utilisej
    getSpecifics();
    //met ah jour la map
    //initialise la map de traduction des id externes -> id internes uniquement pour les nouveaux docs
    fillDocIdTradExtInt(m_currIdent +1, m_file.getInt4() +1);
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
        const bool existe = getDefinition(ident, TAILLE_TESTE_IDENT_EXTERNE);
        if (!existe) continue;
        //<flagDejfinition=19> <identifiantDoc> <identifiantExterne> 
        if (m_file.getInt1() != FLAG_DEJFINITION) 
            throw NindLocalIndexException("NindLocalIndex::fillDocIdTradExtInt A : " + m_fileName);
        const unsigned int identDoc = m_file.getInt3();
        if (identDoc != ident) 
            throw NindLocalIndexException("NindLocalIndex::fillDocIdTradExtInt B : " + m_fileName);
        const unsigned int identExt = m_file.getInt4();       //<identifiantExterne>
        m_docIdTradExtInt[identExt] = identDoc;
        m_currIdent = identDoc;
    }
}
////////////////////////////////////////////////////////////
