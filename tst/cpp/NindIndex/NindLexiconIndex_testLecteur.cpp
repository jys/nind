//
// C++ Implementation: NindLexiconIndex_testLecteur
//
// Description: un test pour lire le lexique fichier et faire differentes mesures.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindIndex/NindLexiconIndex.h"
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
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindLexicon en mode lecteur (pour la recherche)."<<endl;
    cout<<"Charge un lexique avec le fichier lexique puis teste tous les identifiants"<<endl;
    cout<<"à partir du dump de documents originel. Affiche les mesures."<<endl;
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
        //calcule le nom du fichier lexique
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexiconindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        //le lexique lecteur
        NindLexiconIndex nindLexicon(lexiconFileName, false);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        /////////////////////////////////////
        cout<<"1) forme la référence d'interrogation"<<endl;
        start = clock();
        //la correspondance de tous les mots avec leur identifiant
        list<pair<unsigned int, string> > allWords;
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
            //ajoute tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le mot 
                const string word = (*wordIt).word;
                //le terme
                list<string> componants;
                nindIndexTest.split(word, componants);
                const unsigned int id = nindLexicon.getId(componants);
                if (id == 0) throw IntegrityException(word);
                allWords.push_back(pair<unsigned int, string>(id, word));
            }
            cout<<docsNb<<"\r"<<flush;
        }
        docsFile.close();
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //affiche les données de l'indexation
        cout<<allWords.size()<<" mots de "<<docsNb<<" documents répertoriés "<<endl;
        //nindLexicon.dump(std::cerr);

        /////////////////////////////////////
        cout<<"2) demande les "<<allWords.size()<<" mots et vérifie leur identifiant"<<endl;
        start = clock();
        for (list<pair<unsigned int, string> >::const_iterator wordIt = allWords.begin(); 
             wordIt != allWords.end(); wordIt++) {
            list<string> componants;
            nindIndexTest.split(wordIt->second, componants);
            const unsigned int id = nindLexicon.getId(componants);
            if (id != wordIt->first) throw IntegrityException(wordIt->second);
        }
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"OK en "<<cpuTimeUsed<<" secondes"<<endl;

        return true;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
