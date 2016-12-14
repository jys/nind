//
// C++ Implementation: NindAmose_testLecteur
//
// Description: un test vejrifier que nind renvoie bien en lecture toutes les informations 
// reçues en ejcriture et seulement celles-là.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: 2014-2016 LATEJCON. See LICENCE.md file that comes with this distribution
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
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <iomanip> 
#include <fstream>
#include <sstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme de test de NindLexiconAmose, NindTermAmose et NindLocalAmose"<<endl;
    cout<<"en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump de documents qui a servi à l'indexation"<<endl;
    cout<<"avec NindAmose_indexeCorpus."<<endl;
    cout<<"Il est vérifié que "<<endl;
    cout<<"  o chaque mot (simple, composé, entité nommées) est connu du lexique."<<endl;
    cout<<"  o chaque terme est indexé dans le bon fichier."<<endl;
    cout<<"  o chaque occurrence de terme a la bonne localisation."<<endl;
    cout<<endl;
    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" sample_fre.xml.mult.xml.txt"<<endl;
}
////////////////////////////////////////////////////////////
static string date(const unsigned int identification);
static void analyzeWord(const string &word, 
                        string &lemma, 
                        AmoseTypes &type, 
                        string &entitejNommeje);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    
    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const string incompleteFileName = docsFileName.substr(0, docsFileName.find('.'));
        const string lexiconindexFileName = incompleteFileName + ".lexiconindex";
        const string retrolexiconindexFileName = incompleteFileName + ".retrolexiconindex";
        const string termindexFileName = incompleteFileName + ".termindex";
        const string localindexFileName = incompleteFileName + ".localindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        /////////////////////////////////////
        //le lexique lecteur
        NindLexiconAmose nindLexicon(lexiconindexFileName, false);
        NindIndex::Identification identification;
        nindLexicon.getIdentification(identification);
        //affiche les identifiants du lexique
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse lecteur
        NindTermAmose nindTermIndex(termindexFileName, false, identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalIndex(localindexFileName, false, identification);
        unsigned int nbGetLocal = 0;
        unsigned int nbGetTerm = 0;
        unsigned int nbGetLex = 0;
        string dumpLine;
        //ouvre le fichier dump qui sert de rejfejrence
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (getline(docsFile, dumpLine)) {
            //lit 1 ligne = 1 document
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            if (dumpLine.empty()) continue;   //evacue ainsi les lignes vides
            stringstream sdumpLine(dumpLine);
            //10170346  Location.LOCATION:Italie 280,6 création 288,8 création_parti 288,19
            unsigned int noDoc;
            string word;
            unsigned int position, taille;
            char comma;
            sdumpLine >> noDoc;
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localDef;
            const bool trouvej = nindLocalIndex.getLocalDef(noDoc, localDef);
            nbGetLocal++;
            if (!trouvej) {
                cerr<<"document n° "<<noDoc<<" INCONNU dans "<<localindexFileName<<endl;
                continue;
            }
            list<NindLocalIndex::Term>::const_iterator localDefIt = localDef.begin();
            //analyse chaque terme
            while (sdumpLine >> word >> position >> comma >> taille) {
                //le terme
                string lemma;
                AmoseTypes type;
                string entitejNommeje;
                analyzeWord(word, lemma, type, entitejNommeje);
                //si le lemme est vide, raf
                if (lemma.empty()) continue;
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                nbGetLex++;
                const unsigned int id = nindLexicon.getWordId(lemma, type, entitejNommeje);
                if (id == 0) {
                    cerr<<word<<" INCONNU dans "<<lexiconindexFileName<<endl;
                    localDefIt++;
                    continue;
                }
                //rejcupehre la liste des documents ouh est indexej ce terme
                nbGetTerm++;
                list<unsigned int> documentIds;
                const bool trouvej = nindTermIndex.getDocList(id, documentIds);
                if (!trouvej) {
                    cerr<<word<<" INCONNU dans "<<termindexFileName<<endl;
                    localDefIt++;
                    continue;
                }
                //le doc doit estre dans la liste
                list<unsigned int>::const_iterator itdoc = documentIds.begin();
                while (itdoc != documentIds.end()) {
                    if (*itdoc == noDoc) break;
                    itdoc++;
                }
                if (itdoc == documentIds.end()) {
                    cerr<<word<<" INCONNU dans document n° "<<noDoc<<endl;
                    localDefIt++;
                    continue;
                }
                //vejrifie la validitej de la position
                const NindLocalIndex::Term &term = (*localDefIt++);
                const list<NindLocalIndex::Localisation> &localisations = term.localisation;
                const NindLocalIndex::Localisation &localisation =localisations.front();
                if (term.term != id || localisation.position != position || localisation.length != taille) { 
                    cerr<<word<<" ("<<position<<", "<<taille<<") INCONNU dans document n° "<<noDoc<<endl;
                    continue;
                }                
            }
            cout<<noDoc<<"\r"<<flush;
            //if (noDoc ==2) break;
       }
        docsFile.close();
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nbGetLex<<" accès à "<<lexiconindexFileName<<endl;
        cout<<setw(8)<<setfill(' ')<<nbGetTerm<<" accès à "<<termindexFileName<<endl;
        cout<<setw(8)<<setfill(' ')<<nbGetLocal<<" accès à "<<localindexFileName<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////
static string date(const unsigned int identification)
{
    const time_t time = (time_t) identification;
    struct tm * timeinfo = localtime (&time);
    char buffer [80];
    strftime (buffer,80,"%Y-%m-%d %X",timeinfo);
    return string(buffer);
}
////////////////////////////////////////////////////////////
static void analyzeWord(const string &word, 
                        string &lemma, 
                        AmoseTypes &type, 
                        string &entitejNommeje)
{
    //si c'est une entitej nommeje, la sejpare en 2
    const size_t pos = word.find(':');
    if (pos != string::npos) {
        entitejNommeje = word.substr(0, pos);
        lemma = word.substr(pos +1);
        type = NAMED_ENTITY;
    }
    else if (word.find('_') != string::npos) {
        lemma = word;
        type = MULTI_TERM;
    }
    else {
        lemma = word;
        type = SIMPLE_TERM;
    }
}
////////////////////////////////////////////////////////////
