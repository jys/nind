//
// C++ Implementation: NindIndex
//
// Description: La gestion d'un fichier (inverse ou d'index locaux)
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
// Cette classe est dérivable.
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
#include "NindIndex.h"
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
// <donnejesIndexejes>     ::= <blocIndirection>
// <blocEnVrac>            ::= <blocDefinition>
//
// <blocIndirection>       ::= <flagIndirection=47> <addrBlocSuivant> <nombreIndirection> { indirection }
// <flagIndirection=47>    ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreIndirection>     ::= <Integer3>
// <indirection>           ::= <offsetDefinition> <longueurDefinition> 
// <offsetDefinition>      ::= <Integer5>
// <longueurDefinition>    ::= <Integer3>
//
// <blocDefinition>        ::= { <definition> | <vide> }
// <definition>            ::= { <Octet> }
// <vide>                  ::= { <Octet> }
////////////////////////////////////////////////////////////
//<offsetDefinition> <longueurDefinition> = 8
#define TAILLE_INDIRECTION 8
////////////////////////////////////////////////////////////
//brief Creates NindIndex with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if writer, false if reader  
//param lexiconIdentification unique identification of lexicon  (if 0, no checks)
//param definitionMinimumSize size in bytes of the smallest definition
//param indirectionBlocSize number of entries in a single indirection block */
NindIndex::NindIndex(const std::string &fileName,
                             const bool isWriter,
                             const Identification &lexiconIdentification,
                             const unsigned int definitionMinimumSize,
                             const unsigned int indirectionBlocSize):
    NindPadFile(fileName, isWriter, lexiconIdentification, TAILLE_INDIRECTION, indirectionBlocSize),
    m_definitionMinimumSize(definitionMinimumSize),
    countterm(0), countpasterm(0), counttotal(0),
    m_emptyAreas()
{
    try {
        if (m_isExistingWriter) mapEmptySpaces();
    }
    catch (FileException &exc) {
        throw NindIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
NindIndex::~NindIndex()
{
    m_file.close();
}
////////////////////////////////////////////////////////////
//brief Read from file datas of a specified definition and leave result into read buffer 
//param ident ident of definition
//param bytesNb optional number of bytes to read, in case of partial read
//return true if ident was found, false otherwise */
bool NindIndex::getDefinition(const unsigned int ident,
                              const unsigned int bytesNb)
{
    unsigned long int indirection = getEntryPos(ident);
    if (indirection == 0) return false;
    m_file.flush();
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la definition
    m_file.readBuffer(TAILLE_INDIRECTION);
    //<offsetDefinition> <longueurDefinition>
    const unsigned long int offsetDefinition = m_file.getInt5();
    const unsigned int longueurDefinition = (bytesNb == 0) ? m_file.getInt3() : bytesNb;
    //si la definition n'a pas encore ete indexe, retourne false
    if (offsetDefinition == 0) return false;
    //et lit tout ce qui concerne la definition
    m_file.setPos(offsetDefinition, SEEK_SET);    //se positionne sur la definition
    m_file.readBuffer(longueurDefinition);
    return true;
}
////////////////////////////////////////////////////////////
//brief Write on file datas of a specified definition yet constructed into write buffer
//param ident ident of definition
//param fileIdentification unique identification of file 
void NindIndex::setDefinition(const unsigned int ident,
                              const Identification &fileIdentification)
{
    counttotal++;
    //indicateur que l'identification a dejjah ejtej ejcrite
    bool identOk = false;
    //1) trouve l'ancienne indirection si elle existe
    unsigned long int indirection = getEntryPos(ident);
    if (indirection == 0) throw OutOfBoundException("NindIndex::setDefinition : " + m_fileName);
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la dejfinition
    //<offsetDefinition> <longueurDefinition> 
    m_file.readBuffer(TAILLE_INDIRECTION);
    const unsigned long int oldOffsetEntry = m_file.getInt5();
    const unsigned int oldLengthEntry = m_file.getInt3();
    //taille de la definition
    const unsigned int dataSize = m_file.getOutBufferSize();
    //si la taille est ah 0, c'est un effacement
    if (dataSize == 0) {
        //EFFACEMENT
        //met a jour la liste des emplacements vides s'il y a des anciennes donnees
        if (oldLengthEntry != 0) vacateOldArea(oldOffsetEntry, oldLengthEntry);
        m_file.setPos(indirection, SEEK_SET);       //se positionne sur l'indirection de la dejfinition
        //<offsetDefinition> <longueurDefinition> 
        m_file.createBuffer(TAILLE_INDIRECTION);
        m_file.putInt5(0);               // <offsetDefinition> 
        m_file.putInt3(0);               //<longueurDefinition>
        m_file.writeBuffer();            //ecriture effective sur le fichier  
    }
    else {
        //INSERTION ou REMPLACEMENT
        //on ne cree pas d'entree a taille inferieure a m_definitionMinimumSize
        unsigned int definitionSize = dataSize;
        if (definitionSize < m_definitionMinimumSize) definitionSize = m_definitionMinimumSize;
        //2 cas, soit l'entree est assez grande, soit non
        if (definitionSize <= oldLengthEntry) {
            //Si la taille est compatible avec l'ancienne, ecrit au meme endroit
            //3a) ecrit les nouvelles donnees
            m_file.setPos(oldOffsetEntry, SEEK_SET);    //se positionne sur la dejfinition
            m_file.writeBuffer();                           //ecrit le nouveau buffer
            //l'indirection reste inchangee
        }
        else {
            //Sinon ecrit sur un endroit libre et efface virtuellement
            unsigned long int offsetDefinition = 0;
            unsigned int longueurDefinition = 0;
            //2) trouve une place libre
            //cherche d'abord une place compatible (>= tailleDefinition et < tailleDefinition + m_definitionMinimumSize)
            identOk = findNewArea(definitionSize, dataSize, fileIdentification, offsetDefinition, longueurDefinition);
            //3b) ecrit les nouvelles donnees
            m_file.setPos(offsetDefinition, SEEK_SET);    //se positionne sur la dejfinition
            m_file.writeBuffer();                     //ecrit le nouveau buffer
            //4) met a jour la liste des emplacements vides s'il y a des anciennes donnees
            if (oldLengthEntry != 0) vacateOldArea(oldOffsetEntry, oldLengthEntry);
            //5) ecrit la nouvelle indirection
            m_file.setPos(indirection, SEEK_SET);       //se positionne sur l'indirection de la dejfinition
            //<offsetDefinition> <longueurDefinition> 
            m_file.createBuffer(TAILLE_INDIRECTION);
            m_file.putInt5(offsetDefinition);               // <offsetDefinition> 
            m_file.putInt3(longueurDefinition);             //<longueurDefinition>
            m_file.writeBuffer();                           //ecriture effective sur le fichier  
        }
    }
    //6) ecrit la nouvelle identification si pas deja ecrite
    if (!identOk) {
        m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END); 
        addIdentification(fileIdentification);
        countpasterm++;
    }
    else countterm++;
}
////////////////////////////////////////////////////////////
//brief verifie que l'indirection existe et cree un bloc supplementaire si c'est pertinent
//param ident ident of definition
//param fileIdentification unique identification of file 
void NindIndex::checkExtendIndirection(const unsigned int ident,
                                       const Identification &fileIdentification)
{
    //si la dejfinition n'a pas d'indirection, ajoute un bloc d'indirection
    unsigned long int indirection = getEntryPos(ident);
    if (indirection == 0) {
        //la dejfinition est hors des blocs d'indexation actuels, cree un nouveau bloc d'indirection
        addEntriesBlock(fileIdentification);
        //redemande l'indirection pour la dejfinition, erreur si hors limite
        indirection = getEntryPos(ident);
        if (indirection == 0) throw OutOfBoundException("NindIndex::checkExtendIndirection : " + m_fileName);
    }
}
////////////////////////////////////////////////////////////
//ejtablit la carte des vides
void NindIndex::mapEmptySpaces()
{
    //trouve la carte des non vides
    list<pair<unsigned long int, unsigned int> > nonVidesList;
    //les blocs d'indirection sont mis dans les non-vides
    for (list<pair<unsigned long int, unsigned int> >::const_iterator it = m_entriesBlocksMap.begin(); 
        it != m_entriesBlocksMap.end(); it++) {
        const pair<unsigned long int, unsigned int> nonVide((*it).first - TETE_BLOC_INDEX, (*it).second * TAILLE_INDIRECTION + TETE_BLOC_INDEX);
        nonVidesList.push_back(nonVide);
    }
    //prend toutes les entrees d'indirection
    unsigned int ident = 0;
    unsigned long int indirection = getEntryPos(ident);
    while (indirection != 0) {
        m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la dejfinition
        m_file.readBuffer(TAILLE_INDIRECTION);
        //<offsetDefinition> <longueurDefinition> 
        const unsigned long int offsetDefinition = m_file.getInt5();
        const unsigned int longueurDefinition = m_file.getInt3();
        //si la dejfinition n'a pas encore ete indexe, n'en tient pas compte
        if (offsetDefinition != 0) {
            const pair<unsigned long int, unsigned int> nonVide(offsetDefinition, longueurDefinition);
            nonVidesList.push_back(nonVide);
        }
        ident++;
        indirection = getEntryPos(ident);
    } 
    //ajoute l'identification dans les non vides
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    const pair<unsigned long int, unsigned int> nonVide(m_file.getPos(), TAILLE_IDENTIFICATION);
    nonVidesList.push_back(nonVide);
    //ordonne les non vides
    nonVidesList.sort();
    //remplit la carte des vides
    unsigned long int addressePrec = 0;
    unsigned int longueurPrec = 0;
    for (list<pair<unsigned long int, unsigned int> >::const_iterator it = nonVidesList.begin();
        it != nonVidesList.end(); it++) {
        const unsigned int longueurVide = (*it).first - addressePrec - longueurPrec;
        if (longueurVide < 0) throw InvalidFileException("NindIndex::mapEmptySpaces " + m_fileName);
        if (longueurVide > 0) {
            const pair<unsigned long int, unsigned int> emptyArea(addressePrec + longueurPrec, longueurVide);
            m_emptyAreas.push_back(emptyArea); 
        }
        addressePrec = (*it).first;
        longueurPrec = (*it).second;           
    }
    //dumpEmptyAreas();
}
////////////////////////////////////////////////////////////
//trouve une nouvelle zone pour les nouvelles donnejs
bool NindIndex::findNewArea(const unsigned int definitionSize,
                            const unsigned int dataSize,
                            const Identification &fileIdentification,
                            unsigned long int &offsetDefinition,
                            unsigned int &longueurDefinition)
{
    list<pair<unsigned long int, unsigned int> >::iterator memit = m_emptyAreas.end();
    list<pair<unsigned long int, unsigned int> >::iterator it = m_emptyAreas.begin(); 
    while (it != m_emptyAreas.end()) {
        if ((*it).second >= definitionSize) {
            if ((*it).second < definitionSize + m_definitionMinimumSize) {
                offsetDefinition = (*it).first;       //c'est la qu'il faut ecrire
                longueurDefinition = (*it).second;      //on conserve la meme longueur pour des raisons de perfs 
                m_emptyAreas.erase(it);            //efface l'entree
                break;
            }
            else {
                //memorise le plus petit parmi les trop grands
                if (memit != m_emptyAreas.end()) {
                    if ((*it).second < (*memit).second) memit = it;
                }
                else memit = it;
            }                   
        }
        it++;
    }
    //si pas trouve, cherche une place plus grande
    if (it == m_emptyAreas.end()) {
        //si une place plus grande a ete trouve, en prend une partie
        if (memit != m_emptyAreas.end()) {
            offsetDefinition = (*memit).first;       //c'est la qu'il faut ecrire
            longueurDefinition = definitionSize;        //avec la taille juste
            (*memit).first += definitionSize;     //retaille l'entree (minimum m_definitionMinimumSize)
            (*memit).second -= definitionSize;
        }
        //sinon, ecrit a la fin 
        else {
            offsetDefinition = m_file.getFileSize() - TAILLE_IDENTIFICATION;       //ecrit a la fin 
            longueurDefinition = definitionSize;        //avec la taille juste
            //comme c'est en fin de fichier, il faut ecrire la taille de definitionSize
            m_file.putPad(definitionSize - dataSize);                
            //et l'identification au bout (on  ejcrit dans le buffer, pas sur le fichier)
            //<flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
            m_file.putInt1(FLAG_IDENTIFICATION);  
            m_file.putInt3(fileIdentification.lexiconWordsNb);  
            m_file.putInt4(fileIdentification.lexiconTime); 
            m_file.putInt4(fileIdentification.specificFileIdent); 
            return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////
//brief Place l'ancienne zone de donnejes dans la gestion du vide
void NindIndex::vacateOldArea(const unsigned long int oldOffsetEntry,
                              const unsigned int oldLengthEntry)
{
    list<pair<unsigned long int, unsigned int> >::iterator it = m_emptyAreas.begin(); 
    while (it != m_emptyAreas.end()) {
        if ((*it).first + (*it).second == oldOffsetEntry) {
            (*it).second += oldLengthEntry;             //globalisation avec l'entree precedente
            //ca peut arriver que la globalisation avec la suivante soit aussi possible
            list<pair<unsigned long int, unsigned int> >::iterator nextit = it; 
            nextit++;
            if (nextit != m_emptyAreas.end()) {
                if ((*it).first + (*it).second == (*nextit).first) {
                    (*it).second += (*nextit).second;
                    m_emptyAreas.erase(nextit);            //efface l'entree suivante                           
                }
            }
            break;
        }
        if ((*it).first == oldOffsetEntry + oldLengthEntry) {
            (*it).second += oldLengthEntry;             //globalisation avec l'entree suivante
            (*it).first = oldOffsetEntry;
            break;
        }
        if ((*it).first > oldOffsetEntry) {
            const pair<unsigned long int, unsigned int> emptyArea(oldOffsetEntry, oldLengthEntry);
            m_emptyAreas.insert(it, emptyArea);                 //nouvelle entree au bon endroit
            break;
        }
        it++;                    
    }
    if (it == m_emptyAreas.end()) {
        const pair<unsigned long int, unsigned int> emptyArea(oldOffsetEntry, oldLengthEntry);
        m_emptyAreas.push_back(emptyArea);                 //nouvelle entree a la fin
    }
}
////////////////////////////////////////////////////////////
// void NindIndex::dumpEmptyAreas()
// {
//     cout<<"VIDES: "<<m_emptyAreas.size()<<" (";
// //      for (list<pair<unsigned int, unsigned long int> >::iterator it = m_emptyAreas.begin(); it != m_emptyAreas.end(); it++) 
// //          cout<<(*it).first<<" - "<<(*it).second<<", ";
//     cout<<")"<<endl;
// }
////////////////////////////////////////////////////////////

