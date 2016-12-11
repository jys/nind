//
// C++ Implementation: NindIndex_testLecteur
//
// Description: un petit moteur de recherche sur un lexique, fichier inverse et fichier des index locaux existants.
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
//#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndex/NindIndexTest.h"
#include "NindIndex/NindDate.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme d'interrogation d'une base nind préalablement indexée"<<endl;
    cout<<"L'utilisateur indique le terme cherché."<<endl;
    cout<<"Ce peut être un terme simple ou un terme composé."<<endl;
    cout<<"Un terme composé a la forme dièsée (ex \"kyrielle#outil#informatique\")."<<endl;
    cout<<"L'utilisateur indique ensuite le document examiné"<<endl;
    cout<<"et la localisation du terme cherché est affichée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier lexique>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.lexiconindex"<<endl;
}
////////////////////////////////////////////////////////////
#define OFF "\33[m"
#define BLUE "\33[0;34m"
#define BOLD "\033[1;31m"

////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string lexiconFileName = argv[1];
    if (lexiconFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //calcule les noms des fichiers lexique et inverse
        const size_t pos = lexiconFileName.find('.');
        const string termindexFileName = lexiconFileName.substr(0, pos) + ".termindex";
        const string localindexFileName = lexiconFileName.substr(0, pos) + ".localindex";
        //le lexique lecteur
        NindLexiconIndex nindLexicon(lexiconFileName, false);
        NindIndex::Identification identification;
        nindLexicon.getIdentification(identification);
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(termindexFileName, false, identification);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(localindexFileName, false, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        
        while (true) {
            char str [80];
            cout<<endl<<BLUE<<"Entrez le terme à rechercher : "<<OFF;
            cin.getline(str, 80, '\n');
            const string word = string(str);
            if (word.empty()) break;
            //le terme
            list<string> componants;
            nindIndexTest.split(word, componants);
            //trouve son identifiant
            const unsigned int ident = nindLexicon.getWordId(componants);
            if (ident == 0) {
                cout<<"INCONNU"<<endl;
                continue;
            }
            //recupere l'index inverse pour ce terme
            list<NindTermIndex::TermCG> termDef;
            nindTermIndex.getTermDef(ident, termDef);
            //affiche les documents dans lesquels est indexe ce terme
            for (list<NindTermIndex::TermCG>::const_iterator it1 = termDef.begin(); 
                 it1 != termDef.end(); it1++) {
                const NindTermIndex::TermCG &termCG = (*it1);
                cout<<BOLD<<"["<<ident<<"] "<<nindIndexTest.getCgStr(termCG.cg)<<OFF,
                cout<<" "<<termCG.frequency<<" fois dans ";
                const list<NindTermIndex::Document> &documents = termCG.documents;
                for (list<NindTermIndex::Document>::const_iterator it2 = documents.begin(); 
                     it2 != documents.end(); it2++) {
                    const NindTermIndex::Document &doc = (*it2);
                    cout<<doc.ident<<"("<<doc.frequency<<") ";
                }
                cout<<endl;
            }
            cout<<BLUE<<"Entrez le n° de doc à afficher : "<<OFF;
            cin.getline(str, 80, '\n');
            const unsigned int noDoc = atoi(str);
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localDef;
            nindLocalIndex.getLocalDef(noDoc, localDef);
            cerr<<"localDef.size()="<<localDef.size()<<endl;
            for (list<NindLocalIndex::Term>::const_iterator it3 = localDef.begin();
                    it3 != localDef.end(); it3++) {
                const NindLocalIndex::Term &term = (*it3);
                if (term.term == ident) {
                    cout<<nindIndexTest.getCgStr(term.cg)<<"<";
                    const list<NindLocalIndex::Localisation> &localisation = term.localisation;
                    string sep = "";
                    for (list<NindLocalIndex::Localisation>::const_iterator it4 = localisation.begin();
                            it4 != localisation.end(); it4++) {
                        cout<<sep<<(*it4).position<<"("<<(*it4).length<<")";
                        sep = " ";
                    }
                    cout<<"> ";
                }
            }
            cout<<endl;
        }
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
