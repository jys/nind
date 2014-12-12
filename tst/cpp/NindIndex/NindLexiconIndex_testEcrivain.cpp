//
// C++ Implementation: NindLexiconIndex_testEcrivain
//
// Description: un test pour remplir le lexique et faire differentes mesures.
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
    cout<<"Programme de test de NindLexiconIndex en mode écrivain"<<endl;
    cout<<"avec le dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Si le fichier lexique n'existe pas, charge un lexique vide"<<endl;
    cout<<"Si le fichier lexique existe, charge le lexique avec le fichier lexique."<<endl;
    cout<<"Teste l'écriture puis la lecture et affiche résultats et mesures."<<endl;
    cout<<"Pour mesurer les vrais temps d'écriture et de lecture du lexique,"<<endl;
    cout<<"ce programme utilise le principe de la double pesée."<<endl;
    cout<<"Un nombre premier est préférable pour <taille bloc indirection>."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump> <taille bloc indirection> "<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt 100003"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
void formeLexique(const bool pourDeVrai,
                  const string &docsFileName,
                  NindLexiconIndex &nindLexicon,
                  list<pair<unsigned int, string> > &allWords,
                  unsigned int &docsNb);
void interrogeLexique(const bool pourDeVrai,
                      const list<pair<unsigned int, string> > &allWords,
                      NindLexiconIndex &nindLexicon);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<3) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string lexiconEntryNbStr = argv[2];

    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    try {
        //calcule le nom du fichier lexique
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexiconindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        //le lexique ecrivain
        NindLexiconIndex nindLexicon(lexiconFileName, true, lexiconEntryNb);
        //la correspondance de tous les mots avec leur identifiant
        list<pair<unsigned int, string> > allWords;
        unsigned int docsNb;
        /////////////////////////////////////
        cout<<"1) forme le lexique pour de faux"<<endl;
        start = clock();
        formeLexique(false, docsFileName, nindLexicon, allWords, docsNb);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //affiche les données de l'indexation
        cout<<allWords.size()<<" mots de "<<docsNb<<" documents soumis au lexique en ";
        cout<<cpuTimeUsed<<" secondes"<<endl;
        
        cout<<"2) forme le lexique pour de vrai"<<endl;
        start = clock();
        formeLexique(true, docsFileName, nindLexicon, allWords, docsNb);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //affiche les données de l'indexation
        cout<<allWords.size()<<" mots de "<<docsNb<<" documents soumis au lexique en ";
        cout<<cpuTimeUsed<<" secondes"<<endl;

        /////////////////////////////////////
        cout<<"3) redemande les "<<allWords.size()<<" mots soumis et vérifie leur identifiant pour de faux"<<endl;
        start = clock();
        interrogeLexique(false, allWords, nindLexicon);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"OK en "<<cpuTimeUsed<<" secondes"<<endl;

        cout<<"4) redemande les "<<allWords.size()<<" mots soumis et vérifie leur identifiant pour de vrai"<<endl;
        start = clock();
        interrogeLexique(true, allWords, nindLexicon);
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
void formeLexique(const bool pourDeVrai,
                  const string &docsFileName,
                  NindLexiconIndex &nindLexicon,
                  list<pair<unsigned int, string> > &allWords,
                  unsigned int &docsNb)
{
    //la classe d'utilitaires
    NindIndexTest nindIndexTest;
    //lit le fichier dump de documents
    docsNb = 0;
    allWords.clear();
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
        //prend tous les mots à la suite et dans l'ordre
        for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                wordIt != wordsList.end(); wordIt++) {
            //le mot 
            const string word = (*wordIt).word;
            //le terme
            list<string> componants;
            nindIndexTest.split(word, componants);
            unsigned int id = 0;
            if (pourDeVrai) id = nindLexicon.addWord(componants);
            allWords.push_back(pair<unsigned int, string>(id, word));
        }
        cout<<docsNb<<"\r"<<flush;
    }
    docsFile.close();
}
////////////////////////////////////////////////////////////
void interrogeLexique(const bool pourDeVrai,
                      const list<pair<unsigned int, string> > &allWords,
                      NindLexiconIndex &nindLexicon)
{
    //la classe d'utilitaires
    NindIndexTest nindIndexTest;
    for (list<pair<unsigned int, string> >::const_iterator wordIt = allWords.begin(); 
            wordIt != allWords.end(); wordIt++) {
        list<string> componants;
        nindIndexTest.split(wordIt->second, componants);
        if (pourDeVrai) {
            const unsigned int id = nindLexicon.getId(componants);
            if (id != wordIt->first) throw IntegrityException(wordIt->second);
        }
    }
}
////////////////////////////////////////////////////////////
