//
// C++ Implementation: NindLexicon_testInter
//
// Description: un test pour tester les relations lexique ecrivain lexique lecteur.
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
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
#include <stdio.h>
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"Programme de test de NindLexicon en mode écrivain et lecteur."<<endl;
    cout<<"Le but de ce test est de vérifier qu'un lexique lecteur se met"<<endl;
    cout<<"à jour des modifications faites par le lexique écrivain."<<endl;
    cout<<"Le test démarre sans fichier lexique, lequel est créé à partir"<<endl;
    cout<<"du dump de documents spécifié."<<endl;
    cout<<"(Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2.)"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" fre-theJysBox.fdb-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*128
static void displayChar(const struct NindLexicon::LexiconChar &lexiconChar,
                        const string &title);
static string asciiDate(const time_t date);
static void fillWords(NindLexicon &nindLexiconEcrivain,
                      list<string>::const_iterator &wordsToInsertIt,
                      unsigned int count);
static void getWords(const string &dumpLine,
                     list<string> &wordsList);
static void split(const string &word,
                  list<string> &simpleWords);
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

        /////////////////////////////////////
        cout<<"1) vérifie que le fichier "<<lexiconFileName<<" est absent"<<endl;
        FILE *file =  fopen(lexiconFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<lexiconFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<lexiconFileName<<endl;
            return false;
        }
        cout<<"OK"<<endl;
        
        /////////////////////////////////////
        cout<<"2) établit la liste des futures insertions et interrogations à partir de "<<docsFileName<<endl;
        //une liste dans l'ordre pour les insertions et un set pour les interrogations
        list<string> wordsToInsert;   //pour l'ecrivain
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
            list<string> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            if (string(charBuff).empty()) continue;   //evacue ainsi les lignes vides
            docsNb++;
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            getWords(string(charBuff), wordsList);
            for (list<string>::const_iterator wordIt = wordsList.begin(); wordIt != wordsList.end(); wordIt++) {
                wordsToInsert.push_back(*wordIt);
            }
        }
        docsFile.close();
        cout<<docsNb<<" documents lus, "<<wordsToInsert.size()<<" mots"<<endl;
        
        /////////////////////////////////////
        cout<<"3) crée le lexique écrivain puis le lexique lecteur"<<endl;
        NindLexicon nindLexiconEcrivain(lexiconFileName, true);
        NindLexicon nindLexiconLecteur(lexiconFileName, false);
        //verifie que c'est ok
        struct NindLexicon::LexiconChar lexiconChar;
        bool isOk = nindLexiconEcrivain.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "ECRIVAIN");
        if (!isOk) return false;
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;

        /////////////////////////////////////
        cout<<"4) entre le 1/3 des mots dans le lexique"<<endl;
        list<string>::const_iterator  wordsToInsertIt = wordsToInsert.begin();
        unsigned int count = wordsToInsert.size()/3;
        fillWords(nindLexiconEcrivain, wordsToInsertIt, count);
        //verifie que c'est ok
        isOk = nindLexiconEcrivain.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "ECRIVAIN");
        if (!isOk) return false;
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;

        /////////////////////////////////////
        cout<<"5) attend 10s et teste le lexique lecteur, avant accès puis après accès"<<endl;
        sleep(10);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        list<string> componants;
        componants.push_back("toto");
        unsigned int id = nindLexiconLecteur.getId(componants);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        
        /////////////////////////////////////
        cout<<"6) entre les 2/3 des mots dans le lexique"<<endl;
        count = wordsToInsert.size()/3;
        fillWords(nindLexiconEcrivain, wordsToInsertIt, count);
        //verifie que c'est ok
        isOk = nindLexiconEcrivain.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "ECRIVAIN");
        if (!isOk) return false;
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        
        /////////////////////////////////////
        cout<<"7) attend 5s et teste le lexique lecteur, avant accès puis après accès"<<endl;
        sleep(5);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        id = nindLexiconLecteur.getId(componants);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;

        /////////////////////////////////////
        cout<<"8) attend 5s et teste le lexique lecteur, avant accès puis après accès"<<endl;
        sleep(5);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        id = nindLexiconLecteur.getId(componants);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        
        /////////////////////////////////////
        cout<<"9) entre le reste des mots dans le lexique"<<endl;
        count = wordsToInsert.size() - count;
        fillWords(nindLexiconEcrivain, wordsToInsertIt, count);
        //verifie que c'est ok
        isOk = nindLexiconEcrivain.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "ECRIVAIN");
        if (!isOk) return false;
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;

        /////////////////////////////////////
        cout<<"10) attend 10s et teste le lexique lecteur, avant accès puis après accès"<<endl;
        sleep(10);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;
        id = nindLexiconLecteur.getId(componants);
        isOk = nindLexiconLecteur.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "LECTEUR ");
        if (!isOk) return false;

        return true;
     }
    catch (LexiconException &exc) {cerr<<"EXCEPTION :"<<exc.m_word<<" "<<exc.what()<<endl; return false;}
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
static void fillWords(NindLexicon &nindLexiconEcrivain,
                      list<string>::const_iterator &wordsToInsertIt,
                      unsigned int count)
{
    while (count) {
        list<string> componants;
        split(*wordsToInsertIt++, componants);
        count--;
        const unsigned int id = nindLexiconEcrivain.addWord(componants);
    }
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


