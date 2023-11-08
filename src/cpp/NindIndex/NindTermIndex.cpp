//
// C++ Implementation: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
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
#include "NindTermIndex.h"
//#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <dejfinition>           ::= <flagDejfinition=17> <identifiantTerme>
//                             <longueurDonnejes> <donnejesTerme>
// <flagDejfinition=17>    ::= <Entier1>
// <identifiantTerme>      ::= <Entier3>
// <longueurDonnejes>      ::= <Entier3>
// <donnejesTerme>         ::= { <donnejesCG> }
// <donnejesCG>            ::= <flagCg=61> <catejgorie> <frejquenceTerme>
//                             <nbreDocs> <listeDocuments>
// <flagCg=61>             ::= <Entier1>
// <catejgorie>            ::= <Entier1>
// <frejquenceTerme>       ::= <EntierULat>
// <nbreDocs>              ::= <EntierULat>
// <listeDocuments>        ::= { <identDocRelatif> <frejquenceDoc> }
// <identDocRelatif>       ::= <EntierULat>
// <frejquenceDoc>         ::= <EntierULat>
////////////////////////////////////////////////////////////
// <spejcifique>           ::= { <valeur> }
// <valeur>                ::= <Entier4>
////////////////////////////////////////////////////////////
#define FLAG_DEJFINITION 17
#define FLAG_CG 61
//<flagDejfinition=17>(1) <identifiantTerme>(4) = 5
#define OFFSET_LONGUEUR 5
//<flagDejfinition=17>(1) <identifiantTerme>(4) <longueurDonnejes>(3) = 8
#define TAILLE_TESTE_DEJFINITION 8
//<flagDejfinition=17>(1) <identifiantTerme>(4) <longueurDonnejes>(3) <flagCg>(1) <catejgorie>(1) <frejquenceTerme>(1) 
//<nbreDocs>(1) <identDocRelatif>(5) <frejquenceDoc>(1) = 18
//le minimum doit prendre en compte le maximum dans la numejrotation !
#define TAILLE_DEJFINITION_MINIMUM 18
//<flagCg>(1) <catejgorie>(1) <frejquenceTerme>(3) <nbreDocs>(3) = 8
#define TAILLE_TESTE_DEJFINITION_MAXIMUM 8
//<identDocRelatif>(3) <frejquenceDoc>(2) = 5
#define TAILLE_DOC_MAXIMUM 5
//taille minimum du buffer d'ecriture
#define TAILLE_BUFFER_MINIMUM 128
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileNameExtensionLess absolute path file name without extension
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
//param lexiconIdentification unique identification of lexicon */
//param specificsNumber number of specific unsigned int
//param indirectionBlocSize number of entries in a single indirection block */
NindTermIndex::NindTermIndex(const std::string &fileNameExtensionLess,
                             const bool isTermIndexWriter,
                             const Identification &lexiconIdentification,
                             const unsigned int specificsNumber,
                             const unsigned int indirectionBlocSize):
    NindIndex(fileNameExtensionLess + ".nindtermindex", 
              isTermIndexWriter, 
              lexiconIdentification, 
              specificsNumber * 4,
              TAILLE_DEJFINITION_MINIMUM, 
              indirectionBlocSize),
    m_specificsNumber(specificsNumber)
{
}
////////////////////////////////////////////////////////////
NindTermIndex::~NindTermIndex()
{
}
////////////////////////////////////////////////////////////
//brief Read a full term definition as a list of structures
//param ident ident of term
//param termDef structure to receive all datas of the specified term
//return true if term was found, false otherwise */
bool NindTermIndex::getTermDef(const unsigned int ident,
                               list<struct TermCG> &termDef)
{
    const bool existe = getDefinition(ident);
    if (!existe) return false;
    //<flagDejfinition=17> <identifiantTerme> <longueurDonnejes> <donnees>
    if (m_file.getInt1() != FLAG_DEJFINITION) 
        throw NindTermIndexException("NindTermIndex::getTermDef A : " + m_fileName);
    const unsigned int identTerme = m_file.getInt4();
    if (identTerme != ident) 
        throw NindTermIndexException("NindTermIndex::getTermDef B : " + m_fileName);
    const unsigned int longueurDonnejes = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnejesTerme
    m_file.setEndInBuffer(longueurDonnejes);
    //<flagCg> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
    while (!m_file.endOfInBuffer()) {
        if (m_file.getInt1() != FLAG_CG) 
            throw NindTermIndexException("NindTermIndex::getTermDef C : " + m_fileName);
        const unsigned char catejgorie = m_file.getInt1();
        const unsigned int frejquenceTerme = m_file.getUIntLat();
        const unsigned int nbreDocs = m_file.getUIntLat();
        termDef.push_back(TermCG(catejgorie, frejquenceTerme));
        struct TermCG &termCG = termDef.back();
        list<Document> &documents = termCG.documents;
        unsigned int identDocument = 0;         //pour calculer le no de doc absolu avec la numerotation relative
        for (unsigned int it = 0; it != nbreDocs; it++) {
            //<identDocRelatif> <frejquenceDoc>
            identDocument += m_file.getUIntLat();
            const unsigned int frejquenceDoc = m_file.getUIntLat();
            documents.push_back(Document(identDocument, frejquenceDoc));
        }
    }
    return true;
}
////////////////////////////////////////////////////////////
//brief Read specifics as a list of words
//param specifics list to receive all specifics */
void NindTermIndex::getSpecificWords(list<unsigned int> &specifics)
{
    //raz
    specifics.clear();
    //lit les spejcifiques sur le fichier
    getSpecifics();
    for (unsigned int count = 0; count != m_specificsNumber; count++)
        specifics.push_back(m_file.getInt4());  
}
////////////////////////////////////////////////////////////
//brief Write a full term definition as as a list of structures
//param ident ident of term
//param termDef structure containing all datas of the specified term */
//param specifics list of specific unsigned int
//param fileIdentification unique identification of lexicon */
void NindTermIndex::setTermDef(const unsigned int ident,
                               const list<struct TermCG> &termDef,
                               const Identification &fileIdentification,
                               const list<unsigned int> &specifics)
{
    //dejbut section critique ah protejger des control-C
    m_file.beginCriticalSection();
    //1) verifie que le terme n'est pas en dehors du dernier bloc d'indirection
    //il faut le faire maintenant parce que le buffer d'ecriture est unique
    checkExtendIndirection(ident, fileIdentification);
    
    //effacement ?
    if (termDef.size() == 0) {
        //efface dans le fichier en l'ejcrivant vide
        m_file.createBuffer(getSpecificsAndIdentificationSize()); 
        //ejcrit les spejcifiques et l'identification
        writeSpecificsAndIdentification(specifics, fileIdentification);
        setDefinition(ident);
        return;
    }       
    //2) calcule la taille maximum du buffer d'ecriture
    //<flagDejfinition=17> <identifiantTerme> <longueurDonnejes> <donnejesTerme>
    unsigned int tailleMaximum = TAILLE_TESTE_DEJFINITION + getSpecificsAndIdentificationSize();    
    for (list<struct TermCG>::const_iterator it1 = termDef.begin(); it1 != termDef.end(); it1++) {
        //<flagCg> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
        //<identDocRelatif> <frejquenceDoc>
        //le buffer est maximise pour écrire l'identification a la fin
        tailleMaximum += TAILLE_TESTE_DEJFINITION_MAXIMUM + 
                            (*it1).documents.size()*TAILLE_DOC_MAXIMUM;
        //il ne doit pas etre plus petit que le minimum 
        if (tailleMaximum < TAILLE_BUFFER_MINIMUM) tailleMaximum = TAILLE_BUFFER_MINIMUM;
    }
    //3) forme le buffer a ecrire sur le fichier
    m_file.createBuffer(tailleMaximum); 
    //<flagDejfinition=17> <identifiantTerme> <longueurDonnejes> <donnejesTerme>
    m_file.putInt1(FLAG_DEJFINITION);
    m_file.putInt4(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    for (list<struct TermCG>::const_iterator it1 = termDef.begin(); it1 != termDef.end(); it1++) {
        //<flagCg> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
        m_file.putInt1(FLAG_CG);
        m_file.putInt1((*it1).cg);
        m_file.putUIntLat((*it1).frequency);
        const list<struct Document> &documents = (*it1).documents;
        m_file.putUIntLat(documents.size());
        unsigned int identPrec = 0;             //pour mettre les identifiants de documents en relatif
        for (list<struct Document>::const_iterator it2 = documents.begin(); it2 != documents.end(); it2++) {
            //<identDocRelatif> <frejquenceDoc>
            m_file.putUIntLat((*it2).ident - identPrec);
            m_file.putUIntLat((*it2).frequency);
            identPrec = (*it2).ident;
        }
    }
    //ecrit la taille reelle du buffer
    const unsigned int longueurDonnejes = m_file.getOutBufferSize() - TAILLE_TESTE_DEJFINITION;
    m_file.putInt3(longueurDonnejes, OFFSET_LONGUEUR);  //la taille dans la trame
    //calcule le nombre d'extra octets ah mettre dans le buffer pour atteindre la taille minimum
    int extra = m_file.getOutBufferSize() - TAILLE_DEJFINITION_MINIMUM;
    while (extra++ < 0) m_file.putInt1(0);
    //4) ejcrit les spejcifiques et l'identification
    writeSpecificsAndIdentification(specifics, fileIdentification);
    setDefinition(ident);
    //fin section critique ah protejger des control-C
    m_file.endCriticalSection();
}
////////////////////////////////////////////////////////////
//brief write specifics footer and identification into write buffer
//param specifics list of specific unsigned int
//param fileIdentification unique identification of file */
void NindTermIndex::writeSpecificsAndIdentification(const list<unsigned int> &specifics,
                                                    const Identification &fileIdentification)
{
    //controsle de la taille des spejcifiques
    if (specifics.size() != m_specificsNumber) 
        throw NindTermIndexException("NindTermIndex::writeSpecificsAndIdentification" + m_fileName);
    writeSpecificsHeader();
    //{ <valeur> }
    for(list<unsigned int>::const_iterator specIt = specifics.begin(); specIt != specifics.end(); specIt++)
        m_file.putInt4(*specIt);
    writeIdentification(fileIdentification);
}
////////////////////////////////////////////////////////////
