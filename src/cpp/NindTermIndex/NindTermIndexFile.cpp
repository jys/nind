//
// C++ Implementation: NindTermIndexFile
//
// Description: La gestion du fichier inverse en fichier
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindTermIndexFile.h"
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichierInverse>      ::= <inverse> <identification>
// <inverse>             ::= { <segment> }
// <segment>             ::= [ <definitionTerme> | <continuation> ]
// <definitionTerme>     ::= <flagDefinition> <identTerme>
//                            <longueurDonnees> <donnees>
//                            <offsetContinuation>
// <continuation>        ::= <flagContinuation> <identTerme>
//                            <longueurDonnees> <donnees>
//                            <offsetContinuation>
// <flagDefinition>      ::= <Integer1>
// <flagContinuation>    ::= <Integer1>
// <longueurDonnees>     ::= <Integer4>
// <donnees>             ::= { <Octet> }
// <offsetContinuation>  ::= <Integer8>
// 
// <identification>  :   ::= <flagIdentification> <maxIdentifiant>
//                            <identifieurUnique>
// <flagIdentification>    ::= <Integer1>
// <maxIdentifiant>      ::= <Integer4>
// <identifieurUnique>   ::= <dateHeure>
// <dateHeure >          ::= <Integer4>
////////////////////////////////////////////////////////////
#define DEFINITION_FLAG 17
#define CONTINUATION_FLAG 31
#define IDENTIFICATION_FLAG 53
#define IDENT_SIZE 9
#define LOCATION_RESIZE 1000
////////////////////////////////////////////////////////////
//brief Creates NindTermIndexFile with a specified name associated with.
//param fileName absolute path file name
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
NindTermIndexFile::NindTermIndexFile(const std::string &fileName,
                                     const bool isTermIndexWriter)
    throw(OpenFileException, EofException, ReadFileException, WriteFileException, InvalidFileException, OutOfBoundException):
    m_isTermIndexWriter(isTermIndexWriter),
    m_fileName(fileName),
    m_file(fileName, isTermIndexWriter),
    m_file_update(fileName, false),
    m_termLocations()
{
    if (m_isTermIndexWriter) {
        //si fichier inverse ecrivain, ouvre en lecture + ecriture
        bool isOpened = m_file.open("r+b");
        //si fichier absent, cree un fichier vide en ecriture + lecture
        if (!isOpened) {
            isOpened = m_file.open("w+b");
            if (!isOpened) throw OpenFileException(m_fileName);
            //lui colle une identification bidon pour uniformiser les cas
            const unsigned char flag = IDENTIFICATION_FLAG;
            m_file.writeBytes(&flag, 1);
            m_file.writeInt4(0);
            m_file.writeInt4((time_t)time(NULL));  //date de creation du fichier
            m_file.write();
        }
    }
    else {
        //si fichier inverse lecteur, ouvre en lecture seule
        bool isOpened = m_file.open("rb");
        if (!isOpened) throw OpenFileException(m_fileName);
    }
    //ouvre l'autre acces au fichier pour la mise a jour du vecteur d'acces aux termes
    bool isOpened = m_file_update.open("rb");
    if (!isOpened) throw OpenFileException(m_fileName);
    //lit le fichier et initialise le vecteur d'acces aux termes
    //d'abord, il faut trouver le nombre de termes, c'est dans l'identification
    m_file_update.setPos(-IDENT_SIZE, SEEK_END);   //pour pointer sur l'identification
    unsigned char bytes[IDENT_SIZE];
    while (true) {
        //lit le paquet d'identification en une seule fois pour garder la section critique
        m_file_update.readBytes(bytes, IDENT_SIZE);
        //lit le premier byte qui est le flag
        if (bytes[0] == DEFINITION_FLAG || bytes[0] == CONTINUATION_FLAG) {
            //pas de chance, l'ecrivain a augmente le fichier
            unsigned int segmentSize;
            m_file_update.readInt4(segmentSize, &bytes[5]);
            //le fichier pointe sur le premier octet des donnees
            //saute les donnees et l'offset de continuation (8 octets)
            m_file_update.setPos(segmentSize + 8, SEEK_CUR);   //pour pointer sur le segment suivant
            continue;
        }
        if (bytes[0] != IDENTIFICATION_FLAG) throw InvalidFileException(m_fileName);
        break;  //on a trouve l'identification
    }
    //le max des identifiants pour etablir la table des index
    unsigned int maxIdent;
    m_file_update.readInt4(maxIdent, &bytes[1]);
    const RecordLocation nullRecordLocation(0, 0);
    //le vecteur est alloue avec une marge pour supporter les futures augmentations du nbre de termes dues a l'ecrivain
    m_termLocations.resize(maxIdent + LOCATION_RESIZE, nullRecordLocation);
    //on lit le fichier depuis le debut pour peupler le vecteur des locations de termes
    m_file_update.setPos(0, SEEK_SET);
    updateFromFile();
}
////////////////////////////////////////////////////////////
NindTermIndexFile::~NindTermIndexFile()
{
    m_file.close();
    m_file_update.close();
}
////////////////////////////////////////////////////////////
//brief Read a full termIndex as a bytes string
//param ident ident of term
//param bytes buffer address where to write read bytes
//param bytesNb size of buffer where to write read bytes
//return true if term was found, false otherwise */
bool NindTermIndexFile::getTermIndex(const unsigned int ident,
                                     unsigned char *bytes,
                                     const unsigned int bytesNb)
    throw(EofException, ReadFileException, InvalidFileException, OutOfBoundException)
{
    //commence par mettre a jour au cas ou l'ecrivain aurait fait des ajouts
    updateFromFile();
    //trouve l'offset/longueur du 1er segment du terme
    if (ident >= m_termLocations.size()) throw OutOfBoundException("termId");
    const RecordLocation recordLocation = m_termLocations[ident];
    
}
////////////////////////////////////////////////////////////
//brief Read a full termIndex as a bytes string
//param ident ident of term
//param bytes address of bytes to write
//param bytesNb number of bytes to write
//return true if success, false otherwise */
bool NindTermIndexFile::setTermIndex(const unsigned int ident,
                                     const unsigned char *bytes,
                                     const unsigned int bytesNb)
    throw(WriteFileException)
{
}
////////////////////////////////////////////////////////////
//brief set identification
//param wordsNb number of words contained in lexicon
//param identification unique identification of lexicon */
void NindTermIndexFile::setIdentification(const unsigned int wordsNb,
                                          const unsigned int identification)
    throw(WriteFileException)
{
}
////////////////////////////////////////////////////////////
//brief Perform a clear buffer for reading the true file and not its buffer */
void NindTermIndexFile::clearBuffer()
{
}
////////////////////////////////////////////////////////////
//met a jour le vecteur des locations de termes avec le fichier inverse
void NindTermIndexFile::updateFromFile()
    throw(EofException, ReadFileException, InvalidFileException)
{
    const unsigned int maxTerms = m_termLocations.size();
    //on ne prend que les segments de definition de terme (on ignore les continuations)
    while (true) {
        //pas besoin de "faire" des sections critiques, la structure ne bouge pas du fait de l'ecrivain
        //lit le premier byte qui est le flag
        unsigned char flag;
        m_file_update.readBytes(&flag, 1);
        if (flag == IDENTIFICATION_FLAG) {
            //on ramene le pointeur sur l'identification
            m_file_update.setPos(-1, SEEK_CUR);
            break;  //termine
        }
        if (flag != IDENTIFICATION_FLAG && flag != CONTINUATION_FLAG) throw InvalidFileException(m_fileName);
        //c'est un segment de definition ou de continuation
        //lit l'ident du terme et la longueur des données
        unsigned int termIdent;
        m_file_update.readInt4(termIdent);
        unsigned int dataSize;
        m_file_update.readInt4(dataSize);
        //si c'est une definition, la colle dans le vecteur
        if (flag == IDENTIFICATION_FLAG) {
            const unsigned int maxTerms = m_termLocations.size();
            if (maxTerms <= termIdent) {
                //le vecteur est trop petit, on realloue pour 1000 termes supplementaires
                const RecordLocation nullRecordLocation(0, 0);
                m_termLocations.resize(maxTerms + LOCATION_RESIZE, nullRecordLocation);
            }
            //les termIds sont incrementes et, normalement, il ne peut y avoir un trou de 1000
            //c'est donc le test d'integrite des termId
            if (m_termLocations.size() <= termIdent) throw OutOfBoundException("termId");
            //la position de l'enregistrement dans le fichier
            const long int filePos = m_file_update.getPos() - 9; //- flag, ident, size
            //la taille du segment comprend les donnees + flag + ident + taille donnees + offset continuation = +17
            m_termLocations[termIdent] = RecordLocation(filePos, dataSize + 17); 
        }
        //saute les donnees et l'offset de continuation (8 octets)
        m_file_update.setPos(dataSize + 8, SEEK_CUR);   //pour pointer sur le segment suivant
    }
    //le pointeur est sur l'identification
}
////////////////////////////////////////////////////////////
