//
// C++ Implementation: NindLexicon_testLimites
//
// Description: un test pour remplir le lexique aux limites et faire differentes mesures.
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: 2014-2015 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
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
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindLexicon."<<endl;
    cout<<"Charge un lexique vide avec un fichier de mots simples"<<endl;
    cout<<"et les combine pour en faire des mots composés."<<endl;
    cout<<"Le fichier lexique s'appelle limites.lexicon."<<endl;
    cout<<"Permet de mesurer l'occupation mémoire du lexique."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier de mots simples>"<<endl;
    cout<<"ex :   "<<arg0<<" frelemmes.csv"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 128
#define LEXICON_NAME "limites.lexicon"
////////////////////////////////////////////////////////////
static void displayChar(const struct NindLexicon::LexiconChar &lexiconChar, const string &title);
static string asciiDate(const time_t date);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string wordsFileName = argv[1];
    if (wordsFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        cout<<"1) permet de mesurer l'occupation mémoire avec un lexique vide"<<endl;
        start = clock();
        //ouvre le lexique en ecrivain
        NindLexicon nindLexicon(LEXICON_NAME, true);
        struct NindLexicon::LexiconChar lexiconChar;
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
        nindLexicon.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "VIDE ");
        cout<<"taper <RC> pour continuer"<<endl;
        string dummy;
        cin>>dummy;

        cout<<"2) charge le lexique"<<endl;
        start = clock();
        //lit le fichier des mots simples
        unsigned int wordsNb = 0;
        list<string> last10;
        char charBuff[LINE_SIZE];
        ifstream wordsFile(wordsFileName.c_str(), ifstream::in);
        if (wordsFile.fail()) throw OpenFileException(wordsFileName);
        //pour memoriser les 10 derniers mots
        while (wordsFile.good()) {
            wordsFile.getline(charBuff, LINE_SIZE);
            wordsNb++;
            list<string> componants;
            componants.push_back(string(charBuff));
            nindLexicon.addWord(componants);
            //memorise
            last10.push_back(string(charBuff));
            //tous les 10 mots simples, fabrique 10 000 mots composes
            if (wordsNb%10 == 0) {
                for (list<string>::const_iterator it1 = last10.begin(); it1 != last10.end(); it1++) {
                    list<string> comps;
                    comps.push_back(*it1);
                    for (list<string>::const_iterator it2 = last10.begin(); it2 != last10.end(); it2++) {
                        comps.push_back(*it2);
                        nindLexicon.addWord(comps); //100 mots de 2
                        for (list<string>::const_iterator it3 = last10.begin(); it3 != last10.end(); it3++) {
                            comps.push_back(*it3);
                            nindLexicon.addWord(comps); //1000 mots de 3
                        }
                    }
                }
                last10.clear();
            }
            if (wordsNb%1000 == 0) { cout<<wordsNb<<"\r"; cout.flush(); }
            if (wordsNb%10000 == 0) {
                end = clock();
                cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
                cout<<cpuTimeUsed<<" secondes"<<endl;
                start = clock();
                //return true;
            }
        }
        cout<<wordsNb<<" mots lus"<<endl;
        nindLexicon.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "PLEIN");
        cout<<"taper <RC> pour continuer"<<endl;
        cin>>dummy;
        
        return true;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static void displayChar(const struct NindLexicon::LexiconChar &lexiconChar, const string &title)
{
    cout<<title<<" "<<lexiconChar.isOk<<" <"<<lexiconChar.swNb<<", "<<lexiconChar.cwNb;
    cout<<"> mots, <"<<lexiconChar.wordsNb<<", "<<lexiconChar.identification;
    cout<<">  "<<asciiDate((time_t)lexiconChar.identification)<<endl;
}
////////////////////////////////////////////////////////////
static string asciiDate(const time_t date)
{
    const string ascDate(ctime(&date));
    const string::size_type  nowStop = ascDate.rfind(' ');
    return ascDate.substr(0,nowStop+5);
}
////////////////////////////////////////////////////////////
