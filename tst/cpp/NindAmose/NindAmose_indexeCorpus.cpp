//
// C++ Implementation: NindAmose_indexeCorpus
//
// Description: un programme pour remplir le lexique, le fichier inverse et le fichier des index locaux
// a partir d'un corpus deja syntaxiquement analyse issu d'un dump Lucene.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: 2014-2016 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <stringstream>
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
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le lexique,"<<endl;
    cout<<"le fichier inversé et le fichier des index locaux."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <taille lexique> <taille inverse> <taille locaux>"<<endl;
    cout<<"ex :   "<<arg0<<" amose-dump-lucene-index-fre-10.xml.txt 0 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
static void split(const string &word, 
                  list<string> &simpleWords);
static void majInverse (const unsigned int id,
                        const unsigned int noDoc,
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
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexiconindex";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        const string localindexFileName = docsFileName.substr(0, pos) + ".localindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        /////////////////////////////////////
        FILE *file =  fopen(lexiconFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<lexiconFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<lexiconFileName<<endl;
            return false;
        }
        file =  fopen(termindexFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<termindexFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<termindexFileName<<endl;
            return false;
        }
        file =  fopen(localindexFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<localindexFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<localindexFileName<<endl;
            return false;
        }
        /////////////////////////////////////
        cout<<"Forme le lexique, le fichier inversé et le fichier des index locaux avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain
        NindLexiconIndex nindLexicon(lexiconFileName, 
                                     true, 
                                     lexiconEntryNb);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse ecrivain
        NindTermIndex *nindTermIndex = new NindTermIndex(termindexFileName, 
                                                         true, 
                                                         wordsNb, 
                                                         identification,
                                                         termindexEntryNb);
        //le fichier des index locaux
        NindLocalIndex *nindLocalIndex = new NindLocalIndex(localindexFileName, 
                                                            true, 
                                                            wordsNb, 
                                                            identification,
                                                            localindexEntryNb);
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        unsigned int nbMaj = 0;
        string dumpLine;
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (getline(docsFile, dumpLine)) {
            //lit 1 ligne = 1 document
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            if (dumpLine.empty()) continue;   //evacue ainsi les lignes vides
            stringstream sdumpLine(dumpLine);
            //10170346  Location.LOCATION:Italie 280,6 création 288,8 création_parti 288,19
            docsNb++;
            unsigned int noDoc;
            string word;
            unsigned int pos, taille;
            char comma;
            sdumpLine >> noDoc;
            noDoc -= 10170000;
            //la structure d'index locaux se fabrique pour un document complet
            list<NindLocalIndex::Term> localIndex;
            //lit tous les termes et leur localisation/taille
            while (sdumpLine >> word >> pos >> comma >> taille) {
                //le terme
                list<string> componants;
                split(word, componants);
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexicon.addWord(componants);
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termIndex;
                //met a jour la definition du terme
                nindTermIndex->getTermIndex(id, termIndex);
                //si le terme n'existe pas encore, la liste reste vide
                majInverse(id, noDoc, termIndex); 
                //recupere l'identification du lexique
                nindLexicon.getIdentification(wordsNb, identification);
                //ecrit sur le fichier inverse
                nindTermIndex->setTermIndex(id, termIndex, wordsNb, identification);
                nbMaj +=1;
                //augmente l'index local 
                localIndex.push_back(NindLocalIndex::Term(id, NO_CG));
                NindLocalIndex::Term &term = localIndex.back();
                term.localisation.push_back(NindLocalIndex::Localisation(pos, taille));               
            }
            //ecrit la definition sur le fichier des index locaux
            nindLocalIndex->setLocalIndex(noDoc, localIndex, wordsNb, identification);
        }
        docsFile.close();
        end = clock();
        cout<<nbMaj<<" accès / mises à jour sur "<<lexiconFileName<<endl;
        cout<<nbMaj<<" mises à jour sur "<<termindexFileName<<endl;
        cout<<docsNb<<" mises à jour sur "<<localindexFileName<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
//brief split words into single words
//param word composed word with "#"
//param simpleWords return list of single words */
static void split(const string &word, 
                  list<string> &simpleWords)
{
    simpleWords.clear();
    string simpleWord;
    stringstream sword(word);
    while (getline(sword, simpleWord, '_')) {
        simpleWords.push_back(simpleWord);
    }
}
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int id,
                        const unsigned int noDoc,
                        list<NindTermIndex::TermCG> &termIndex) 
{
    list<NindTermIndex::TermCG>::iterator it1 = termIndex.begin(); 
    //il n'y a pas de cg, donc 0 ou 1 termCG
    if (it1 != termIndex.end()) {
        //si le terme existe deja, on ajoute le doc
        list<NindTermIndex::Document> &documents = (*it1).documents;
        //trouve la place dans la liste ordonnee
        list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
        while (it2 != documents.end()) {
            //deja dans la liste, incremente la frequence
            if ((*it2).ident == noDoc) {
                (*it2).frequency +=1;  
                break;
            }
            //insere a l'interieur de la liste
            if ((*it2).ident > noDoc) {
                documents.insert(it2, NindTermIndex::Document(noDoc, 1));
                break;
            }
            it2++;
        }
        //si fin de liste, insere en fin
        if (it2 == documents.end()) documents.push_back(NindTermIndex::Document(noDoc, 1));
        //met a jour la frequence globale de la cg
        (*it1).frequency +=1;
    }
    else {
        //c'est un nouveau terme
        termIndex.push_back(NindTermIndex::TermCG(NO_CG, 1));
        NindTermIndex::TermCG &termCG = termIndex.back();
        list<NindTermIndex::Document> &documents = termCG.documents;
        documents.push_back(NindTermIndex::Document(noDoc, 1));
    }
}
