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
#include "NindIndex.h"
//#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <donnejesIndexejes>     ::= <indirection>
// <blocEnVrac>            ::= <blocDejfinition>
//
// <indirection>           ::= <offsetDejfinition> <longueurDejfinition> 
// <offsetDejfinition>     ::= <Integer5>
// <longueurDejfinition>   ::= <Integer3>
//
// <blocDejfinition>       ::= { <dejfinition> | <vide> }
// <dejfinition>           ::= { <Octet> }
// <vide>                  ::= { <Octet> }
////////////////////////////////////////////////////////////
//<offsetDejfinition> <longueurDejfinition> = 8
#define TAILLE_INDIRECTION 8
////////////////////////////////////////////////////////////
//brief Creates NindIndex with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if writer, false if reader  
//param lexiconIdentification unique identification of lexicon  (if 0, no checks)
//param specificsSize size in bytes of specific datas
//param definitionMinimumSize size in bytes of the smallest definition
//param indirectionBlocSize number of entries in a single indirection block */
NindIndex::NindIndex(const std::string &fileName,
                             const bool isWriter,
                             const Identification &lexiconIdentification,
                             const unsigned int specificsSize,
                             const unsigned int definitionMinimumSize,
                             const unsigned int indirectionBlocSize):
    NindPadFile(fileName, 
                isWriter, 
                lexiconIdentification, 
                specificsSize, 
                TAILLE_INDIRECTION, 
                indirectionBlocSize),
    m_definitionMinimumSize(definitionMinimumSize),
    m_emptyAreas()
{
    if (m_isExistingWriter) mapEmptySpaces();
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
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la dejfinition
    m_file.readBuffer(TAILLE_INDIRECTION);
    //<offsetDejfinition> <longueurDejfinition>
    const unsigned long int offsetDejfinition = m_file.getInt5();
    const unsigned int longueurDejfinition = (bytesNb == 0) ? m_file.getInt3() : bytesNb;
    //si la dejfinition n'a pas encore ete indexe, retourne false
    if (offsetDejfinition == 0) return false;
    //et lit tout ce qui concerne la dejfinition
    m_file.setPos(offsetDejfinition, SEEK_SET);    //se positionne sur la dejfinition
    m_file.readBuffer(longueurDejfinition);
    return true;
}
////////////////////////////////////////////////////////////
//brief Write on file datas of a specified definition yet constructed into write buffer
//buffer contains datas of specified definition + specific datas + identification
//param ident ident of definition
void NindIndex::setDefinition(const unsigned int ident)
{
    //taille des spejcifiques et de l'identification qui sont en queue de buffer
    const int tailleQueue = getSpecificsAndIdentificationSize();
    //1) trouve l'ancienne indirection si elle existe
    unsigned long int indirection = getEntryPos(ident);
    if (indirection == 0) throw NindIndexException("NindIndex::setDefinition : " + m_fileName);
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la dejfinition
    //<offsetDejfinition> <longueurDejfinition> 
    m_file.readBuffer(TAILLE_INDIRECTION);
    const unsigned long int oldOffsetEntry = m_file.getInt5();
    const unsigned int oldLengthEntry = m_file.getInt3();
    //taille de la dejfinition
    const unsigned int dataSize = m_file.getOutBufferSize() - tailleQueue;
    //si la taille est ah 0, c'est un effacement
    if (dataSize == 0) {
        //EFFACEMENT
        //met a jour la liste des emplacements vides s'il y a des anciennes donnees
        if (oldLengthEntry != 0) vacateOldArea(oldOffsetEntry, oldLengthEntry);
        //ejcrit sur le fichier les spejcifiques et l'identification qui constituent le buffer
        m_file.setPos(-tailleQueue, SEEK_END);      //se positionne sur les spejcifiques
        m_file.writeBuffer();                       //ejcriture effective sur le fichier  
        //met ah 0 l'indirection sur le fichier
        setIndirection(indirection, 0, 0);
        return;
    }
    //INSERTION ou REMPLACEMENT
    //on est susr que l'entreje a une taille supejrieure ou ejgale ah m_definitionMinimumSize
    //deux cas, soit l'entree est assez grande, soit non
    //si la taille est compatible avec l'ancienne, ejcrit au meme endroit
    if (dataSize <= oldLengthEntry) {
        //REMPLACEMENT AU MESME ENDROIT
        //3a) ecrit les nouvelles donnees
        m_file.setPos(oldOffsetEntry, SEEK_SET);        //se positionne sur la dejfinition
        m_file.writeBuffer(0, dataSize);                //ejcrit la nouvelle dejfinition, juste elle
        //ejcrit sur le fichier les spejcifiques et l'identification qui constituent le buffer
        m_file.setPos(-tailleQueue, SEEK_END);          //se positionne sur les spejcifiques
        m_file.writeBuffer(dataSize, tailleQueue);      //ejcriture effective sur le fichier             
        //l'indirection reste inchangee
        return;
    }
    //INSERTION ou REMPLACEMENT NOUVEL ENDROIT
    //Sinon ecrit sur un endroit libre et efface virtuellement
    unsigned long int offsetDejfinition = 0;
    unsigned int longueurDejfinition = 0;
    //2) trouve une place libre
    //cherche d'abord une place compatible (>= dataSize et < dataSize + m_definitionMinimumSize)
    const bool AlaFin = findNewArea(dataSize, offsetDejfinition, longueurDejfinition);
    //3b) ecrit les nouvelles donnees
    m_file.setPos(offsetDejfinition, SEEK_SET);    //se positionne sur la dejfinition
    //si en fin, ejcrit donnejes + spejcifiques + identification
    if (AlaFin) {
        m_file.writeBuffer();                     //ecrit le buffer entier
    }
    else {
        m_file.writeBuffer(0, dataSize);                //ejcrit la nouvelle dejfinition, juste elle
        //ejcrit sur le fichier les spejcifiques et l'identification qui constituent le buffer
        m_file.setPos(-tailleQueue, SEEK_END);          //se positionne sur les spejcifiques
        m_file.writeBuffer(dataSize, tailleQueue);      //ejcriture effective sur le fichier             
    }
    //4) met a jour la liste des emplacements vides s'il y a des anciennes donnees
    if (oldLengthEntry != 0) vacateOldArea(oldOffsetEntry, oldLengthEntry);
    //5) ecrit la nouvelle indirection
    setIndirection(indirection, offsetDejfinition, longueurDejfinition);
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
        if (indirection == 0) throw NindIndexException("NindIndex::checkExtendIndirection : " + m_fileName);
    }
}
////////////////////////////////////////////////////////////
//ejcrit une nouvelle indirection dans l'index
void NindIndex::setIndirection(const unsigned long int indirection,
                               const unsigned long int offsetDejfinition,
                               const unsigned int longueurDejfinition)
{
    //se positionne sur l'indirection de la dejfinition
    m_file.setPos(indirection, SEEK_SET);
    //<offsetDejfinition> <longueurDejfinition> 
    m_file.createBuffer(TAILLE_INDIRECTION);
    m_file.putInt5(offsetDejfinition);               // <offsetDejfinition> 
    m_file.putInt3(longueurDejfinition);             //<longueurDejfinition>
    m_file.writeBuffer();                           //ecriture effective sur le fichier      
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
        const pair<unsigned long int, unsigned int> nonVide((*it).first - TAILLE_TETE_INDEX, (*it).second * TAILLE_INDIRECTION + TAILLE_TETE_INDEX);
        nonVidesList.push_back(nonVide);
    }
    //prend toutes les entrees d'indirection
    unsigned int ident = 0;
    unsigned long int indirection = getEntryPos(ident);
    while (indirection != 0) {
        m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la dejfinition
        m_file.readBuffer(TAILLE_INDIRECTION);
        //<offsetDejfinition> <longueurDejfinition> 
        const unsigned long int offsetDejfinition = m_file.getInt5();
        const unsigned int longueurDejfinition = m_file.getInt3();
        //si la dejfinition n'a pas encore ete indexe, n'en tient pas compte
        if (offsetDejfinition != 0) {
            const pair<unsigned long int, unsigned int> nonVide(offsetDejfinition, longueurDejfinition);
            nonVidesList.push_back(nonVide);
        }
        ident++;
        indirection = getEntryPos(ident);
    } 
    //ajoute les spejcifiques et l'identification dans les non vides
    //taille des spejcifiques et de l'identification qui sont en queue de buffer
    const int tailleQueue = getSpecificsAndIdentificationSize();
    m_file.setPos(-tailleQueue, SEEK_END);       //se positionne sur les spejcifiques
    const pair<unsigned long int, unsigned int> nonVide(m_file.getPos(), tailleQueue);
    nonVidesList.push_back(nonVide);
    //ordonne les non vides
    nonVidesList.sort();
    //remplit la carte des vides
    unsigned long int addressePrec = 0;
    unsigned int longueurPrec = 0;
    for (list<pair<unsigned long int, unsigned int> >::const_iterator it = nonVidesList.begin();
        it != nonVidesList.end(); it++) {
        const unsigned int longueurVide = (*it).first - addressePrec - longueurPrec;
        if (longueurVide < 0) throw NindIndexException("NindIndex::mapEmptySpaces " + m_fileName);
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
bool NindIndex::findNewArea(const unsigned int dataSize,
                            unsigned long int &offsetDejfinition,
                            unsigned int &longueurDejfinition)
{
    list<pair<unsigned long int, unsigned int> >::iterator memit = m_emptyAreas.end();
    list<pair<unsigned long int, unsigned int> >::iterator it = m_emptyAreas.begin(); 
    while (it != m_emptyAreas.end()) {
        if ((*it).second >= dataSize) {
            //taille suffisante
            if ((*it).second < dataSize + m_definitionMinimumSize) {
                //mais pas trop grande
                offsetDejfinition = (*it).first;       //c'est la qu'il faut ecrire
                longueurDejfinition = (*it).second;    //on conserve la meme longueur pour des raisons de perfs 
                m_emptyAreas.erase(it);               //efface l'entree
                return false;                         //pas ah la fin
            }
            //memorise le plus petit parmi les trop grands
            if (memit != m_emptyAreas.end()) {                  //premiere comparaison ?
                if ((*it).second < (*memit).second) memit = it; //non, mejmorise le plus petit
            }
            else memit = it;                                    //oui mejmorise le premier 
        }
        it++;
    }
    //pas trouvej, cherche une place plus grande
    //si une place plus grande a ete trouve, en prend une partie
    if (memit != m_emptyAreas.end()) {
        offsetDejfinition = (*memit).first;       //c'est la qu'il faut ecrire
        longueurDejfinition = dataSize;     //avec la taille juste
        (*memit).first += dataSize;        //retaille l'entree (minimum m_definitionMinimumSize)
        (*memit).second -= dataSize;
        return false;                            //pas ah la fin
    }
    //ejcrit a la fin 
    offsetDejfinition = m_file.getFileSize() - getSpecificsAndIdentificationSize();       //ecrit a la fin 
    longueurDejfinition = (dataSize > m_definitionMinimumSize)? dataSize : m_definitionMinimumSize;  //avec la taille 
    return true;                                //ah la fin
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

