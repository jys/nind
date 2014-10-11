//
// C++ Implementation: NindLexicon_testLecteur
//
// Description: un test pour lire le lexique fichier et faire differentes mesures.
// idem NindLexicon_test2 mais avec fichier lexique
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
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
    cout<<"Programme de test de NindLexicon en mode lecteur (pour la recherche)."<<endl;
    cout<<"Charge un lexique avec le fichier lexique puis teste tous les identifiants"<<endl;
    cout<<"à partir du dump de documents originel. Affiche les mesures."<<endl;
    cout<<"(Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2.)"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" fre-theJysBox.fdb-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
static void getWords(const string &dumpLine, list<string> &wordsList);
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //calcule le nom du fichier lexique
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        //le lexique lecteur
        NindLexicon nindLexicon(lexiconFileName, false);
        /////////////////////////////////////
        cout<<"1) vérifie l'intégrité du lexique"<<endl;
        start = clock();
        struct NindLexicon::LexiconChar lexiconSizes;
        const bool isOk = nindLexicon.integrityAndCounts(lexiconSizes);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //integrite
        if (isOk) cout<<"lexique OK ";
        else cout<<"lexique NOK ";
        //nombres de mots
        cout<<lexiconSizes.swNb<<" mots simples, ";
        cout<<lexiconSizes.cwNb<<" mots composés"<<endl;
        cout<<cpuTimeUsed<<" secondes"<<endl;

        /////////////////////////////////////
        cout<<"2) forme la référence d'interrogation"<<endl;
        start = clock();
        //nindLexicon.dump(std::cerr);
        //la correspondance de tous les mots avec leur identifiant
        list<pair<unsigned int, string> > allWords;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
        //while (!docsFile.eof()) {
            list<string> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            if (string(charBuff).empty()) continue;   //evacue ainsi les lignes vides
            docsNb++;
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            getWords(string(charBuff), wordsList);
            //for(list<string>::const_iterator it = wordsList.begin(); it != wordsList.end(); it++) cerr<<(*it)<<endl;
            //ajoute tous les mots à la suite et dans l'ordre
            for (list<string>::const_iterator wordIt = wordsList.begin(); wordIt != wordsList.end(); wordIt++) {
                list<string> componants;
                split(*wordIt, componants);
                const unsigned int id = nindLexicon.getId(componants);
                allWords.push_back(pair<unsigned int, string>(id, *wordIt));
            }
        }
        docsFile.close();
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //affiche les données de l'indexation
        cout<<allWords.size()<<" mots de "<<docsNb<<" documents répertoriés "<<endl;
        //nindLexicon.dump(std::cerr);

        /////////////////////////////////////
        cout<<"3) demande les "<<allWords.size()<<" mots et vérifie leur identifiant"<<endl;
        start = clock();
        for (list<pair<unsigned int, string> >::const_iterator wordIt = allWords.begin(); wordIt != allWords.end(); wordIt++) {
            list<string> componants;
            split(wordIt->second, componants);
            const unsigned int id = nindLexicon.getId(componants);
            if (id != wordIt->first) throw IntegrityException(wordIt->second);
        }
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"OK en "<<cpuTimeUsed<<" secondes"<<endl;

        return true;
    }
    catch (LexiconException &exc) {cerr<<"EXCEPTION :"<<exc.m_word<<" "<<exc.what()<<endl; return false;}
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static void getWords(const string &dumpLine, list<string> &wordsList)
{
    //cerr<<dumpLine<<endl;
    wordsList.clear();
    //1111005 <=> 1 len=12  ::  famille (NC), famille#heureux (NC), heureux (ADJ), se_ressembler (V), ..., façon (NC)
    if (dumpLine.empty()) return;   //evacue ainsi les lignes vides
    size_t posDeb = dumpLine.find("::  ", 0);
    if (posDeb == string::npos) throw FormatFileException();
    posDeb += 4;
    while (true) {
        size_t posSep = dumpLine.find(' ', posDeb);
        wordsList.push_back(dumpLine.substr(posDeb, posSep-posDeb));
        posDeb = posSep + 1;
        posSep = dumpLine.find("), ", posDeb);
        if (posSep == string::npos) break;
        posDeb = posSep + 3;
    }
}
////////////////////////////////////////////////////////////
//decoupe le mot sur les '#' et retourne une liste ordonnee de mots simples
static void split(const string &word, list<string> &simpleWords)
{
    simpleWords.clear();
    size_t posDeb = 0;
    while (true) {
        const size_t posSep = word.find('#', posDeb);
        if (posSep == string::npos) {
            simpleWords.push_back(word.substr(posDeb));
            break;
        }
        simpleWords.push_back(word.substr(posDeb, posSep-posDeb));
        if (posSep == string::npos) break;
        posDeb = posSep + 1;
    }
}
////////////////////////////////////////////////////////////


