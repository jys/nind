//
// C++ Implantation: NindLexiconFile
//
// Description: La gestion du lexique en fichier
// Étude et maquette d'un lexique complet ANT2012.JYS.R357 revA
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconFile.h"
//#include <iostream>
#include <string.h>
using namespace antinno::nindex;
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
    throw(OpenFileException, WriteFileException):
    m_fromLexiconWriter(fromLexiconWriter),
    m_fileName(fileName),
    m_file(fileName, fromLexiconWriter, BUFFER_SIZE)
{
    if (m_fromLexiconWriter) {
        //si lexique memoire ecrivain, ouvre en lecture + ecriture
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
    unsigned char flag;
    m_file.readBytes(&flag, 1);
    if (flag == SIMPLE_WORD_FLAG) {
        m_file.readInt4(ident);
        isSimpleWord = true;
        m_file.readString(simpleWord);
        return true;
    }
    if (flag == COMPOUND_WORD_FLAG) {
        m_file.readInt4(ident);
        isSimpleWord = false;
        m_file.readInt4(compoundWord.first);
        m_file.readInt4(compoundWord.second);
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
    throw(EofException, ReadFileException, InvalidFileException)
{
    //lit le paquet d'identification en une seule fois pour garder la section critique
    unsigned char bytes[IDENT_SIZE];
    m_file.readBytes(bytes, IDENT_SIZE);
    //lit le premier byte qui est le flag
    unsigned char flag = bytes[0];
    if (flag == IDENTIFICATION_FLAG) {
        m_file.readInt4(maxIdent, &bytes[1]);
        m_file.readInt4(identification, &bytes[5]);
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
    throw(WriteFileException, BadUseException)
{
    if (!m_fromLexiconWriter) throw BadUseException("lexicon file is not writable");
    unsigned char flag = SIMPLE_WORD_FLAG;
    m_file.writeBytes(&flag, 1);
    m_file.writeInt4(ident);
    m_file.writeString(simpleWord);
    flag = IDENTIFICATION_FLAG;
    m_file.writeBytes(&flag, 1);
    m_file.writeInt4(maxIdent);
    m_file.writeInt4(identification);
    m_file.write();
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
    throw(WriteFileException, BadUseException)
{
    if (!m_fromLexiconWriter) throw BadUseException("lexicon file is not writable");
    unsigned char flag = COMPOUND_WORD_FLAG;
    m_file.writeBytes(&flag, 1);
    m_file.writeInt4(ident);
    m_file.writeInt4(compoundWord.first);
    m_file.writeInt4(compoundWord.second);
    flag = IDENTIFICATION_FLAG;
    m_file.writeBytes(&flag, 1);
    m_file.writeInt4(maxIdent);
    m_file.writeInt4(identification);
    m_file.write();
    m_file.setPos(-IDENT_SIZE, SEEK_CUR);  //pour pointer sur l'identification
}
////////////////////////////////////////////////////////////
//brief Perform a clear buffer for reading the true file and not its buffer */
void NindLexiconFile::clearBuffer()
{
    m_file.flush();
}
////////////////////////////////////////////////////////////
