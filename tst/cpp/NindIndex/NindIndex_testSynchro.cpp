//
// C++ Implementation: NindIndex_testSynchro
//
// Description: un test pour verifier que les lecteurs se synchronisent bien sur l'ecrivain.
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
    cout<<"Programme de test de la synchronisation des lecteurs de NindIndex sur"<<endl;
    cout<<"l'écrivain."<<endl;
    cout<<"Le corpus est un fichier texte avec une ligne par document."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Les fichiers lexique, de termes et d'index locaux doivent être absents."<<endl;
    cout<<"À chaque écriture sur le lexique, une lecture est faite pour"<<endl;
    cout<<"vérifier que la synchronisation par le fichier physique fonctionne. "<<endl;
    cout<<"Idem pour le fichier des termes."<<endl;
    cout<<"Idem pour le fichier des index locaux."<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour chaque fichier"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <taille lexique> <taille termes> <taille locaux>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
#define RED "\033[1;31m"
#define BLA "\033[0m"
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<5) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string lexiconEntryNbStr = argv[2];
    const string termindexEntryNbStr = argv[3];
    const string localindexEntryNbStr = argv[4];
    
    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    const unsigned int termindexEntryNb = atoi(termindexEntryNbStr.c_str());
    const unsigned int localindexEntryNb = atoi(localindexEntryNbStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique, de termes et index locaux
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
                                          0,
                                          0);
        //Initialise les lecteurs
        //le lexique lecteur
        NindLexiconIndex nindLexicon(incompleteFileName, false);
        //l'identification
        const NindIndex::Identification identification = nindLexicon.getIdentification();
        //le fichier des termes lecteur
        NindTermIndex nindTermIndex(incompleteFileName, false, identification, 0);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(incompleteFileName, false, identification);
        //lit le fichier dump de documents
        NindIndex_litDumpS2 nindIndex_litDumpS2(docsFileName);
        unsigned int nbInconnus = 0, nbTermNok = 0, nbTermOk = 0, nbLocalNok = 0, nbLocalOk = 0;
        unsigned int docsNb = 0;
        unsigned int noDocAnt, noDocFb;
        while (nindIndex_litDumpS2.documentSuivant(noDocAnt, noDocFb)) {
            docsNb++;
            unsigned int termsNb = 0;
            cout<<docsNb<<"\r"<<flush;
            //la structure d'index locaux se fabrique pour un document complet
            nindIndex_indexe.newDoc(noDocFb);
            //lit tous les termes et leur localisation/taille
            list<string> composants;
            unsigned char cg;
            list<pair<unsigned int, unsigned int> > localisation;
            while (nindIndex_litDumpS2.motSuivant(composants, cg, localisation)) {
                termsNb++;
                //indexe le terme
                nindIndex_indexe.indexe(composants, cg, localisation);
                //vejrifie que tout est bien ejcrit et immejdiatement disponible
                unsigned int id = nindLexicon.getWordId(composants);
                if (id == 0) {
                    nbInconnus += 1;
                    continue;
                }
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termDef;
                nindTermIndex.getTermDef(id, termDef);
                if (termDef.size() == 0) cerr<<"termDef.size()="<<termDef.size()<<endl;
                //si le terme n'existe pas encore, la liste reste vide
                if (NindIndex_indexe::trouveDoc(noDocFb, cg, termDef)) nbTermOk++;
                else {
                    nbTermNok++;
                    cerr<<"docsNb= "<<docsNb<<" noDocFb="<<noDocFb<<" noDocAnt="<<noDocAnt<<" id="<<id;
                    cerr<<" composants.size()="<<composants.size()<<"composants.front()="<< composants.front()<<endl;
                    throw IntegrityException("Fin par les termes");
                }
            }
            //ejcrit la definition sur le fichier des index locaux
            nindIndex_indexe.newDoc(0);
            //vejrifie que tout est bien ejcrit et immejdiatement disponible
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localDef;
            nindLocalIndex.getLocalDef(noDocFb, localDef);
            if (localDef.size() == termsNb) nbLocalOk++;
            else {
                nbLocalNok++;
                cerr<<"docsNb= "<<docsNb<<" noDocFb="<<noDocFb<<" noDocAnt="<<noDocAnt;
                cerr<<" localDef.size()="<<localDef.size()<<" termsNb="<<termsNb<<endl;
                throw IntegrityException("Fin par les docs");
            }
        }
        nindIndex_indexe.flush();
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.lexiconAccessNb()<<" accès / mises à jour sur "<<nindIndex_indexe.getLexiconFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.termindexAccessNb()<<" mises à jour sur "<<nindIndex_indexe.getTermFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nindIndex_indexe.localindexAccessNb()<<" mises à jour sur "<<nindIndex_indexe.getLocalFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbInconnus<<BLA<<" occurrences inconnues du lexique"<<endl;
        cout<<setw(8)<<setfill(' ')<<nbTermOk<<" occurrences consultées avec succès dans "<<nindIndex_indexe.getTermFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbTermNok<<BLA<<" occurrences consultées en échec dans "<<nindIndex_indexe.getTermFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbLocalOk<<" occurrences consultées avec succès dans "<<nindIndex_indexe.getLocalFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbLocalNok<<BLA<<" occurrences consultées en échec dans "<<nindIndex_indexe.getLocalFileName()<<endl;

        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
