//
// C++ Implementation: NindIndex_testLecteur
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux.
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
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndex_indexe.h"
#include "NindIndex_litDumpS2.h"
#include "NindDate.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
#include <iomanip> 
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme de test de NindLexicon, NindTermIndex et NindLocalIndex en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Une indication de complexité est donnée en sommant toutes les longueurs"<<endl;
    cout<<"de listes de documents utilisées."<<endl;
    cout<<"Le mode timeControl permet des mesures de temps par double pesée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents> [<timeControl>]"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define RED "\033[1;31m"
#define BLA "\033[0m"
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
        //calcule les noms des fichiers lexique, de termes et index locaux
        const string incompleteFileName = docsFileName.substr(0, docsFileName.find('.'));
        //pour calculer le temps consomme
        clock_t start = clock(), end;
        double cpuTimeUsed;
        
        //le lexique lecteur
        NindLexiconIndex nindLexicon(incompleteFileName, false);
        const NindIndex::Identification identification = nindLexicon.getIdentification();
        //affiche les identifiants du lexique
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(incompleteFileName, false, identification, 0);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(incompleteFileName, false, identification);
        //lit le fichier dump de documents
        NindIndex_litDumpS2 nindIndex_litDumpS2(docsFileName);
        unsigned int docsNb = 0;
        unsigned int nbInconnus = 0, nbTermNok = 0, nbTermOk = 0, nbLocalNok = 0, nbLocalOk = 0;
        unsigned int complexite = 0;
        unsigned int noDocAnt, noDocFb;
        while (nindIndex_litDumpS2.documentSuivant(noDocAnt, noDocFb)) {
            docsNb++;
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localDef;
            if (timeControl < 1) nindLocalIndex.getLocalDef(noDocFb, localDef);
            list<NindLocalIndex::Term>::const_iterator localDefIt = localDef.begin();
            //lit tous les termes et leur localisation/taille
            list<string> composants;
            unsigned char cg;
            list<pair<unsigned int, unsigned int> > localisation;
            while (nindIndex_litDumpS2.motSuivant(composants, cg, localisation)) {
                unsigned int id = 0;
                //recupere l'id du terme dans le lexique
                if (timeControl < 3) id = nindLexicon.getWordId(composants);
                if (id == 0) {
                    nbInconnus +=1;
                    nbTermNok +=1;
                    nbLocalNok +=1;
                    continue;
                }
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termDef;
                if (timeControl < 2) nindTermIndex.getTermDef(id, termDef);
                //si le terme n'existe pas encore, la liste reste vide
                if (NindIndex_indexe::trouveDoc(noDocFb, cg, termDef)) nbTermOk +=1;
                else nbTermNok +=1;
                //calcul complexite
                for (list<NindTermIndex::TermCG>::const_iterator it1 = termDef.begin(); it1 != termDef.end(); it1++) {
                        complexite += (*it1).documents.size();
                }
                //verifie l'index local
                const NindLocalIndex::Term &term = (*localDefIt++);
                if (NindIndex_indexe::trouveTerme(id, cg, localisation, term)) nbLocalOk +=1;
                else nbLocalNok +=1;
            }
            cout<<docsNb<<"\r"<<flush;
            //if (docsNb ==100) break;
        }
        cout<<RED<<setw(8)<<setfill(' ')<<nbInconnus<<BLA<<" occurrences inconnues du lexique"<<endl;
        cout<<setw(8)<<setfill(' ')<<complexite<<" = complexité dans les termes"<<endl;
        cout<<setw(8)<<setfill(' ')<<nbTermOk<<" occurrences consultées avec succès dans "<<nindTermIndex.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbTermNok<<BLA<<" occurrences consultées en échec dans "<<nindTermIndex.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbLocalOk<<" occurrences consultées avec succès dans "<<nindLocalIndex.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbLocalNok<<BLA<<" occurrences consultées en échec dans "<<nindLocalIndex.getFileName()<<endl;
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;        
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
