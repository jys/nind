//
// C++ Implementation: NindLexicon_test4
//
// Description: un test pour comparer les mots du lexique a mots composes et du lexique sans mot compose.
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexiconA.h"
#include "NindLexicon/NindLexiconH.h"
#include "NindLexicon/NindExceptions.h"
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"Programme pour trouver l'écart entre les mots du lexique"<<endl;
    cout<<"avec gestion des mots composés et du lexique sans gestion"<<endl;
    cout<<"des mots composés."<<endl;
    cout<<"Charge un lexique vide avec le dump de documents spécifié."<<endl;
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
        cout<<"1) forme le lexique avec gestion des mots composés"<<endl;
        //le lexique
        NindLexiconA nindLexiconA;
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
                const unsigned int id = nindLexiconA.addWord(componants);
            }
        }
        docsFile.close();
        cout<<docsNb<<" documents lus"<<endl;
        //le dumpe
        unsigned int id = 0;
        set<string> lexiqueCSet;
        while (true) {
            list<string> componants;
            nindLexiconA.getWord(++id, componants);
            if (componants.size() == 0) break;
            string word;
            string sep = "";
            for (list<string>::const_iterator it = componants.begin(); it != componants.end(); it++){
                word += sep + (*it);
                sep = "#";
            }
            lexiqueCSet.insert(word);
            //cerr<<word<<endl;
        }
        cout<<lexiqueCSet.size()<<" mots dans le lexique"<<endl;

        cout<<"2) forme le lexique sans gestion des mots composés"<<endl;
        //le lexique
        NindLexiconH nindLexiconH;
        //lit le fichier dump de documents
        docsNb = 0;
        docsFile.open(docsFileName.c_str(), ifstream::in);
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
                const unsigned int id = nindLexiconH.addWord(*wordIt);
            }
        }
        docsFile.close();
        cout<<docsNb<<" documents lus"<<endl;
        //le dumpe
        id = 0;
        set<string> lexiqueHSet;
        while (true) {
            const string word = nindLexiconH.getWord(++id);
            if (word == "") break;
            lexiqueHSet.insert(word);
            //cerr<<word<<endl;
        }
        cout<<lexiqueHSet.size()<<" mots dans le lexique"<<endl;

        set<string> diff1 = lexiqueCSet;
        for (set<string>::const_iterator ith = lexiqueHSet.begin(); ith != lexiqueHSet.end(); ith++) {
            const set<string>::const_iterator itc = diff1.find(*ith);
            if (itc != diff1.end()) diff1.erase(itc);
        }
        cout<<diff1.size()<<" mots avec MC et pas sans MC"<<endl;
        
        set<string> diff2 = lexiqueHSet;
        for (set<string>::const_iterator itc = lexiqueCSet.begin(); itc != lexiqueCSet.end(); itc++) {
            const set<string>::const_iterator ith = diff2.find(*itc);
            if (ith != diff2.end()) diff2.erase(ith);
        }
        cout<<diff2.size()<<" mots sans MC et pas avec MC"<<endl;
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


