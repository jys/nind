//
// C++ Implementation: NindTermIndex_testLecteur
//
// Description: un test pour remplir le lexique et le fichier inverse et faire differentes mesures.
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
#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndexTest.h"
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
    cout<<"Programme de test de NindTermIndex en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump de documents spécifié."<<endl;
    cout<<"(Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2.)"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    
    try {
        //calcule les noms des fichiers lexique et inverse
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        
        //le lexique lecteur
        NindLexicon nindLexicon(lexiconFileName, false);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(termindexFileName, false,  wordsNb, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        unsigned int nbInconnus, nbNok, nbOk = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
            unsigned int noDoc;
            list<NindIndexTest::WordDesc> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            if (string(charBuff).empty()) continue;   //evacue ainsi les lignes vides
            docsNb++;
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            nindIndexTest.getWords(string(charBuff), noDoc, wordsList);
            //prend tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le terme
                list<string> componants;
                nindIndexTest.split((*wordIt).word, componants);
                //la cg
                const unsigned char cg = nindIndexTest.getCgIdent((*wordIt).cg);
                list<NindTermIndex::TermCG> termIndex;
                unsigned int id;
                while (true) {
                    try {
                        //recupere l'id du terme dans le lexique
                        id = nindLexicon.getId(componants);
                        break;
                    }
                    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl;}
                }
                if (id == 0) {
                    nbInconnus +=1;
                    nbNok +=1;
                    continue;
                }
                while (true) {
                    try {
                        //recupere l'index inverse pour ce terme
                        nindTermIndex.getTermIndex(id, termIndex);
                        break;
                    }
                    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl;}
                }
                //si le terme n'existe pas encore, la liste reste vide
                list<NindTermIndex::TermCG>::iterator it1 = termIndex.begin(); 
                while (it1 != termIndex.end()) {
                    if ((*it1).cg == cg) {
                        //c'est la meme cg, on va chercher le doc 
                        list<NindTermIndex::Document> &documents = (*it1).documents;
                        //trouve le doc dans la liste ordonnee
                        list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
                        while (it2 != documents.end()) {
                            //le doc devrait etre dans la liste ordonnee
                            if ((*it2).ident == noDoc) {
                                nbOk +=1;  
                                break;
                            }
                            it2++;
                        }
                        if (it2 == documents.end()) nbNok +=1;
                        break;
                    }
                    it1++;
                }
                if (it1 == termIndex.end()) nbNok +=1;
            }
            cout<<docsNb<<"\r"<<flush;
            //if (docsNb ==100) break;
        }        
        docsFile.close();
        cout<<nbOk<<" occurrences consultées avec succès"<<endl;
        cout<<nbNok<<" occurrences consultées en échec"<<endl;
        cout<<nbInconnus<<" occurrences inconnues du lexique"<<endl;
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;        
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
