//
// C++ Implementation: Nind_testNumbersInLocalIndexes
//
// Description: un test pour comparer les 3 facons de coder les nombres sur le fichier des index locaux
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
#include "NindIndexTest.h"
#include "NindBasics/NindFile.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test des codages de NindFile."<<endl;
    cout<<"Les identifiants de termes obtenus auprès du lexique déjà constitué"<<endl;
    cout<<"sont écrits sous 3 formes dans 3 fichiers différents :"<<endl;
    cout<<"1) identifiants sur 3 octets, 2) identifiants en latecon non signés"<<endl;
    cout<<"3) incrément d'identifiants en latecon signés"<<endl;
    cout<<"Les termes sont fournis par le dump de documents spécifié."<<endl;
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
        //calcule les noms des fichiers lexique et resultats
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        const string int3FileName = "Nind_testNumbersInLocalIndexes.int3";
        const string latUFileName = "Nind_testNumbersInLocalIndexes.latU";
        const string latSFileName = "Nind_testNumbersInLocalIndexes.latS";
        //le lexique lecteur
        NindLexicon nindLexicon(lexiconFileName, false);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        cout<<"Lexique : nb termes="<<wordsNb<<" date="<<identification<<endl;
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        //ouvre les 3 fichiers resultats
        NindFile int3File(int3FileName);          
        NindFile latUFile(latUFileName);          
        NindFile latSFile(latSFileName);         
        bool isOpened = int3File.open("w+b");
        if (!isOpened) throw OpenFileException(int3FileName);
        isOpened = latUFile.open("w+b");
        if (!isOpened) throw OpenFileException(latUFileName);
        isOpened = latSFile.open("w+b");
        if (!isOpened) throw OpenFileException(latSFileName);
        unsigned int nbOccurences = 0;
        unsigned int docsNb = 0;
        //lit le fichier dump de documents
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
            unsigned int noDoc;
            //list<pair<string, string> > wordsList;
            list<NindIndexTest::WordDesc> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            if (string(charBuff).empty()) continue;   //evacue ainsi les lignes vides
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            docsNb++;
            nindIndexTest.getWords(string(charBuff), noDoc, wordsList);
            unsigned int termePrec = 0;
            //prend tous les mots à la suite et dans l'ordre
//            for (list<pair<string, string> >::const_iterator wordIt = wordsList.begin(); wordIt != wordsList.end(); wordIt++) {
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); wordIt != wordsList.end(); wordIt++) {
                //le terme
                //cerr<<(*wordIt).first<<endl;
                list<string> componants;
                nindIndexTest.split((*wordIt).word, componants);
                const unsigned int id = nindLexicon.getId(componants);
                //cerr<<id<<endl;
                int3File.createBuffer(3);
                int3File.putInt3(id);
                int3File.writeBuffer();
                latUFile.createBuffer(5);
                latUFile.putUIntLat(id);
                latUFile.writeBuffer();
                const unsigned int termeIncrement = id - termePrec;
                //cerr<<termeIncrement<<endl;
                latSFile.createBuffer(5);
                latSFile.putSIntLat(termeIncrement);
                latSFile.writeBuffer();
                termePrec = id;
                nbOccurences +=1;
            }
            cout<<docsNb<<"\r"<<flush;
        }
        cout<<nbOccurences<<" occurrences de termes écrites depuis "<<docsNb<<" documents"<<endl;
        cout<<int3File.getFileSize()<<" octets dans "<<int3FileName<<endl;
        cout<<latUFile.getFileSize()<<" octets dans "<<latUFileName<<endl;
        cout<<latSFile.getFileSize()<<" octets dans "<<latSFileName<<endl;
        docsFile.close();
        int3File.close();
        latUFile.close();
        latSFile.close();
        
        //relit les 3 fichiers
        isOpened = int3File.open("rb");
        if (!isOpened) throw OpenFileException(int3FileName);
        isOpened = latUFile.open("rb");
        if (!isOpened) throw OpenFileException(latUFileName);
        isOpened = latSFile.open("rb");
        if (!isOpened) throw OpenFileException(latSFileName);
        unsigned int termePrec = 0;
        docsNb = 0; 
        int3File.setPos(0, SEEK_SET);
        latUFile.setPos(0, SEEK_SET);
        latSFile.setPos(0, SEEK_SET);
        for (unsigned int i = 0; i != nbOccurences; i++) {
            const unsigned int termeId = int3File.readInt3();
            if (termeId != latUFile.readUIntLat()) throw IncompatibleFileException(latUFileName);
            const unsigned int increment = latSFile.readSIntLat();
            if (increment == termeId) docsNb +=1;
            else if (termePrec + increment != termeId) throw IncompatibleFileException(latSFileName);
            termePrec = termeId;
        }
        cout<<nbOccurences<<" occurrences relues avec "<<docsNb<<" documents détectés"<<endl;
        int3File.close();
        latUFile.close();
        latSFile.close();
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
