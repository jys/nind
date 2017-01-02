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
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndexTest.h"
#include "NindDate.h"
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
#define LINE_SIZE 256
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindLexicon et NindTermIndex en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump du lexique spécifié."<<endl;
    cout<<"($PY/Nind_dumpLexicon.py crée le dump du lexique.)"<<endl;
    cout<<"Le mode timeControl permet des mesures de temps par double pesée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump lexique> [<timeControl>]"<<endl;
    cout<<"ex :   "<<arg0<<" work/FRE.lexiconindex-dumpall.txt"<<endl;
}
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    string timeControlStr = "0";
    if (argc>2) timeControlStr = argv[2];
    const unsigned int timeControl = atoi(timeControlStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique et inverse
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexiconindex";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        //le lexique lecteur
        NindLexiconIndex nindLexicon(lexiconFileName, false);
        const NindIndex::Identification identification = nindLexicon.getIdentification();
        //affiche les identifiants du lexique
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(termindexFileName, false, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;

        //lit le fichier dump de lexique
        unsigned int lineNb = 0;
        unsigned int totalOccurences = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
            unsigned int id;
            string word;
            docsFile>>id>>word;
            if (word == "") continue;
            lineNb++;
            list<string> componants;
            nindIndexTest.split(word, componants);
            //recupere l'id du terme dans le lexique
            if (timeControl < 3) id = nindLexicon.getWordId(componants);
            if (id == 0) throw IntegrityException(word);
            //recupere l'index inverse pour ce terme
            list<NindTermIndex::TermCG> termDef;
            if (timeControl < 2) nindTermIndex.getTermDef(id, termDef);
            for (list<NindTermIndex::TermCG>::const_iterator it1 = termDef.begin(); it1 != termDef.end(); it1++) {
                totalOccurences += (*it1).frequency;
            }
        }        
        docsFile.close();
        end = clock();
        cout<<lineNb<<" mots lus dans le dump du lexique"<<endl;
        cout<<totalOccurences<<" occurences trouvées dans le corpus"<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;        
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
