//
// C++ Implementation: NindIndex_testEcrivain
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux
// et faire differentes mesures.
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
#include "NindIndex_indexe.h"
#include "NindIndexTest.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindIndex en mode écrivain (pour l'indexation)."<<endl;
    cout<<"Le lexique et les fichiers inverse et d'index locaux sont créés à partir"<<endl;
    cout<<"du dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Les fichiers lexique, inverse et d'index locaux doivent être absents."<<endl;
    cout<<"Les termes sont bufferisés avant indexation. La taille du buffer est spécifiée."<<endl;
    cout<<"(une taille de 0 signifie une indexation des termes au fil de l'eau)."<<endl;
    cout<<"Lorsque le buffer est plein, il est indexé puis vidé."<<endl;
    cout<<"Lorsque tous les documents ont été lus, les termes bufferisés sont indexés."<<endl;
    cout<<"Les documents sont indexés au fur et à mesure de leur lecture."<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le lexique,"<<endl;
    cout<<"le fichier inversé et le fichier des index locaux."<<endl;
    cout<<"Le mode timeControl permet des mesures de temps par double pesée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <taille buffer termes> <taille lexique> <taille inverse> <taille locaux> [<timeControl>]"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt 0 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<6) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string bufferSizeStr = argv[2];
    const string lexiconEntryNbStr = argv[3];
    const string termindexEntryNbStr = argv[4];
    const string localindexEntryNbStr = argv[5];
    string timeControlStr = "0";
    if (argc>6) timeControlStr = argv[6];
    
    const unsigned int bufferSize = atoi(bufferSizeStr.c_str());
    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    const unsigned int termindexEntryNb = atoi(termindexEntryNbStr.c_str());
    const unsigned int localindexEntryNb = atoi(localindexEntryNbStr.c_str());
    const unsigned int timeControl = atoi(timeControlStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const size_t pos = docsFileName.find('.');
//        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
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
        //l'acces aux index
        NindIndex_indexe nindIndex_indexe(lexiconFileName, 
                                          termindexFileName, 
                                          localindexFileName,
                                          lexiconEntryNb,
                                          termindexEntryNb,
                                          localindexEntryNb,
                                          bufferSize,
                                          timeControl);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
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
            //la structure d'index locaux se fabrique pour un document complet
            nindIndex_indexe.newDoc(noDoc);
            //la position absolue dans le fichier source du terme precedent
            //(le dump est considere comme fichier source parce que nous n'avons pas les vrais fichiers sources)
            //prend tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le mot 
                const string word = (*wordIt).word;
                //le terme
                list<string> componants;
                nindIndexTest.split(word, componants);
                //la cg, position et taille
                const unsigned char cg = nindIndexTest.getCgIdent((*wordIt).cg);
                const unsigned int pos = (*wordIt).pos;
                const unsigned int size = word.size();
                //indexe le terme
                nindIndex_indexe.indexe(componants, cg, pos, size);
            }
            //ecrit la definition sur le fichier des index locaux
            nindIndex_indexe.newDoc(0);
            cout<<docsNb<<"\r"<<flush;
        }
        nindIndex_indexe.flush();
        end = clock();
        cout<<nindIndex_indexe.lexiconAccessNb()<<" accès / mises à jour sur "<<lexiconFileName<<endl;
        cout<<nindIndex_indexe.termindexAccessNb()<<" mises à jour sur "<<termindexFileName<<endl;
        cout<<nindIndex_indexe.localindexAccessNb()<<" mises à jour sur "<<localindexFileName<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
        docsFile.close();
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
