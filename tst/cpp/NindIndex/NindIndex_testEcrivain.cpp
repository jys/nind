//
// C++ Implantation: NindIndex_testEcrivain
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux
// et faire differentes mesures.
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
#include "NindIndex_indexe.h"
#include "NindIndex_litDumpS2.h"
#include "NindDate.h"
#include "NindFichiers.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <iomanip> 
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
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
        const string incompleteFileName = docsFileName.substr(0, docsFileName.find('.'));
        //vejrifie que le systehme de fichiers est cohejrent
        if (!NindFichiers::fichiersCohejrents(incompleteFileName, true)) {
            cout<<"Des anciens fichiers existent !"<<endl;
            cout<<"Veuillez les effacer par la commande : rm "<<incompleteFileName + ".nind*"<<endl;
            return false;
        }
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        /////////////////////////////////////
        cout<<"Forme le lexique, le fichier des termes et le fichier des index locaux avec "<<docsFileName<<endl;
        start = clock();
        //l'acces aux index
        NindIndex_indexe nindIndex_indexe(incompleteFileName, 
                                          lexiconEntryNb,
                                          termindexEntryNb,
                                          localindexEntryNb,
                                          bufferSize,
                                          timeControl);
        //lit le fichier dump de documents
        NindIndex_litDumpS2 nindIndex_litDumpS2(docsFileName);
        unsigned int docsNb = 0;
        unsigned int noDocAnt, noDocFb;
        while (nindIndex_litDumpS2.documentSuivant(noDocAnt, noDocFb)) {
            docsNb++;
            cout<<docsNb<<"\r"<<flush;
            //la structure d'index locaux se fabrique pour un document complet
            nindIndex_indexe.newDoc(noDocFb);
            //lit tous les termes et leur localisation/taille
            list<string> composants;
            unsigned char cg;
            list<pair<unsigned int, unsigned int> > localisation;
            while (nindIndex_litDumpS2.motSuivant(composants, cg, localisation)) {
                //indexe le terme
                nindIndex_indexe.indexe(composants, cg, localisation);
            }
            //ejcrit la definition sur le fichier des index locaux
            nindIndex_indexe.newDoc(0);
        }
        nindIndex_indexe.flush();
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.lexiconAccessNb()<<" accès / mises à jour sur "<<nindIndex_indexe.getLexiconFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.termindexAccessNb()<<" mises à jour sur "<<nindIndex_indexe.getTermFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.localindexAccessNb()<<" mises à jour sur "<<nindIndex_indexe.getLocalFileName()<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
