//
// C++ Implementation: NindLexicon_test3
//
// Description: un test pour remplir le lexique et faire la mesure de l'occupation memoire.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexiconF.h"
#include "NindLexicon/NindExceptions.h"
#include <string>
#include <list>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"Programme de test de NindLexicon (NindLexiconF)."<<endl;
    cout<<"Teste les versions asymétriques sans retro-lexiques."<<endl;
    cout<<"Charge un lexique vide avec le dump de documents spécifié."<<endl;
    cout<<"Permet de mesurer l'occupation mémoire du lexique."<<endl;
    cout<<"Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2."<<endl;
    cout<<"Teste l'écriture puis la lecture et affiche résultats et mesures."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" fre-theJysBox.fdb-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*64
static void getWords(const string &dumpLine, list<string> &wordsList);
static void split(const string &word, list<string> &simpleWords);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        cout<<"1) permet de mesurer l'occupation mémoire avec un lexique vide"<<endl;
        //le lexique
        NindLexiconF nindLexicon;
        cout<<"taper <RC> pour continuer"<<endl;
        string dummy;
        cin>>dummy;

        cout<<"2) charge le lexique"<<endl;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
        //while (!docsFile.eof()) {
            list<string> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            docsNb++;
            //if (docsFile.fail()) throw FormatFileException(docsFileName);
            getWords(string(charBuff), wordsList);
            //ajoute tous les mots à la suite et dans l'ordre
            for (list<string>::const_iterator wordIt = wordsList.begin(); wordIt != wordsList.end(); wordIt++) {
                list<string> componants;
                split(*wordIt, componants);
                const unsigned int id = nindLexicon.addWord(componants);
            }
        }
        docsFile.close();
        cout<<docsNb<<" documents soumis à l'indexation"<<endl;
        
        cout<<"3) permet de mesurer l'occupation mémoire avec un lexique vide"<<endl;
        cout<<"taper <RC> pour continuer"<<endl;
        cin>>dummy;

        /////////////////////////////////////
        cout<<"4) vérifie l'intégrité"<<endl;
        struct NindLexiconF::LexiconSizes lexiconSizes;
        const bool isOk = nindLexicon.integrityAndCounts(lexiconSizes);
        //integrite
        if (isOk) cout<<"lexique OK ";
        else cout<<"lexique NOK ";
        //nombres de mots
        cout<<lexiconSizes.swNb<<" mots simples, ";
        cout<<lexiconSizes.cwNb<<" mots composés"<<endl;

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
        simpleWords.push_back(word.substr(posDeb, posSep-posDeb));
        if (posSep == string::npos) break;
        posDeb = posSep + 1;
    }
}
////////////////////////////////////////////////////////////


