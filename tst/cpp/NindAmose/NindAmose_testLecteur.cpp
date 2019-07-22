//
// C++ Implementation: NindAmose_testLecteur
//
// Description: un test vejrifier que nind renvoie bien en lecture toutes les informations 
// reçues en ejcriture et seulement celles-là.
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
#include "NindAmose/NindTermAmose.h"
#include "NindAmose/NindLocalAmose.h"
#include "NindAmose/NindLexiconAmose.h"
#include "NindAmose_litTexteAnalysej.h"
#include "NindIndex/NindDate.h"
#include "NindExceptions.h"
#include <glob.h>
#include <string>
#include <list>
#include <iostream>
#include <iomanip> 
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme de test de NindLexiconAmose, NindTermAmose et NindLocalAmose"<<endl;
    cout<<"en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump des documents qui ont servi à l'indexation"<<endl;
    cout<<"avec NindAmose_indexeCorpus."<<endl;
    cout<<"Il est vérifié que "<<endl;
    cout<<"  o chaque mot (simple, composé, entité nommées) est connu du lexique."<<endl;
    cout<<"  o chaque terme est indexé dans le bon fichier."<<endl;
    cout<<"  o chaque occurrence de terme a la bonne localisation."<<endl;
    cout<<endl;
    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichiers dump documents> <racine fichiers>"<<endl;
    cout<<"ex :   "<<arg0<<" \"clef/ger/*.xml.mult.xml.txt\" \"clef/ger\""<<endl;
}
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<3) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string incompleteFileName = argv[2];
    
    try {
        //pour calculer le temps consomme
        clock_t start = clock(), end;
        double cpuTimeUsed;
        /////////////////////////////////////
        //le lexique lecteur
        NindLexiconAmose nindLexiconAmose(incompleteFileName, false);
        const NindIndex::Identification identification = nindLexiconAmose.getIdentification();
        //affiche les identifiants du lexique
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse lecteur
        NindTermAmose nindTermAmose(incompleteFileName, false, identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(incompleteFileName, false, identification);
        unsigned int nbGetLocal = 0;
        unsigned int nbGetTerm = 0;
        unsigned int nbGetLex = 0;
        //lit tous les fichiers
        glob_t globbuf;
        const int err = glob(docsFileName.c_str(), 0, NULL, &globbuf);
        if(err != 0) throw OpenFileException(docsFileName);
        for (size_t i = 0; i < globbuf.gl_pathc; i++) {
            //lit le fichier dump de documents
            NindAmose_litTexteAnalysej nindAmose_litTexteAnalysej(globbuf.gl_pathv[i]);
            unsigned int noDoc;
            while (nindAmose_litTexteAnalysej.documentSuivant(noDoc)) {
                cout<<noDoc<<"\r"<<flush;
                //recupere l'index local du doc             
                list<NindLocalIndex::Term> localDef;
                nindLocalAmose.getLocalDef(noDoc, localDef);
                list<NindLocalIndex::Term>::const_iterator localDefIt = localDef.begin();
                nbGetLocal++;
                //lit tous les termes et leur localisation/taille
                string lemme;
                AmoseTypes type;
                string entitejNommeje;
                unsigned int position, taille;
                while (nindAmose_litTexteAnalysej.motSuivant(lemme, type, entitejNommeje, position, taille)) {
                    //recupere l'id du terme dans le lexique
                    nbGetLex++;
                    const unsigned int id = nindLexiconAmose.getWordId(lemme, type, entitejNommeje);
                    if (id == 0) {
                        cerr<<lemme<<" INCONNU dans "<<nindLexiconAmose.getFileName()<<endl;
                        continue;
                    }
                    //rejcupehre la liste des documents ouh est indexej ce terme
                    nbGetTerm++;
                    list<unsigned int> documentIds;
                    const bool trouvej = nindTermAmose.getDocList(id, documentIds);
                    if (!trouvej) {
                        cerr<<lemme<<" INCONNU dans "<<nindTermAmose.getFileName()<<endl;
                    }
                    continue;
                    //le doc doit estre dans la liste
                    list<unsigned int>::const_iterator itdoc = documentIds.begin();
                    while (itdoc != documentIds.end()) {
                        if (*itdoc == noDoc) break;
                        itdoc++;
                    }
                    if (itdoc == documentIds.end()) {
                        cerr<<lemme<<" INCONNU dans document n° "<<noDoc<<endl;
                        continue;
                    }
                    //vejrifie la validitej de la position
                    const NindLocalIndex::Term &term = (*localDefIt++);
                    const list<NindLocalIndex::Localisation> &localisations = term.localisation;
                    const NindLocalIndex::Localisation &localisation =localisations.front();
                    if (term.term != id || localisation.position != position || localisation.length != taille) { 
                        cerr<<lemme<<" ("<<position<<", "<<taille<<") INCONNU dans document n° "<<noDoc<<endl;
                        throw FormatFileException(docsFileName);
                        continue;
                    }                
                }
            }
        }
        globfree(&globbuf);
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nbGetLex<<" accès à "<<nindLexiconAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbGetTerm<<" accès à "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbGetLocal<<" accès à "<<nindLocalAmose.getFileName()<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////
