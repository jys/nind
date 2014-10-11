//
// C++ Implementation: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358
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
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichierInverse>      ::= { <indirection> <inverse> } <identification> 
//
// <indirection>         ::= <flagIndirection> <indirectionSuivante> <nombreIndirection> { entree }
// <flagIndirection>     ::= <Integer1>
// <indirectionSuivante> ::= <Integer8>
// <nombreIndirection>   ::= <Integer4>
// <entree>              ::= <offsetEntree> <longueurEntree> 
// <offsetEntree>        ::= <Integer8>
// <longueurEntree>      ::= <Integer4>
//
// <inverse>             ::= { <definitionTerme> | <vide> }
// <definitionTerme>     ::= <flagDefinition> <identTerme> <longueurDonnees> <donnees>
// <flagDefinition>      ::= <Integer1>
// <vide>                ::= <flagVide> <longueurDonnees> <donnees>
// <flagVide>            ::= <Integer1>
// <longueurDonnees>     ::= <Integer4>
// <donnees>             ::= { <Octet> }
// <donnees>             ::= { <donneesTermeCG> }
// <donneesTermeCG>      ::= <flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
// <flagCg>              ::= <Integer1>
// <categorie>           ::= <Integer4>
// <frequenceTerme>      ::= <Integer4>
// <nbreDocs>            ::= <Integer4>
// <listeDocuments>      ::= { <identDocument> <frequenceDoc> }
// <identDocument>       ::= <Integer4>
// <frequenceDoc>        ::= <Integer4>
//
// <identification>  :   ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
// <flagIdentification>  ::= <Integer1>
// <maxIdentifiant>      ::= <Integer4>
// <identifieurUnique>   ::= <dateHeure>
// <dateHeure >          ::= <Integer4>
//
////////////////////////////////////////////////////////////
#define INDIRECTION_FLAG 47
#define DEFINITION_FLAG 17
#define IDENTIFICATION_FLAG 53
#define EMPTY_FLAG 57
#define CG_FLAG 61
#define IDENT_SIZE 9
//#define BUFFER_SIZE 1024
#define EMPTY_INDEX_SIZE 1024*10
#define INDIRECTION_HEAD 12
#define ENTREE_SIZE 12
////////////////////////////////////////////////////////////
//brief Creates NindTermIndex with a specified name associated with.
//param fileName absolute path file name
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon */
NindTermIndex::NindTermIndex(const std::string &fileName,
                             const bool isTermIndexWriter,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification)
    throw(OpenFileException, EofException, ReadFileException, WriteFileException, IncompatibleFileException, InvalidFileException, OutOfBoundException):
    m_isTermIndexWriter(isTermIndexWriter),
    m_fileName(fileName),
    m_file(fileName, isTermIndexWriter),
    m_indirectionMapping(),
    m_emptyAreas()
{
    if (m_isTermIndexWriter) {
        //si fichier inverse ecrivain, ouvre en ecriture + lecture
        bool isOpened = m_file.open("r+b");
        if (isOpened) {
            //si le fichier existe, l'analyse pour trouver les differents blocs et l'identification
            //etablit la carte des indirections et des trous
            long int indirectionSuivante = 0;        //memorise pour verification
            m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
            while (true) {
                const unsigned char flag = m_file.readChar();     //lit le flag
                if (flag == INDIRECTION_FLAG) {
                    const long pos = m_file.getPos();           //la position a l'interieur du fichier
                    //verifie que le bloc d'indirection etait bien prevu a cet endroit
                    if (pos != indirectionSuivante + 1) throw InvalidFileException(m_fileName); 
                    indirectionSuivante = m_file.readInt8();
                    const unsigned int nombreIndirection = m_file.readInt4();
                    const pair<long, unsigned int> indirection(pos + INDIRECTION_HEAD, nombreIndirection);
                    m_indirectionMapping.push_back(indirection);
                    //saute le bloc d'indirection
                    m_file.setPos(nombreIndirection * ENTREE_SIZE, SEEK_CUR);    //pour aller au suivant
                    continue;
                }
                if (flag == DEFINITION_FLAG) {
                    m_file.readInt4();        //on s'en fout
                    const unsigned int longueurDonnees = m_file.readInt4();        
                    m_file.setPos(longueurDonnees, SEEK_CUR);  //pour aller au suivant
                    continue;
                }
                if (flag == EMPTY_FLAG) {
                    const long pos = m_file.getPos();
                    const unsigned int longueurDonnees = m_file.readInt4();   
                    const pair<long, unsigned int> emptyArea(pos, longueurDonnees);
                    m_emptyAreas.push_back(emptyArea);
                    m_file.setPos(longueurDonnees, SEEK_CUR);  //pour aller au suivant
                    continue;
                }
                if (flag == IDENTIFICATION_FLAG) {
                    const unsigned int maxIdent = m_file.readInt4();
                    const unsigned int identification = m_file.readInt4();
                    if (maxIdent != lexiconWordsNb || identification != lexiconIdentification) 
                        throw IncompatibleFileException(m_fileName); 
                    break;
                }
                throw InvalidFileException(m_fileName); 
            }
        }
        else {
            //si le fichier n'existe pas, le cree vide en ecriture + lecture
            isOpened = m_file.open("w+b");
            if (!isOpened) throw OpenFileException(m_fileName);
            //lui colle un index vide
            m_file.createBuffer(17);
            m_file.putChar(INDIRECTION_FLAG);                // <flagIndirection>
            m_file.putInt8(0);                        // <indirectionSuivante>
            m_file.putInt8(EMPTY_INDEX_SIZE);         // <longueurIndirection>
            m_file.writeBuffer();                             //ecriture effective sur le fichier   
            //remplit la zone d'indirection avec des 0
            m_file.writeValue(0, EMPTY_INDEX_SIZE*ENTREE_SIZE);
            //lui colle l'identification du lexique a suivre
            //utilise le meme buffer
            m_file.putChar(IDENTIFICATION_FLAG);                // <flagIdentification>
            m_file.putInt4(lexiconWordsNb);           // <maxIdentifiant>
            m_file.putInt4(lexiconIdentification);    // <identifieurUnique>
            m_file.writeBuffer();                             //ecriture effective sur le fichier
        }
    }
    else {
        //si fichier inverse lecteur, ouvre en lecture seule
        bool isOpened = m_file.open("rb");
        if (!isOpened) throw OpenFileException(m_fileName);
        //etablit la carte des indirections  
        m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
        while (true) {
            const unsigned char flag = m_file.readChar();     //lit le flag
            if (flag != INDIRECTION_FLAG) throw InvalidFileException(m_fileName);
            const long pos = m_file.getPos();           //la position a l'interieur du fichier
            const long int indirectionSuivante = m_file.readInt8();
            const unsigned int nombreIndirection = m_file.readInt4();
            const pair<long, unsigned int> indirection(pos + INDIRECTION_HEAD, nombreIndirection);
            m_indirectionMapping.push_back(indirection);
            if (indirectionSuivante == 0) break;        //si pas d'extension, termine
            //saute au bloc d'indirection suivant
            m_file.setPos(indirectionSuivante, SEEK_SET);    //pour aller au suivant
        }
        //verifie l'apairage avec le lexique
        const unsigned char flag = m_file.readChar();     //lit le flag
        if (flag != IDENTIFICATION_FLAG) throw InvalidFileException(m_fileName);
        const unsigned int maxIdent = m_file.readInt4();
        const unsigned int identification = m_file.readInt4();
        if (maxIdent != lexiconWordsNb || identification != lexiconIdentification) 
            throw IncompatibleFileException(m_fileName); 
    }
}
////////////////////////////////////////////////////////////
NindTermIndex::~NindTermIndex()
{
    m_file.close();
}
////////////////////////////////////////////////////////////
//brief Read a full termIndex as a list of structures
//param ident ident of term
//param termIndex structure to receive all datas of the specified term
//return true if term was found, false otherwise */
bool NindTermIndex::getTermIndex(const unsigned int ident,
                                 list<struct TermCG> &termIndex)
    throw(EofException, ReadFileException, InvalidFileException, OutOfBoundException)
{
    long int offsetEntree;
    unsigned int longueurEntree;
    getIndirection(ident, offsetEntree, longueurEntree);
    //si le terme n'a pas encore ete indexe, retourne false
    if (offsetEntree == 0) return false;
    //et lit tout ce qui concerne le terme
    m_file.setPos(offsetEntree, SEEK_SET);    //se positionne sur le terme
    m_file.readBuffer(longueurEntree);
    //<flagDefinition> <identTerme> <longueurDonnees> <donnees>
    if (m_file.getChar() != DEFINITION_FLAG) throw InvalidFileException("GT1 : " + m_fileName);
    const unsigned int identTerme = m_file.getInt4();
    if (identTerme != ident) throw InvalidFileException("GT2 : " + m_fileName);
    const unsigned int longueurDonnees = m_file.getInt4();
    if (longueurEntree != longueurDonnees + 9) throw InvalidFileException("GT3 : " + m_fileName);
    //<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
    while (!m_file.endOfBuffer()) {
        if (m_file.getChar() != CG_FLAG) throw InvalidFileException("GT4 : " + m_fileName);
        const unsigned int categorie = m_file.getInt4();
        const unsigned int frequenceTerme = m_file.getInt4();
        const unsigned int nbreDocs = m_file.getInt4();
        termIndex.push_back(TermCG(categorie, frequenceTerme));
        struct TermCG &termCG = termIndex.back();
        list<Document> &documents = termCG.documents;
        for (unsigned int it = 0; it != nbreDocs; it++) {
            //<identDocument> <frequenceDoc>
            const unsigned int identDocument = m_file.getInt4();
            const unsigned int frequenceDoc = m_file.getInt4();
            documents.push_back(Document(identDocument, frequenceDoc));
        }
    }
    return true;
}
////////////////////////////////////////////////////////////
//brief Write a full termIndex as a bytes string
//param ident ident of term
//param termIndex structure containing all datas of the specified term */
void NindTermIndex::setTermIndex(const unsigned int ident,
                                 const list<struct TermCG> &termIndex)
    throw(WriteFileException)
{
    //1) calcule la taille du buffer d'ecriture
    //<flagDefinition> <identTerme> <longueurDonnees> <donnees>
    unsigned int longueurEntree = 9;
    for (list<struct TermCG>::const_iterator it1 = termIndex.begin(); it1 != termIndex.end(); it1++) {
        //<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
        longueurEntree += 13 + (*it1).documents.size()*8;        
    }
    //2) forme le buffer a ecrire sur le fichier
    unsigned char *buffer = new unsigned char(longueurEntree);
    
    
}
////////////////////////////////////////////////////////////
//brief set identification
//param wordsNb number of words contained in lexicon
//param identification unique identification of lexicon */
void NindTermIndex::setIdentification(const unsigned int wordsNb,
                                      const unsigned int identification)
    throw(WriteFileException)
{
}
////////////////////////////////////////////////////////////
//recupere l'indirection du terme specifie
void NindTermIndex::getIndirection(const unsigned int ident,
                                   long int &offsetEntree,
                                   unsigned int &longueurEntree)
    throw(EofException, ReadFileException, OutOfBoundException)
{
    //trouve l'indirection du terme
    unsigned int firstIdent = 0;
    long int firstIndirection = 0;
    for (list<pair<long int, unsigned int> >::const_iterator it = m_indirectionMapping.begin(); 
         it != m_indirectionMapping.end(); it++) {
        if (ident < firstIdent + (*it).second) {
            firstIndirection = (*it).first;
            break;
        }
        firstIdent += (*it).second;
    }
    //erreur si le terme cherche n'a pas d'indirection
    if (firstIndirection == 0) throw OutOfBoundException(m_fileName);
    //lit l'indirection du terme
    const long int indirection = (ident - firstIdent) * ENTREE_SIZE + firstIndirection;
    //force la relecture du buffer
    m_file.flush();
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection du terme
    m_file.readBuffer(ENTREE_SIZE);
    offsetEntree = m_file.getInt8();
    longueurEntree = m_file.getInt4();
}
////////////////////////////////////////////////////////////
