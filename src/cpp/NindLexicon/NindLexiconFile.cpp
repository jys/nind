//
// C++ Implantation: NindLexiconFile
//
// Description: La gestion du lexique en fichier
// Étude et maquette d'un lexique complet ANT2012.JYS.R357 revA
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconFile.h"
//#include <iostream>
#include <string.h>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichierLexique>        ::= <lexique> <identification>
// <lexique>               ::= { <definitionMot> }
// <definitionMot>         ::= [ <definitionMotSimple> | <definitionMotCompose> ]
// <definitionMotSimple>   ::= <flagMotSimple> <identifiant> <motSimple>
// <definitionMotCompose>  ::= <flagMotCompose> <identifiant> <motCompose>
// <flagMotSimple>         ::= <Integer1>
// <flagMotCompose>        ::= <Integer1>
// <motSimple>             ::= <longueurMot> <motUtf8>
// <motCompose>            ::= <identifiant> <identifiant>
// <longueurMot>           ::= <Integer1>
// <motUtf8>               ::= { <Byte> }
// <identifiant>           ::= <Integer4>
// <identification>        ::= <flagIdentification> <identificationLexique>
// <flagIdentification>    ::= <Integer1>
// <identificationLexique> ::= <maxIdentifiant> <identifieurUnique>
// <maxIdentifiant>        ::= <Integer4>
// <identifieurUnique>     ::= <dateHeure>
// <dateHeure >            ::= <Integer4>
////////////////////////////////////////////////////////////
#define SIMPLE_WORD_FLAG 13
#define COMPOUND_WORD_FLAG 29
#define IDENTIFICATION_FLAG 53
#define BUFFER_SIZE 300
#define IDENT_SIZE 9
////////////////////////////////////////////////////////////
//brief Creates LexiconFile with a specified name associated with.
//param fileName absolute path file name
//param fromLexiconWriter true if from lexicon writer, false if from lexicon reader  */
NindLexiconFile::NindLexiconFile(const std::string &fileName,
                                 const bool fromLexiconWriter)
    throw(OpenFileException, WriteFileException, OutWriteBufferException):
    m_fromLexiconWriter(fromLexiconWriter),
    m_fileName(fileName),
    m_file(fileName, fromLexiconWriter)
{
    if (m_fromLexiconWriter) {
        //si lexique memoire ecrivain, cree un buffer intermediaire d'ecriture
        m_file.createBuffer(BUFFER_SIZE);
        //si lexique memoire ecrivain, ouvre en lecture + ecriture
        bool isOpened = m_file.open("r+b");
        //si fichier absent, cree un fichier vide en ecriture + lecture
        if (!isOpened) {
            isOpened = m_file.open("w+b");
            if (!isOpened) throw OpenFileException(m_fileName);
            //lui colle une identification bidon pour uniformiser les cas
            m_file.putChar(IDENTIFICATION_FLAG);
            m_file.putInt4(0);
            m_file.putInt4((time_t)time(NULL));  //date de creation du fichier
            m_file.writeBuffer();               //ecriture effective sur le fichier
        }
    }
    else {
        //si lexique memoire lecteur, ouvre en lecture seule
        bool isOpened = m_file.open("rb");
        if (!isOpened) throw OpenFileException(m_fileName);
    }
    m_file.setPos(0, SEEK_SET);  //pour pointer sur la tete
}
////////////////////////////////////////////////////////////
NindLexiconFile::~NindLexiconFile()
{
    m_file.close();
}
////////////////////////////////////////////////////////////
//brief Read next record of lexicon file as word definition.
//if next record is word definition, file pointer is advanced to the next record,
//else, file pointer is left unchanged
//param ident where ident will be returned
//param isSimpleWord where true will be returned if it is a simple word, false otherwise
//param simpleWord where simple word will be returned, if it is a simple word
//param compoundWord where compound word will be returned, if it is a compound word
//return true if next record is word definition, false otherwise */
bool NindLexiconFile::readNextRecordAsWordDefinition(unsigned int &ident,
                                                     bool &isSimpleWord,
                                                     string &simpleWord,
                                                     pair<unsigned int, unsigned int> &compoundWord)
    throw(EofException, ReadFileException, InvalidFileException)
{
    //on peut lire au fil de l'eau car l'ecrivain ne peut modifier qu'a partir de l'identification
    //lit le premier byte qui est le flag
    const unsigned char flag = m_file.readChar();
    if (flag == SIMPLE_WORD_FLAG) {
        ident = m_file.readInt4();
        isSimpleWord = true;
        simpleWord = m_file.readString();
        return true;
    }
    if (flag == COMPOUND_WORD_FLAG) {
        ident = m_file.readInt4();
        isSimpleWord = false;
        compoundWord.first = m_file.readInt4();
        compoundWord.second = m_file.readInt4();
        return true;
    }
    if (flag == IDENTIFICATION_FLAG) {
        //on est a la fin, il faut ramener le pointeur de fichier 1 octet en arriere
        m_file.setPos(-1, SEEK_CUR);
        return false;
    }
    throw InvalidFileException(m_fileName);
}
////////////////////////////////////////////////////////////
//brief Read next record of lexicon file as lexicon identification.
//file pointer is left unchanged
//param maxIdent where max ident will be returned
//param identification where unique identification will be returned
//return true if next record is lexicon identification, false otherwise */
bool NindLexiconFile::readNextRecordAsLexiconIdentification(unsigned int &maxIdent,
                                                            unsigned int &identification)
    throw(EofException, ReadFileException, InvalidFileException, OutReadBufferException)
{
    //lit le paquet d'identification en une seule fois pour garder la section critique
    m_file.readBuffer(IDENT_SIZE);
    //lit le premier byte qui est le flag
    const unsigned char flag = m_file.getChar();
    if (flag == IDENTIFICATION_FLAG) {
        maxIdent = m_file.getInt4();
        identification = m_file.getInt4();
        m_file.setPos(-IDENT_SIZE, SEEK_CUR);  //pour pointer sur l'identification
        return true;
    }
    if (flag == SIMPLE_WORD_FLAG || flag == COMPOUND_WORD_FLAG) {
        //on n'est pas a la fin, il faut ramener le pointeur de fichier 9 octets en arriere
        m_file.setPos(-IDENT_SIZE, SEEK_CUR);
        return false;
    }
    throw InvalidFileException(m_fileName);
}
////////////////////////////////////////////////////////////
//brief Write simple word definition on lexicon file.
//param ident word ident
//param simpleWord simple word
//param maxIdent max ident
//param identification unique identification  */
void NindLexiconFile::writeSimpleWordDefinition(const unsigned int ident,
                                                const string &simpleWord,
                                                const unsigned int maxIdent,
                                                const unsigned int identification)
    throw(WriteFileException, BadUseException, OutWriteBufferException)
{
    if (!m_fromLexiconWriter) throw BadUseException("lexicon file is not writable");
    m_file.putChar(SIMPLE_WORD_FLAG);
    m_file.putInt4(ident);
    m_file.putString(simpleWord);
    m_file.putChar(IDENTIFICATION_FLAG);
    m_file.putInt4(maxIdent);
    m_file.putInt4(identification);
    m_file.writeBuffer();
    m_file.setPos(-IDENT_SIZE, SEEK_CUR);  //pour pointer sur l'identification
}
////////////////////////////////////////////////////////////
//brief Write compound word definition on lexicon file.
//param ident word ident
//param compoundWord compound word
//param maxIdent max ident
//param identification unique identification  */
void NindLexiconFile::writeCompoundWordDefinition(const unsigned int ident,
                                                  const pair<unsigned int, unsigned int> compoundWord,
                                                  const unsigned int maxIdent,
                                                  const unsigned int identification)
    throw(WriteFileException, BadUseException, OutWriteBufferException)
{
    if (!m_fromLexiconWriter) throw BadUseException("lexicon file is not writable");
    m_file.putChar(COMPOUND_WORD_FLAG);
    m_file.putInt4(ident);
    m_file.putInt4(compoundWord.first);
    m_file.putInt4(compoundWord.second);
    m_file.putChar(IDENTIFICATION_FLAG);
    m_file.putInt4(maxIdent);
    m_file.putInt4(identification);
    m_file.writeBuffer();
    m_file.setPos(-IDENT_SIZE, SEEK_CUR);  //pour pointer sur l'identification
}
////////////////////////////////////////////////////////////
//brief Perform a clear buffer for reading the true file and not its buffer */
void NindLexiconFile::clearBuffer()
{
    m_file.flush();
}
////////////////////////////////////////////////////////////
