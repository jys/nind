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
// <fichier>               ::= { <blocIndexej> <blocEnVrac> } <blocIdentification> 
//
// <blocIndexej>           ::= <flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
// <flagIndexej=47>        ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreIndex>           ::= <Integer3>
//
// <donnejesIndexejes>     ::= { <Octet> }
// <blocEnVrac>            ::= { <Octet> }
//
// <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
// <flagIdentification=53> ::= <Integer1>
// <maxIdentifiant>        ::= <Integer3>
// <identifieurUnique>     ::= <dateHeure>
// <identifieurSpecifique> ::= <Integer4>
// <dateHeure >            ::= <Integer4>
////////////////////////////////////////////////////////////
#define FLAG_INDEXEJ 47
////////////////////////////////////////////////////////////
//brief Creates NindPadFile with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if writer, false if reader  
//param referenceIdentification unique identification for checking (if 0, no checks)
//param dataEntrySize size in bytes of one data entry (for neo writer)
//param dataEntriesBlocSize number of entries in a single block (for neo writer) */
NindPadFile::NindPadFile(const string &fileName,
                         const bool isWriter,
                         const Identification &referenceIdentification,
                         const unsigned int dataEntrySize,
                         const unsigned int dataEntriesBlocSize):
    m_file(fileName),
    m_fileName(fileName),
    m_isWriter(isWriter),
    m_dataEntrySize(dataEntrySize),
    m_dataEntriesBlocSize(dataEntriesBlocSize),
    m_entriesBlocksMap()
{
    try {
        if (m_isWriter) {
            //si fichier ejcrivain, ouvre en ecriture + lecture
            bool isOpened = m_file.open("r+b");
            if (isOpened) {
                //si le fichier existe, l'analyse pour trouver le mapping des blocs et l'identification
                //ejtablit la carte des blocs d'entrejes       
                mapEntriesBlocks();
                //verifie l'apairage avec le lexique
                checkIdentification(referenceIdentification); 
            }
            else {
                //si le fichier n'existe pas, le creje vide en ejcriture + lecture
                //la taille du bloc de dejfinitions doit estre spejcifieje diffejrente de 0
                if (dataEntriesBlocSize == 0) 
                {
                    string errorString = string("NindPadFile. In write mode, dataEntriesBlocSize "
                        "must be non null. ") + m_fileName;
                    cerr<<errorString<<endl;
                    throw BadUseException(errorString);
                }
                isOpened = m_file.open("w+b");
                if (!isOpened) throw OpenFileException(m_fileName);
                //ajoute un bloc d'entrejes suivi de l'identification
                addEntriesBlock(referenceIdentification);
            }
        }
        else {
            //si fichier lecteur, ouvre en lecture seule
            bool isOpened = m_file.open("rb");
            if (!isOpened) throw OpenFileException(m_fileName);
            //ejtablit la carte des blocs d'entrejes        
            mapEntriesBlocks();
            //verifie l'apairage avec le lexique
            checkIdentification(referenceIdentification); 
        }
    }
    catch (FileException &exc) {
        throw NindIndexException(m_fileName);
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
    //se positionne sur l'identification
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       
    const unsigned long int entriesBlock = m_file.getPos();
    m_file.createBuffer(TETE_BLOC_INDEX);
    //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
    m_file.putInt1(FLAG_INDEXEJ);
    m_file.putInt5(0);
    m_file.putInt3(m_dataEntriesBlocSize);
    m_file.writeBuffer();                               //ecriture effective sur le fichier   
    //remplit la zone d'indirection avec des 0
    m_file.writeValue(0, m_dataEntriesBlocSize*m_dataEntrySize);
    //lui colle l'identification du lexique a suivre
    addIdentification(fileIdentification);
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
    const pair<unsigned long int, unsigned int> entrejes(entriesBlock + TETE_BLOC_INDEX, m_dataEntriesBlocSize);
    m_entriesBlocksMap.push_back(entrejes);
}
////////////////////////////////////////////////////////////
//brief get size of 1rst entries block
//return size of 1rst entries block */
unsigned int NindPadFile::getFirstEntriesBlockSize()
{
    m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
    //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
    m_file.readBuffer(TETE_BLOC_INDEX);
    if (m_file.getInt1() != FLAG_INDEXEJ) 
        throw InvalidFileException("NindPadFile::getFirstEntriesBlockSize : " + m_fileName);
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
//brief get identification of file
//param identification where unique identification of file is returned */
void NindPadFile::getFileIdentification(Identification &identification)
{
    m_file.flush();      //pour faire une lecture sur le fichier physique
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw InvalidFileException("NindPadFile::getFileIdentification : " + m_fileName);
    const unsigned int wordsNb = m_file.getInt3();
    const unsigned int time = m_file.getInt4();
    const unsigned int specific = m_file.getInt4();
    identification = Identification(wordsNb, time, specific);
}
////////////////////////////////////////////////////////////
//retourne la position d'une entreje sans synchronisation
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
    m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
    while (true) {
        //<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
        m_file.readBuffer(TETE_BLOC_INDEX);
        if (m_file.getInt1() != FLAG_INDEXEJ) 
            throw InvalidFileException("NindPadFile::mapEntriesBlocks : " + m_fileName);
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
//ejcrit l'identification du fichier ah l'adresse courente du fichier
void NindPadFile::addIdentification(const Identification &fileIdentification)
{
    //le pointeur est censej estre au bon endroit
    m_file.createBuffer(TAILLE_IDENTIFICATION);
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    m_file.putInt1(FLAG_IDENTIFICATION);
    m_file.putInt3(fileIdentification.lexiconWordsNb);
    m_file.putInt4(fileIdentification.lexiconTime);
    m_file.putInt4(fileIdentification.specificFileIdent);
    m_file.writeBuffer();                               //ecriture effective sur le fichier
}
////////////////////////////////////////////////////////////
//vejrifie l'apairage avec la rejfejrence
void NindPadFile::checkIdentification(const Identification &referenceIdentification)
{
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw InvalidFileException("NindPadFile::checkIdentification : " + m_fileName);
    const unsigned int maxIdent = m_file.getInt3();
    const unsigned int identification = m_file.getInt4();
    //si la rejfejrence est nulle, pas de comparaison de valeurs
    if (referenceIdentification == Identification(0, 0, 0)) return;
    //si la rejfejrence n'est pas nulle, comparaison de valeurs non spejcifiques
    if (Identification(maxIdent, identification, 0) != referenceIdentification) {
        throw IncompatibleFileException(m_fileName); 
    }
}
////////////////////////////////////////////////////////////
