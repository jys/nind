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
#include "NindIndex/NindLexiconIndex.h"
#include "NindAmose/NindTermAmose.h"
#include "NindAmose/NindLocalAmose.h"
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
    cout<<"L'utilisateur indique le terme cherché et sa catégorie grammaticale."<<endl;
    cout<<"Ce peut être un terme simple ou un terme composé."<<endl;
    cout<<"Un terme composé a la forme dièsée (ex \"kyrielle#outil#informatique\")."<<endl;
    cout<<"Valeurs des catégories grammaticales :ADJ, ADV, CONJ, DET, DETERMINEUR,"<<endl;
    cout<<"    DIVERS, DIVERS_DATE, EXCLAMATION, INTERJ, NC, NOMBRE, NP, PART,"<<endl;
    cout<<"    PONCTU, PREP, PRON, V, DIVERS_PARTICULE, CLASS, AFFIX"<<endl;
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
        NindTermAmose nindTermAmose(termindexFileName, wordsNb, identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(localindexFileName, wordsNb, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        const time_t time = (time_t) identification;
        cout<<"N° de terme max : "<<wordsNb<<" dernière mise à jour : "<<ctime(&time);
        
        while (true) {
            char str [80];
            cout<<endl<<BLUE<<"Entrez le terme à rechercher (ex \"bleu\") : "<<OFF;
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
            //demande la categorie grammaticale
            unsigned int cg;
            while (true) {
                cout<<BLUE<<"Entrez la catégorie grammaticale (ex \"ADJ\") : "<<OFF;
                cin.getline(str, 80, '\n');
                const string cgStr = string(str);
                try {
                    cg = nindIndexTest.getCgIdent(cgStr);
                    break;
                }
                catch (...) { 
                    cout<<"Valeurs des catégories grammaticales :ADJ, ADV, CONJ, DET, DETERMINEUR,"<<endl;
                    cout<<"    DIVERS, DIVERS_DATE, EXCLAMATION, INTERJ, NC, NOMBRE, NP, PART,"<<endl;
                    cout<<"    PONCTU, PREP, PRON, V, DIVERS_PARTICULE, CLASS, AFFIX"<<endl;
                }
            }
            //recupere le nombre d'occurences pour ce terme + CG
            const unsigned int nbOcc = nindTermAmose.getTermFreq(ident, cg);
            cout<<nbOcc<<" occurences trouvées"<<endl;
            //recupere le nombre de documents pour ce terme + CG
            const unsigned int nbDocs = nindTermAmose.getDocFreq(ident, cg);
            cout<<nbDocs<<" documents trouvés"<<endl;
            //recupere l'index inverse pour ce terme + CG
            list<NindTermIndex::Document> documents;
            nindTermAmose.getDocumentsList(ident, cg, documents);
            for (list<NindTermIndex::Document>::const_iterator it2 = documents.begin(); 
                    it2 != documents.end(); it2++) {
                const NindTermIndex::Document &doc = (*it2);
                cout<<doc.ident<<"("<<doc.frequency<<") ";
            }
            cout<<endl;
            cout<<BLUE<<"Entrez le n° de doc à afficher : "<<OFF;
            cin.getline(str, 80, '\n');
            const unsigned int noDoc = atoi(str);
            //recupere la taille du doc en nombre d'occurences de termes
            const unsigned int nbTerms = nindLocalAmose.getDocLength(noDoc);
            cout<<nbTerms<<" termes + CG indexés dans ce document"<<endl;
            //recupere les termes + CG uniques indexes dans ce document
            set<NindLocalAmose::TermCg> uniqueTermsSet;
            nindLocalAmose.getUniqueTerms(noDoc, uniqueTermsSet);
            cout<<uniqueTermsSet.size()<<" termes + CG uniques indexés dans ce document"<<endl;
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localIndex;
            nindLocalAmose.getLocalIndex(noDoc, localIndex);
            for (list<NindLocalIndex::Term>::const_iterator it3 = localIndex.begin();
                    it3 != localIndex.end(); it3++) {
                const NindLocalIndex::Term &term = (*it3);
                if (term.term == ident && term.cg == cg) {
                    cout<<"<";
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
