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
// <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
//
// <blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
// <flagIndirection>       ::= <Integer1>
// <addrBlocSuivant>       ::= <Integer5>
// <nombreIndirection>     ::= <Integer3>
// <indirection>           ::= <offsetDefinition> <longueurDefinition> 
// <offsetDefinition>      ::= <Integer5>
// <longueurDefinition>    ::= <Integer3>
//
// <blocDefinition>        ::= { <definition> | <vide> }
// <definition>            ::= { <Octet> }
// <vide>                  ::= { <Octet> }
//
// <blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
// <flagIdentification>    ::= <Integer1>
// <maxIdentifiant>        ::= <Integer3>
// <identifieurUnique>     ::= <dateHeure>
// <dateHeure >            ::= <Integer4>
//
////////////////////////////////////////////////////////////
#define FLAG_INDIRECTION 47
#define FLAG_IDENTIFICATION 53
//<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
#define TAILLE_IDENTIFICATION 8
//<flagIndirection> <addrBlocSuivant> <nombreIndirection> = 9
#define TETE_INDIRECTION 9
//<offsetDefinition> <longueurDefinition> = 8
#define TAILLE_INDIRECTION 8
////////////////////////////////////////////////////////////
//brief Creates NindIndex with a specified name associated with.
//param fileName absolute path file name
//param isWriter true if writer, false if reader  
//param lexiconWordsNb number of words contained in lexicon (if 0, no checks)
//param lexiconIdentification unique identification of lexicon  (if 0, no checks)
//param definitionMinimumSize size in bytes of the smallest definition
//param indirectionBlocSize number of entries in a single indirection block */
NindIndex::NindIndex(const std::string &fileName,
                             const bool isWriter,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification,
                             const unsigned int definitionMinimumSize,
                             const unsigned int indirectionBlocSize)
    throw(NindIndexException, InvalidFileException):
    m_file(fileName),
    m_fileName(fileName),
    m_isWriter(isWriter),
    m_definitionMinimumSize(definitionMinimumSize),
    m_indirectionBlocSize(indirectionBlocSize),
    m_indirectionMapping(),
    m_emptyAreas()
{
    try {
        if (m_isWriter) {
            //si fichier inverse ecrivain, ouvre en ecriture + lecture
            bool isOpened = m_file.open("r+b");
            if (isOpened) {
                //si le fichier existe, l'analyse pour trouver les indirections, l'identification et les vides
                //etablit la carte des indirections       
                mapIndirection();
                //verifie l'apairage avec le lexique
                checkIdentification(lexiconWordsNb, lexiconIdentification); 
                //trouve la carte des non vides
                list<pair<unsigned long int, unsigned int> > nonVidesList;
                //les blocs d'indirection sont mis dans les non-vides
                for (list<pair<unsigned long int, unsigned int> >::const_iterator it = m_indirectionMapping.begin(); 
                    it != m_indirectionMapping.end(); it++) {
                    const pair<unsigned long int, unsigned int> nonVide((*it).first - TETE_INDIRECTION, (*it).second * TAILLE_INDIRECTION + TETE_INDIRECTION);
                    nonVidesList.push_back(nonVide);
                }
                //prend toutes les entrees d'indirection
                unsigned int ident = 0;
                unsigned long int indirection = getIndirection(ident);
                while (indirection != 0) {
                    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection du terme
                    m_file.readBuffer(TAILLE_INDIRECTION);
                    //<offsetDefinition> <longueurDefinition> 
                    const unsigned long int offsetDefinition = m_file.getInt5();
                    const unsigned int longueurDefinition = m_file.getInt3();
                    //si le terme n'a pas encore ete indexe, n'en tient pas compte
                    if (offsetDefinition != 0) {
                        const pair<unsigned long int, unsigned int> nonVide(offsetDefinition, longueurDefinition);
                        nonVidesList.push_back(nonVide);
                    }
                    ident++;
                    indirection = getIndirection(ident);
                } 
                //ajoute l'identification dans les non vides
                m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
                //<flagIdentification> <maxIdentifiant> <identifieurUnique>
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
                    if (longueurVide < 0) 
                    {
                      throw InvalidFileException("NindIndex : " + m_fileName);
                    }
                    if (longueurVide > 0) {
                        const pair<unsigned long int, unsigned int> emptyArea(addressePrec + longueurPrec, longueurVide);
                        m_emptyAreas.push_back(emptyArea); 
                    }
                    addressePrec = (*it).first;
                    longueurPrec = (*it).second;           
                }
                //dumpEmptyAreas();
            }
            else {
                //si le fichier n'existe pas, le cree vide en ecriture + lecture
                //la taille du bloc d'indirection doit etre specifiee differente de 0
                if (m_indirectionBlocSize == 0) 
                {
                  std::string errorString = 
                    std::string("NindIndex. In write mode, indirectionBlocSize "
                                "must be non null. ") + m_fileName;
                  std::cerr << errorString << std::endl;
                  throw BadUseException(errorString);
                }
                isOpened = m_file.open("w+b");
                if (!isOpened) throw OpenFileException(m_fileName);
                //lui colle une zone d'indirection vide suivie de l'identification
                addIndirection(lexiconWordsNb, lexiconIdentification);
                //renseigne la zone d'indirection
                const pair<unsigned int, unsigned long int> indirection(TETE_INDIRECTION, m_indirectionBlocSize);
                m_indirectionMapping.push_back(indirection);
            }
        }
        else {
            //si fichier lecteur, ouvre en lecture seule
            bool isOpened = m_file.open("rb");
            if (!isOpened) throw OpenFileException(m_fileName);
            //etablit la carte des indirections       
            mapIndirection();
            //verifie l'apairage avec le lexique
            checkIdentification(lexiconWordsNb, lexiconIdentification); 
        }
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
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
//return true if ident was found, false otherwise */
bool NindIndex::getDefinition(const unsigned int ident)
    throw(EofException, ReadFileException, OutReadBufferException)
{
    const unsigned long int indirection = getIndirection(ident);
    if (indirection == 0) return false;
    m_file.flush();
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection de la definition
    m_file.readBuffer(TAILLE_INDIRECTION);
    //<offsetDefinition> <longueurDefinition>
    const unsigned long int offsetDefinition = m_file.getInt5();
    const unsigned int longueurDefinition = m_file.getInt3();
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
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon 
void NindIndex::setDefinition(const unsigned int ident,
                              const unsigned int lexiconWordsNb,
                              const unsigned int lexiconIdentification)
    throw(EofException, WriteFileException, BadAllocException, OutWriteBufferException, OutReadBufferException, 
        OutOfBoundException)
{
    //1) trouve l'ancienne indirection si elle existe
    unsigned long int indirection = getIndirection(ident);
    if (indirection == 0) throw OutOfBoundException("NindIndex::setTermIndex : " + m_fileName);
    m_file.setPos(indirection, SEEK_SET);    //se positionne sur l'indirection du terme
    //<offsetDefinition> <longueurDefinition> 
    m_file.readBuffer(TAILLE_INDIRECTION);
    const unsigned long int ancienOffsetEntree = m_file.getInt5();
    const unsigned int ancienneLongueurEntree = m_file.getInt3();
    bool identOk = false;       //vrai quand l'identification est ecrite
    //taille de la definition
    const unsigned int tailleDonnees = m_file.getOutBufferSize();
    //on ne cree pas d'entree a taille inferieure a m_definitionMinimumSize
    unsigned int tailleDefinition = tailleDonnees;
    if (tailleDefinition < m_definitionMinimumSize) tailleDefinition = m_definitionMinimumSize;
    //2 cas, soit l'entree est assez grande, soit non
    if (tailleDefinition <= ancienneLongueurEntree) {
        //Si la taille est compatible avec l'ancienne, ecrit au meme endroit
        //3a) ecrit les nouvelles donnees
        m_file.setPos(ancienOffsetEntree, SEEK_SET);    //se positionne sur le terme
        m_file.writeBuffer();                           //ecrit le nouveau buffer
        //l'indirection reste inchangee
    }
    else {
        //Sinon ecrit sur un endroit libre et efface virtuellement
        unsigned long int offsetDefinition = 0;
        unsigned int longueurDefinition = 0;
        //2) trouve une place libre
        //cherche d'abord une place compatible (>= tailleDefinition et < tailleDefinition + m_definitionMinimumSize)
        list<pair<unsigned long int, unsigned int> >::iterator memit = m_emptyAreas.end();
        list<pair<unsigned long int, unsigned int> >::iterator it = m_emptyAreas.begin(); 
        while (it != m_emptyAreas.end()) {
            if ((*it).second >= tailleDefinition) {
                if ((*it).second < tailleDefinition + m_definitionMinimumSize) {
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
                longueurDefinition = tailleDefinition;        //avec la taille juste
                (*memit).first += tailleDefinition;     //retaille l'entree (minimum m_definitionMinimumSize)
                (*memit).second -= tailleDefinition;
            }
            //sinon, ecrit a la fin 
            else {
                offsetDefinition = m_file.getFileSize() - TAILLE_IDENTIFICATION;       //ecrit a la fin 
                longueurDefinition = tailleDefinition;        //avec la taille juste
                //comme c'est en fin de fichier, il faut ecrire la taille de tailleDefinition
                m_file.putPad(tailleDefinition - tailleDonnees);                
                //et l'identification au bout
                //<flagIdentification> <maxIdentifiant> <identifieurUnique>
                m_file.putInt1(FLAG_IDENTIFICATION);  
                m_file.putInt3(lexiconWordsNb);  
                m_file.putInt4(lexiconIdentification); 
                identOk = true;
            }
        }
        //3b) ecrit les nouvelles donnees
        m_file.setPos(offsetDefinition, SEEK_SET);    //se positionne sur le terme
        m_file.writeBuffer();                     //ecrit le nouveau buffer
        //4) met a jour la liste des emplacements vides s'il y a des anciennes donnees
        if (ancienneLongueurEntree != 0) {
            it = m_emptyAreas.begin(); 
            while (it != m_emptyAreas.end()) {
                if ((*it).first + (*it).second == ancienOffsetEntree) {
                    (*it).second += ancienneLongueurEntree;             //globalisation avec l'entree precedente
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
                if ((*it).first == ancienOffsetEntree + ancienneLongueurEntree) {
                    (*it).second += ancienneLongueurEntree;             //globalisation avec l'entree suivante
                    (*it).first = ancienOffsetEntree;
                    break;
                }
                if ((*it).first > ancienOffsetEntree) {
                    const pair<unsigned long int, unsigned int> emptyArea(ancienOffsetEntree, ancienneLongueurEntree);
                    m_emptyAreas.insert(it, emptyArea);                 //nouvelle entree au bon endroit
                    break;
                }
                it++;                    
            }
            if (it == m_emptyAreas.end()) {
                const pair<unsigned long int, unsigned int> emptyArea(ancienOffsetEntree, ancienneLongueurEntree);
                m_emptyAreas.push_back(emptyArea);                 //nouvelle entree a la fin
            }
        }
        //5) ecrit la nouvelle indirection
        m_file.setPos(indirection, SEEK_SET);       //se positionne sur l'indirection du terme
        //<offsetDefinition> <longueurDefinition> 
        m_file.createBuffer(TAILLE_INDIRECTION);
        m_file.putInt5(offsetDefinition);               // <offsetDefinition> 
        m_file.putInt3(longueurDefinition);             //<longueurDefinition>
        m_file.writeBuffer();                       //ecriture effective sur le fichier  
    }
    //6) ecrit la nouvelle identification si pas deja ecrite
    if (!identOk) {
        m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
        //<flagIdentification> <maxIdentifiant> <identifieurUnique>
        m_file.createBuffer(TAILLE_IDENTIFICATION);
        m_file.putInt1(FLAG_IDENTIFICATION);
        m_file.putInt3(lexiconWordsNb);
        m_file.putInt4(lexiconIdentification);
        m_file.writeBuffer();                       //ecriture effective sur le fichier  
        m_file.flush();
    }
}
////////////////////////////////////////////////////////////
//brief verifie que l'indirection existe et cree un bloc supplementaire si c'est pertinent
//param ident ident of definition
//param lexiconWordsNb number of words contained in lexicon 
//param lexiconIdentification unique identification of lexicon 
void NindIndex::checkExtendIndirection(const unsigned int ident,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification)
    throw(BadAllocException, OutWriteBufferException, WriteFileException, OutOfBoundException)
{
    //si le terme n'a pas d'indirection, ajoute un bloc d'indirection
    unsigned long int indirection = getIndirection(ident);
    if (indirection == 0) {
        //le terme est hors des blocs d'indexation actuels, cree un nouveau bloc d'indirection
        m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
        const unsigned long int blocIndirection = m_file.getPos();
        addIndirection(lexiconWordsNb, lexiconIdentification);  //bloc d'indirection = identification a la fin 
        pair<unsigned long int, unsigned int> &indirectionBlocPrec = m_indirectionMapping.back();
        //se positionne sur le <addrBlocSuivant> du dernier bloc
        //<flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
        m_file.setPos(indirectionBlocPrec.first -8, SEEK_SET);   //pour pointer <addrBlocSuivant>
        m_file.createBuffer(5);
        m_file.putInt5(blocIndirection);
        m_file.writeBuffer();
        const pair<unsigned long int, unsigned int> indirectionBloc(blocIndirection + TETE_INDIRECTION, m_indirectionBlocSize);
        m_indirectionMapping.push_back(indirectionBloc);
        //redemande l'indirection pour le terme, erreur si hors limite
        indirection = getIndirection(ident);
        if (indirection == 0) throw OutOfBoundException("NindIndex::checkExtendIndirection : " + m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get size of 1rst indirection block
//return size of 1rst indirection block */
unsigned int NindIndex::getFirstIndirectionBlockSize()
    throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, InvalidFileException)
{
    m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
    //<flagIndirection> <addrBlocSuivant> <nombreIndirection>
    m_file.readBuffer(TETE_INDIRECTION);
    if (m_file.getInt1() != FLAG_INDIRECTION) 
        throw InvalidFileException("NindIndex::mapIndirection : " + m_fileName);
    m_file.getInt5();
    return m_file.getInt3();
}
////////////////////////////////////////////////////////////
//brief get identification of lexicon
//param wordsNb where number of words contained in lexicon is returned
//param identification where unique identification of lexicon is returned */
void NindIndex::getFileIdentification(unsigned int &wordsNb, unsigned int &identification)
    throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, InvalidFileException)
{
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification> <maxIdentifiant> <identifieurUnique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw InvalidFileException("NindIndex::getIdentification : " + m_fileName);
    wordsNb = m_file.getInt3();
    identification = m_file.getInt4();
}
////////////////////////////////////////////////////////////
//etablit la carte des indirections  
void NindIndex::mapIndirection()
    throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, InvalidFileException)
{
    m_file.setPos(0, SEEK_SET);  //positionne en tete du fichier
    while (true) {
        //<flagIndirection> <addrBlocSuivant> <nombreIndirection>
        m_file.readBuffer(TETE_INDIRECTION);
        if (m_file.getInt1() != FLAG_INDIRECTION) 
            throw InvalidFileException("NindIndex::mapIndirection : " + m_fileName);
        const unsigned long int addrBlocSuivant = m_file.getInt5();
        const unsigned int nombreIndirection = m_file.getInt3();
        const unsigned long pos = m_file.getPos(); 
        const pair<unsigned int, unsigned long int> indirection(pos, nombreIndirection);
        m_indirectionMapping.push_back(indirection);
        if (addrBlocSuivant == 0) break;        //si pas d'extension, termine
        //saute au bloc d'indirection suivant
        m_file.setPos(addrBlocSuivant, SEEK_SET);    //pour aller au suivant
    }
}
////////////////////////////////////////////////////////////
//return l'offset de l'indirection du terme specifie, 0 si hors limite
unsigned long int NindIndex::getIndirection(const unsigned int ident)
{
    //trouve l'indirection du terme
    unsigned int firstIdent = 0;
    list<pair<unsigned long int, unsigned int> >::const_iterator it = m_indirectionMapping.begin(); 
    while (it != m_indirectionMapping.end()) {
        if (ident < firstIdent + (*it).second) return (ident - firstIdent) * TAILLE_INDIRECTION + (*it).first;
        firstIdent += (*it).second;
        it++;
    }
    //si le terme cherche n'a pas d'indirection, retourne 0
    return 0;
}
////////////////////////////////////////////////////////////
//ajoute un bloc d'indirection vide suivi d'une identification a la position courante du fichier
void NindIndex::addIndirection(const unsigned int lexiconWordsNb,
                                   const unsigned int lexiconIdentification)
    throw(BadAllocException, OutWriteBufferException, WriteFileException)
{
    //le fichier est deja positionne au bon endroit
    //<flagIndirection_1> <addrBlocSuivant_5> <nombreIndirection_3>
    m_file.createBuffer(TETE_INDIRECTION);
    m_file.putInt1(FLAG_INDIRECTION);
    m_file.putInt5(0);
    m_file.putInt3(m_indirectionBlocSize);
    m_file.writeBuffer();                               //ecriture effective sur le fichier   
    //remplit la zone d'indirection avec des 0
    m_file.writeValue(0, m_indirectionBlocSize*TAILLE_INDIRECTION);
    //lui colle l'identification du lexique a suivre
    //utilise le meme buffer
    //<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
    m_file.putInt1(FLAG_IDENTIFICATION);
    m_file.putInt3(lexiconWordsNb);
    m_file.putInt4(lexiconIdentification);
    m_file.writeBuffer();                               //ecriture effective sur le fichier
}
////////////////////////////////////////////////////////////
//verifie l'apairage avec le lexique
void NindIndex::checkIdentification(const unsigned int lexiconWordsNb,
                                    const unsigned int lexiconIdentification)
    throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, 
          InvalidFileException, IncompatibleFileException)
{
    m_file.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
    //<flagIdentification> <maxIdentifiant> <identifieurUnique>
    m_file.readBuffer(TAILLE_IDENTIFICATION);
    if (m_file.getInt1() != FLAG_IDENTIFICATION) 
        throw InvalidFileException("NindIndex::checkIdentification : " + m_fileName);
    const unsigned int maxIdent = m_file.getInt3();
    const unsigned int identification = m_file.getInt4();
    //si c'est le fichier lexique qui est verifie, pas de comparaison de valeurs
    if (lexiconWordsNb == 0 && lexiconIdentification == 0) return;
    //si ce n'est pas le fichier lexique qui est verifie, comparaison de valeurs
    if (maxIdent != lexiconWordsNb || identification != lexiconIdentification) 
    {
        std::cerr << "NindIndex::checkIdentification failed "
                  << m_fileName << " : "
                  << maxIdent << "/" << lexiconWordsNb
                  << " ; " << identification << "/" << lexiconIdentification
                  << std::endl;
        throw IncompatibleFileException(m_fileName); 
    }
}
////////////////////////////////////////////////////////////
void NindIndex::dumpEmptyAreas()
{
    cout<<"VIDES: "<<m_emptyAreas.size()<<" (";
//      for (list<pair<unsigned int, unsigned long int> >::iterator it = m_emptyAreas.begin(); it != m_emptyAreas.end(); it++) 
//          cout<<(*it).first<<" - "<<(*it).second<<", ";
    cout<<")"<<endl;
}
////////////////////////////////////////////////////////////
