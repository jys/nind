//
// C++ Implementation: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindTermIndex.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
// <flagDefinition>        ::= <Integer1>
// <identifiantTerme>      ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesTerme>          ::= { <donneesCG> }
// <donneesCG>             ::= <flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
// <flagCg>                ::= <Integer1>
// <categorie>             ::= <Integer1>
// <frequenceTerme>        ::= <IntegerULat>
// <nbreDocs>              ::= <IntegerULat>
// <listeDocuments>        ::= { <identDocRelatif> <frequenceDoc> }
// <identDocRelatif>       ::= <IntegerULat>
// <frequenceDoc>          ::= <IntegerULat>
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
#define FLAG_CG 61
//<flagDefinition> <identifiantTerme> <longueurDonnees> = 7
#define TETE_DEFINITION 7
//<flagDefinition>(1) <identTerme>(3) <longueurDonnees>(3) <flagCg>(1) <categorie>(1) <frequenceTerme>(1) 
//<nbreDocs>(1) <identDocRelatif>(3) <frequenceDoc>(1) = 15
#define TAILLE_DEFINITION_MINIMUM 15
//<flagCg>(1) <categorie>(1) <frequenceTerme>(3) <nbreDocs>(3) = 8
#define TETE_DEFINITION_MAXIMUM 8
//<identDocRelatif>(3) <frequenceDoc>(2) = 5
#define TAILLE_DOC_MAXIMUM 5
//<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
#define TAILLE_IDENTIFICATION 8
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileName absolute path file name
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
//param indirectionBlocSize number of entries in a single indirection block */
NindTermIndex::NindTermIndex(const std::string &fileName,
                             const bool isTermIndexWriter,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification,
                             const unsigned int indirectionBlocSize)
    throw(NindIndexException):
    NindIndex(fileName, 
              isTermIndexWriter, 
              lexiconWordsNb, 
              lexiconIdentification, 
              TAILLE_DEFINITION_MINIMUM, 
              indirectionBlocSize)
{
}
////////////////////////////////////////////////////////////
NindTermIndex::~NindTermIndex()
{
}
////////////////////////////////////////////////////////////
//brief Read a full termIndex as a list of structures
//param ident ident of term
//param termIndex structure to receive all datas of the specified term
//return true if term was found, false otherwise */
bool NindTermIndex::getTermIndex(const unsigned int ident,
                                 list<struct TermCG> &termIndex)
    throw(NindTermIndexException)
{
    try {
        const bool existe = getDefinition(ident);
        if (!existe) return false;
        //<flagDefinition> <identifiantTerme> <longueurDonnees> <donnees>
        if (m_file.getInt1() != FLAG_DEFINITION) throw InvalidFileException("NindTermIndex::getTermIndex A : " + m_fileName);
        const unsigned int identTerme = m_file.getInt3();
        if (identTerme != ident) throw InvalidFileException("NindTermIndex::getTermIndex B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donneesTerme
        m_file.setEndInBuffer(longueurDonnees);
        //<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
        while (!m_file.endOfInBuffer()) {
            if (m_file.getInt1() != FLAG_CG) throw InvalidFileException("NindTermIndex::getTermIndex C : " + m_fileName);
            const unsigned char categorie = m_file.getInt1();
            const unsigned int frequenceTerme = m_file.getUIntLat();
            const unsigned int nbreDocs = m_file.getUIntLat();
            termIndex.push_back(TermCG(categorie, frequenceTerme));
            struct TermCG &termCG = termIndex.back();
            list<Document> &documents = termCG.documents;
            unsigned int identDocument = 0;         //pour calculer le no de doc absolu avec la numerotation relative
            for (unsigned int it = 0; it != nbreDocs; it++) {
                //<identDocRelatif> <frequenceDoc>
                identDocument += m_file.getUIntLat();
                const unsigned int frequenceDoc = m_file.getUIntLat();
                documents.push_back(Document(identDocument, frequenceDoc));
            }
        }
        return true;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindTermIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief Write a full termIndex as as a list of structures
//param ident ident of term
//param termIndex structure containing all datas of the specified term */
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
void NindTermIndex::setTermIndex(const unsigned int ident,
                                 const list<struct TermCG> &termIndex,
                                 const unsigned int lexiconWordsNb,
                                 const unsigned int lexiconIdentification)
    throw(NindTermIndexException)
{
    try {
        //1) verifie que le terme n'est pas en dehors du dernier bloc d'indirection
        //il faut le faire maintenant parce que le buffer d'ecriture est unique
        checkExtendIndirection(ident, lexiconWordsNb, lexiconIdentification);
        
        //2) calcule la taille maximum du buffer d'ecriture
        //<flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
        unsigned int tailleMaximum = TETE_DEFINITION;   
        for (list<struct TermCG>::const_iterator it1 = termIndex.begin(); it1 != termIndex.end(); it1++) {
            //<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
            //<identDocRelatif> <frequenceDoc>
            //le buffer est maximise pour écrire l'identification a la fin
            tailleMaximum += TETE_DEFINITION_MAXIMUM + (*it1).documents.size()*TAILLE_DOC_MAXIMUM + TAILLE_IDENTIFICATION;        
            //il ne doit pas etre plus petit que le minimum 
            if (tailleMaximum < TAILLE_DEFINITION_MINIMUM) tailleMaximum = TAILLE_DEFINITION_MINIMUM;
        }
        //3) forme le buffer a ecrire sur le fichier
        m_file.createBuffer(tailleMaximum); 
        //<flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
        m_file.putInt1(FLAG_DEFINITION);
        m_file.putInt3(ident);
        m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
        for (list<struct TermCG>::const_iterator it1 = termIndex.begin(); it1 != termIndex.end(); it1++) {
            //<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
            m_file.putInt1(FLAG_CG);
            m_file.putInt1((*it1).cg);
            m_file.putUIntLat((*it1).frequency);
            const list<struct Document> &documents = (*it1).documents;
            m_file.putUIntLat(documents.size());
            unsigned int identPrec = 0;             //pour mettre les identifiants de documents en relatif
            for (list<struct Document>::const_iterator it2 = documents.begin(); it2 != documents.end(); it2++) {
                //<identDocRelatif> <frequenceDoc>
                m_file.putUIntLat((*it2).ident - identPrec);
                m_file.putUIntLat((*it2).frequency);
                identPrec = (*it2).ident;
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
        throw NindTermIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
