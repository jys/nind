//
// C++ Implementation: Nind_indexeCorpus
//
// Description: un programme pour remplir le lexique, le fichier inverse et le fichier des index locaux
// a partir d'un corpus deja syntaxiquement analyse issu d'un dump Lucene.
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
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndex/NindLexiconIndex.h"
#include "NindAmose_litTexteAnalysej.h"
#include "NindIndex/NindDate.h"
#include "NindIndex/NindFichiers.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <iomanip> 
#include <fstream>
#include <sstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'indexation d'un corpus déjà syntaxiquement analysé issu d'un"<<endl;
    cout<<"dump Lucene d'un corpus Amose."<<endl;
    cout<<"Le corpus est un fichier texte avec une ligne par document :"<<endl;
    cout<<"<n° document>  { <terme> <localisation>,<taille> }"<<endl;
    cout<<"Le lexique et les fichiers inverse et d'index locaux sont créés."<<endl;
    cout<<"Les fichiers lexique, inverse et d'index locaux doivent être absents."<<endl;
    cout<<"Les documents sont indexés au fur et à mesure de leur lecture."<<endl;
    cout<<"L'indexation des termes et des documents se fait document par document."<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le lexique,"<<endl;
    cout<<"le fichier inversé et le fichier des index locaux."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <taille lexique> <taille inverse> <taille locaux>"<<endl;
    cout<<"ex :   "<<arg0<<" sample_fre.xml.mult.xml.txt 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
static void splitWord(const string &lemma,
                      const unsigned int type,
                      const string &namedEntity,
                      list<string> &simpleWords);
static void majInverse (const unsigned int noDoc,
                        const unsigned int freq,
                        list<NindTermIndex::TermCG> &termIndex);
////////////////////////////////////////////////////////////
#define NO_CG 0
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<4) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string lexiconEntryNbStr = argv[2];
    const string termindexEntryNbStr = argv[3];
    const string localindexEntryNbStr = argv[4];
    
    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    const unsigned int termindexEntryNb = atoi(termindexEntryNbStr.c_str());
    const unsigned int localindexEntryNb = atoi(localindexEntryNbStr.c_str());

    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const string incompleteFileName = docsFileName.substr(0, docsFileName.find('.'));
        //vejrifie que le systehme de fichiers est cohejrent
        if (!NindFichiers::fichiersCohejrents(incompleteFileName, false, false)) {
            cout<<"Des anciens fichiers existent et sont incohérents!"<<endl;
            cout<<"Veuillez les effacer par la commande : rm "<<incompleteFileName + ".nind{*index,retrolexicon}"<<endl;
            return false;
        }
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        /////////////////////////////////////
        cout<<"Forme le lexique, le fichier inversé et le fichier des index locaux avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain avec retro lexique (meme taille d'indirection que le fichier inverse)
        NindLexiconIndex nindLexicon(incompleteFileName, true, false, lexiconEntryNb, termindexEntryNb);
        NindIndex::Identification identification = nindLexicon.getIdentification();
        //le fichier inverse ecrivain
        NindTermIndex nindTermIndex(incompleteFileName, true, identification, 0, termindexEntryNb);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(incompleteFileName, true, identification, localindexEntryNb);
        //lit le fichier dump de documents
        //lit le fichier dump de documents
        NindAmose_litTexteAnalysej nindAmose_litTexteAnalysej(docsFileName);
        unsigned int docsNb = 0;
        unsigned int nbMajTerm = 0, nbMajLex = 0;
        unsigned int noDoc;
        const list<unsigned int> spejcifiques;
        while (nindAmose_litTexteAnalysej.documentSuivant(noDoc)) {
            docsNb++;
            cout<<noDoc<<"\r"<<flush;
            //bufferisation des termes pour un mesme document
            map<unsigned int, unsigned int> bufferTermesParDoc;
            //la structure d'index locaux se fabrique pour un document complet
            list<NindLocalIndex::Term> localIndex;
            string lemme;
            AmoseTypes type;
            string entitejNommeje;
            unsigned int position, taille;
            while (nindAmose_litTexteAnalysej.motSuivant(lemme, type, entitejNommeje, position, taille)) {
                //le terme
                list<string> componants;
                splitWord(lemme, type, entitejNommeje, componants);
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexicon.addWord(componants);
                nbMajLex++;
                //cherche s'il existe dejjah dans le buffer
                map<unsigned int, unsigned int>::iterator itterm = bufferTermesParDoc.find(id);
                //s'il n'existe pas, le creje 
                if (itterm == bufferTermesParDoc.end()) bufferTermesParDoc[id] = 1;
                //sinon increjmente le compteur
                else (*itterm).second++;
                //augmente l'index local 
                localIndex.push_back(NindLocalIndex::Term(id, NO_CG));
                NindLocalIndex::Term &term = localIndex.back();
                term.localisation.push_back(NindLocalIndex::Localisation(position, taille));               
            }
            //recupere l'identification du lexique
            identification = nindLexicon.getIdentification();
            //indexe tous les termes trouvejs dans le document
            for (map<unsigned int, unsigned int>::const_iterator itterm = bufferTermesParDoc.begin();
                 itterm != bufferTermesParDoc.end(); itterm++) {
                const unsigned int &idterm = (*itterm).first;
                const unsigned int &freq = (*itterm).second;
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termIndex;
                //met a jour la definition du terme
                nindTermIndex.getTermDef(idterm, termIndex);
                //si le terme n'existe pas encore, la liste reste vide
                majInverse(noDoc, freq, termIndex); 
                //ecrit sur le fichier inverse
                nindTermIndex.setTermDef(idterm, termIndex, identification, spejcifiques);
                nbMajTerm +=1;
            }
            //ecrit la definition sur le fichier des index locaux
            nindLocalIndex.setLocalDef(noDoc, localIndex, identification);
        }
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nbMajLex<<" accès / mises à jour sur "<<nindLexicon.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbMajTerm<<" mises à jour sur "<<nindTermIndex.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<docsNb<<" mises à jour sur "<<nindLocalIndex.getFileName()<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int noDoc,
                        const unsigned int freq,
                        list<NindTermIndex::TermCG> &termIndex) 
{
    //si le terme n'existe pas encore, la liste est crejeje avec un ejlejment
    if (termIndex.size() == 0) {
        //creje un ejlejment vide
        termIndex.push_back(NindTermIndex::TermCG());
    }
    //travaille sur l'unique ejlejment
    NindTermIndex::TermCG &termcg = termIndex.front();
    list<NindTermIndex::Document> &documents = termcg.documents;       //documents dejjah lah
    //trouve la place dans la liste ordonnee
    list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
    while (it2 != documents.end()) {
        //deja dans la liste, incremente la frequence
        if ((*it2).ident == noDoc) {
            (*it2).frequency += freq;  
            break;
        }
        //insere ah l'interieur de la liste
        if ((*it2).ident > noDoc) {
            documents.insert(it2, NindTermIndex::Document(noDoc, freq));
            break;
        }
        it2++;
    }
    //si fin de liste, insere en fin
    if (it2 == documents.end()) documents.push_back(NindTermIndex::Document(noDoc, freq));
    //met a jour la frequence globale de la cg
    termcg.frequency += freq;
}
////////////////////////////////////////////////////////////
//met en forme le mot pour interroger le lexique
static void splitWord(const string &lemma,
                      const unsigned int type,
                      const string &namedEntity,
                      list<string> &simpleWords)
{
    //le premier ejlejment d'une entitej nommeje est le type de l'entitej
    if (type == NAMED_ENTITY) {
        simpleWords.push_back("§");
        simpleWords.push_back(namedEntity);
    }
    string simpleWord;
    stringstream sword(lemma);
    while (getline(sword, simpleWord, '_')) {
        if (!simpleWord.empty()) simpleWords.push_back(simpleWord);
    }
}
////////////////////////////////////////////////////////////
