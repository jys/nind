//
// C++ Implantation: NindPadFile
//
// Description: La gestion basique d'un fichier nind : correspondance ident -> donnejes et identification
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
// Cette classe est dérivable.
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
#include "NindPadFile.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichier>               ::= <tailleEntreje> <tailleSpejcifiques> { <blocIndexej> <blocEnVrac> } 
//                             <blocSpejcifique> <blocIdentification> 
//
// <tailleEntreje>         ::= <Integer1>
// <tailleSpejcifiques>    ::= <Integer3>
//
// <blocIndexej>           ::= <flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
// <flagIndexej=47>        ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreIndex>           ::= <Integer3>
//
// <donnejesIndexejes>     ::= { <Octet> }
// <blocEnVrac>            ::= { <Octet> }
//
// <blocSpejcifique>       ::= <flagSpecifique=57> <spejcifiques> 
// <flagSpecifique=57>     ::= <Integer1>
// <spejcifique>           ::= { <Octet> }
//
// <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> 
// <flagIdentification=53> ::= <Integer1>
// <maxIdentifiant>        ::= <Integer4>
// <identifieurUnique>     ::= <dateHeure>
// <dateHeure >            ::= <Integer4>
////////////////////////////////////////////////////////////
#define FLAG_INDEXEJ 47
#define FLAG_SPEJCIFIQUE 57
#define FLAG_IDENTIFICATION 53
//<tailleIndex>(1) <tailleSpejcifiques>(3) = 4
#define TAILLE_ENTETE_FIXE 4
//<flagSpecifique=57>(1)
#define TAILLE_ENTETE_SPEJCIFIQUE 1
//<flagIdentification=53>(1) <maxIdentifiant>(4) <identifieurUnique>(4) = 9
#define TAILLE_IDENTIFICATION 9
////////////////////////////////////////////////////////////
//brief Creates NindPadFile with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if writer, false if reader  
//param referenceIdentification unique identification for checking (if 0, no checks)
//param specificsSize size in bytes of specific datas
//param dataEntrySize size in bytes of one data entry (for neo writer)
//param dataEntriesBlocSize number of entries in a single block (for neo writer) */
NindPadFile::NindPadFile(const string &fileName,
                         const bool isWriter,
                         const Identification &referenceIdentification,
                         const unsigned int specificsSize,
                         const unsigned int dataEntrySize,
                         const unsigned int dataEntriesBlocSize):
    m_file(fileName),
    m_fileName(fileName),
    m_isWriter(isWriter),
    m_isExistingWriter(false),
    m_specificsSize(specificsSize),
    m_dataEntrySize(dataEntrySize),
    m_dataEntriesBlocSize(dataEntriesBlocSize),
    m_entriesBlocksMap()
{
    if (m_isWriter) {
        //si fichier ejcrivain, ouvre en ecriture + lecture
        bool isOpened = m_file.open("r+b");
        if (isOpened) {
            //si le fichier existe, l'analyse pour trouver le mapping des blocs et l'identification
            //lit la taille d'une donneje indexeje at la taille des spejcifiques
            m_file.setPos(0, SEEK_SET);
            m_file.readBuffer(TAILLE_ENTETE_FIXE);
            m_dataEntrySize = m_file.getInt1();
            if (m_dataEntrySize != dataEntrySize) 
                throw NindPadFileException("NindPadFile::NindPadFile bad index size " + m_fileName);
            m_specificsSize = m_file.getInt3();
            if (m_specificsSize != specificsSize) 
                throw NindPadFileException("NindPadFile::NindPadFile bad specifics size " + m_fileName);
            //ejtablit la carte des blocs d'entrejes       
            mapEntriesBlocks();
            //vejrifie la structure des spejcifiques
            checkSpejcifiques();
            //vejrifie l'apairage avec le lexique
            checkIdentification(referenceIdentification); 
            m_isExistingWriter = true;
        }
        else {
            //si le fichier n'existe pas, le creje vide en ejcriture + lecture
            //la taille d'une entreje doit estre spejcifieje diffejrente de 0
            if (dataEntrySize == 0) 
                throw NindPadFileException("NindPadFile::NindPadFile null dataEntrySize " + m_fileName);
            //la taille du bloc de dejfinitions doit estre spejcifieje diffejrente de 0
            if (dataEntriesBlocSize == 0) 
                throw NindPadFileException("NindPadFile::NindPadFile null dataEntriesBlocSize " + m_fileName);
            isOpened = m_file.open("w+b");
            if (!isOpened) throw NindPadFileException("NindPadFile::NindPadFile " + m_fileName);
            //taille d'une donneje indexeje et taille des spejcifiques en teste de fichier
            m_file.createBuffer(TAILLE_ENTETE_FIXE);
            m_file.putInt1(m_dataEntrySize);
            m_file.putInt3(m_specificsSize);
            m_file.writeBuffer();                               //ecriture effective sur le fichier   
            //augmente le fichier de sa queue
            m_file.writeValue(0, m_specificsSize + TAILLE_ENTETE_SPEJCIFIQUE + TAILLE_IDENTIFICATION);
            //ajoute un bloc d'entrejes suivi des spejcifiques suivi de l'identification (le tout ah 0)
            addEntriesBlock(referenceIdentification);
        }
    }
    else {
        //si fichier lecteur, ouvre en lecture seule
        bool isOpened = m_file.open("rb");
        if (!isOpened) throw NindPadFileException("NindPadFile::NindPadFile open error " + m_fileName);
        //lit la taille d'une donneje indexeje et la taille des spejcifiques
        m_file.setPos(0, SEEK_SET);
        m_file.readBuffer(TAILLE_ENTETE_FIXE);
        m_dataEntrySize = m_file.getInt1();
        m_specificsSize = m_file.getInt3();
        //ejtablit la carte des blocs d'entrejes        
        mapEntriesBlocks();
        //vejrifie la structure des spejcifiques
        checkSpejcifiques();
        //vejrifie l'apairage avec le lexique
        checkIdentification(referenceIdentification); 
    }
}
////////////////////////////////////////////////////////////
NindPadFile::~NindPadFile()
{
}
////////////////////////////////////////////////////////////
//brief get position of specified entry
//param ident ident of specified entry
//return offset into file of specified entry (0 if out of bounds)*/
unsigned long int NindPadFile::getEntryPos(const unsigned int ident)
{
    //trouve l'entreje
    unsigned long int position = getJustEntryPos(ident);
    if (m_isWriter) return position; //le processus ejcrivain n'a pas besoin de synchronisation
    //synchronisation des lecteurs
    if (position == 0) {
        //le processus lecteur met ainsi ah jour sa table d'indirection
        //ejtablit la carte des indirections  
        mapEntriesBlocks();
        position = getJustEntryPos(ident);
    }
    return position;
}
////////////////////////////////////////////////////////////
//brief add empty entries block 
//param fileIdentification unique identification of file */
void NindPadFile::addEntriesBlock(const Identification &fileIdentification)
{
    //se positionne sur les spejcifiques
    const int offset = TAILLE_ENTETE_SPEJCIFIQUE + TAILLE_IDENTIFICATION + m_specificsSize;
    m_file.setPos(-offset, SEEK_END); 
    const unsigned long int entriesBlock = m_file.getPos();
    m_file.createBuffer(TAILLE_TETE_INDEX);
    //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
    m_file.putInt1(FLAG_INDEXEJ);
    m_file.putInt5(0);
    m_file.putInt3(m_dataEntriesBlocSize);
    m_file.writeBuffer();                               //ecriture effective sur le fichier   
    //remplit la zone d'indirection augenteje des spejcifiques avec des 0
    m_file.writeValue(0, m_dataEntriesBlocSize*m_dataEntrySize);
    m_file.writeValue(FLAG_SPEJCIFIQUE, 1);
    m_file.writeValue(0, m_specificsSize);
    //lui colle l'identification du lexique a suivre
    m_file.createBuffer(TAILLE_IDENTIFICATION);
    writeIdentification(fileIdentification);
    m_file.writeBuffer();                               //ecriture effective sur le fichier
    //si ce n'est pas le premier bloc, il faut chaisner dans le fichier
    if (m_entriesBlocksMap.size() != 0) {
        pair<unsigned long int, unsigned int> &indirectionBlocPrec = m_entriesBlocksMap.back();
        //se positionne sur le <addrBlocSuivant> du dernier bloc
        //<flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
        m_file.setPos(indirectionBlocPrec.first -8, SEEK_SET);   //pour pointer <addrBlocSuivant>
        m_file.createBuffer(5);
        m_file.putInt5(entriesBlock);
        m_file.writeBuffer();
    }
    //met ah jour la carte des entrejes
    const pair<unsigned long int, unsigned int> entrejes(entriesBlock + TAILLE_TETE_INDEX, m_dataEntriesBlocSize);
    m_entriesBlocksMap.push_back(entrejes);
}
////////////////////////////////////////////////////////////
//brief get size of 1rst entries block
//return size of 1rst entries block */
unsigned int NindPadFile::getFirstEntriesBlockSize()
{
    m_file.setPos(TAILLE_ENTETE_FIXE, SEEK_SET);  //positionne aprehs l'en-teste fixe
    //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
    m_file.readBuffer(TAILLE_TETE_INDEX);
    if (m_file.getInt1() != FLAG_INDEXEJ) 
        throw NindPadFileException("NindPadFile::getFirstEntriesBlockSize : " + m_fileName);
    m_file.getInt5();
    return m_file.getInt3();
}
////////////////////////////////////////////////////////////
//return l'identifiant maximum possible avec le systehme actuel d'indirection
unsigned int NindPadFile::getMaxIdent() const
{
    unsigned int maxIdent = 0;
    for (list<pair<unsigned long int, unsigned int> >::const_iterator it = m_entriesBlocksMap.begin(); 
        it != m_entriesBlocksMap.end(); it++) 
        maxIdent += (*it).second;
    return maxIdent;
}
////////////////////////////////////////////////////////////
//brief Read from file specific datas and leave result into read buffer */
void NindPadFile::getSpecifics()
{
    m_file.flush();
    //se positionne sur la teste des spejcifiques
    const int offset = TAILLE_ENTETE_SPEJCIFIQUE + TAILLE_IDENTIFICATION + m_specificsSize;
    m_file.setPos(-offset, SEEK_END); 
    //Lit les spejcifiques 
    m_file.readBuffer(m_specificsSize + TAILLE_ENTETE_SPEJCIFIQUE);
    if (m_file.getInt1() != FLAG_SPEJCIFIQUE) 
        throw NindPadFileException("NindPadFile::getSpecifics : " + m_fileName); 
}
////////////////////////////////////////////////////////////
//brief get identification of file
//param identification where unique identification of file is returned */
void NindPadFile::getFileIdentification(Identification &identification)
{
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw NindPadFileException("NindPadFile::getFileIdentification : " + m_fileName);
    const unsigned int wordsNb = m_file.getInt4();
    const unsigned int time = m_file.getInt4();
    identification = Identification(wordsNb, time);
}
////////////////////////////////////////////////////////////
//brief get size of specific datas + identification */
unsigned int NindPadFile::getSpecificsAndIdentificationSize() const
{
    return m_specificsSize + TAILLE_ENTETE_SPEJCIFIQUE + TAILLE_IDENTIFICATION;
}   
////////////////////////////////////////////////////////////
//brief get file name
//return file name of lexicon */
string NindPadFile::getFileName()
{
    return m_fileName;
}
////////////////////////////////////////////////////////////
//brief write specifics header into write buffer */
void NindPadFile::writeSpecificsHeader()
{
    //<flagSpecifique=57>
    m_file.putInt1(FLAG_SPEJCIFIQUE);
}   
////////////////////////////////////////////////////////////
//brief write identification into write buffer
//param fileIdentification unique identification of file */
void NindPadFile::writeIdentification(const Identification &fileIdentification)
{
    //le pointeur d'ejcriture est supposej au bon endroit dans le buffer
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> 
    m_file.putInt1(FLAG_IDENTIFICATION);
    m_file.putInt4(fileIdentification.lexiconWordsNb);
    m_file.putInt4(fileIdentification.lexiconTime);
}   
////////////////////////////////////////////////////////////
//retourne la position d'une entreje 
unsigned long int NindPadFile::getJustEntryPos(const unsigned int ident)
{
    //trouve l'entreje
    unsigned int firstIdent = 0;
    list<pair<unsigned long int, unsigned int> >::const_iterator it = m_entriesBlocksMap.begin(); 
    while (it != m_entriesBlocksMap.end()) {
        if (ident < firstIdent + (*it).second) return (ident - firstIdent) * m_dataEntrySize + (*it).first;
        firstIdent += (*it).second;
        it++;
    }
    //si la dejfinition chercheje n'existe pas, retourne 0
    return 0;
}
////////////////////////////////////////////////////////////
//ejtablit la carte des blocs d'entrejes
void NindPadFile::mapEntriesBlocks()
{
    m_entriesBlocksMap.clear();
    m_file.setPos(TAILLE_ENTETE_FIXE, SEEK_SET);  //positionne derriehre l'enteste fixe
    while (true) {
        //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
        m_file.readBuffer(TAILLE_TETE_INDEX);
        if (m_file.getInt1() != FLAG_INDEXEJ) 
            throw NindPadFileException("NindPadFile::mapEntriesBlocks : " + m_fileName);
        const unsigned long int addrBlocSuivant = m_file.getInt5();
        const unsigned int nombreIndex = m_file.getInt3();
        const unsigned long pos = m_file.getPos(); 
        const pair<unsigned int, unsigned long int> entrejes(pos, nombreIndex);
        m_entriesBlocksMap.push_back(entrejes);
        if (addrBlocSuivant == 0) break;        //si pas d'extension, termine
        //saute au bloc d'indirection suivant
        m_file.setPos(addrBlocSuivant, SEEK_SET);    //pour aller au suivant
    }
}
////////////////////////////////////////////////////////////
//vejrifie la structure des spejcifiques
void NindPadFile::checkSpejcifiques()
{
    //se positionne sur la teste des spejcifiques
    const int offset = TAILLE_IDENTIFICATION + TAILLE_ENTETE_SPEJCIFIQUE + m_specificsSize;
    m_file.setPos(-offset, SEEK_END); 
    //<flagSpecifique=57>
    if (m_file.readInt1() != FLAG_SPEJCIFIQUE)
        throw NindPadFileException("NindPadFile::checkSpejcifiques : " + m_fileName); 
}
////////////////////////////////////////////////////////////
//vejrifie l'apairage avec la rejfejrence
void NindPadFile::checkIdentification(const Identification &referenceIdentification)
{
    //se positionne sur l'identification
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> 
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw NindPadFileException("NindPadFile::checkIdentification : " + m_fileName);
    const unsigned int maxIdent = m_file.getInt4();
    const unsigned int identification = m_file.getInt4();
    //si la rejfejrence est nulle, pas de comparaison de valeurs
    if (referenceIdentification == Identification(0, 0)) return;
    //si la rejfejrence n'est pas nulle, comparaison 
    if (Identification(maxIdent, identification) != referenceIdentification) {
        throw NindPadFileException("NindPadFile::checkIdentification Incompatible file: " + m_fileName); 
    }
}
////////////////////////////////////////////////////////////
