//
// C++ Implementation: NindIndex_testLecteur
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndex/NindIndexTest.h"
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
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(termindexFileName, false,  wordsNb, identification);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(localindexFileName, false, wordsNb, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        const time_t time = (time_t) identification;
        cout<<"N° de terme max : "<<wordsNb<<" dernière mise à jour : "<<ctime(&time);
        
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
            const unsigned int ident = nindLexicon.getId(componants);
            if (ident == 0) {
                cout<<"INCONNU"<<endl;
                continue;
            }
            //recupere l'index inverse pour ce terme
            list<NindTermIndex::TermCG> termIndex;
            nindTermIndex.getTermIndex(ident, termIndex);
            //affiche les documents dans lesquels est indexe ce terme
            for (list<NindTermIndex::TermCG>::const_iterator it1 = termIndex.begin(); 
                 it1 != termIndex.end(); it1++) {
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
            list<NindLocalIndex::Term> localIndex;
            nindLocalIndex.getLocalIndex(noDoc, localIndex);
            for (list<NindLocalIndex::Term>::const_iterator it3 = localIndex.begin();
                    it3 != localIndex.end(); it3++) {
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
