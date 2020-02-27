//
// C++ Implementation: NindSearchAmose
//
// Description: un petit moteur de recherche sur un lexique, fichier inverse et fichier des index locaux existants.
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
#include "NindAmose/NindLexiconAmose.h"
#include "NindAmose/NindTermAmose.h"
#include "NindAmose/NindLocalAmose.h"
#include "NindIndex/NindDate.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'interrogation d'une base nind préalablement indexée."<<endl;
    cout<<"L'utilisateur indique le terme cherché."<<endl;
    cout<<"Ce peut être un terme simple, un terme composé ou une entité nommée."<<endl;
    cout<<"Exemple de terme simple : \"important\""<<endl;
    cout<<"Exemple de terme composé : \"kyrielle_outil_informatique\""<<endl;
    cout<<"Exemple d'entité nommée : \"Person.PERSON:Diego_Maradona\""<<endl;
    cout<<"L'utilisateur indique ensuite le document examiné"<<endl;
    cout<<"et la localisation du terme cherché est affichée,"<<endl;
    cout<<"ainsi que les mesures utiles aux calculs de pertinence"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier lexique>"<<endl;
    cout<<"ex :   "<<arg0<<" sample_fre.lexiconindex"<<endl;
}
////////////////////////////////////////////////////////////
#define OFF "\33[m"
#define BLUE "\33[0;34m"
#define BOLD "\033[1;31m"
////////////////////////////////////////////////////////////
static void analyzeWord(const string &word,
                        string &lemma,
                        AmoseTypes &type,
                        string &entitejNommeje);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string lexiconFileName = argv[1];
    if (lexiconFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //calcule les noms des fichiers lexique et inverse
        const string incompleteFileName = lexiconFileName.substr(0, lexiconFileName.find('.'));
        //le lexique lecteur
        NindLexiconAmose nindLexicon(incompleteFileName, false);
        const NindIndex::Identification identification = nindLexicon.getIdentification();
        //le fichier inverse lecteur
        NindTermAmose nindTermAmose(incompleteFileName, false, identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(incompleteFileName, false, identification);
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //affiche les mesures globales pour les calculs de pertinence
        cout<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(SIMPLE_TERM)<<" SIMPLE_TERM uniques"<<endl;       //3.6 getUniqueTermCount()
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(MULTI_TERM)<<" MULTI_TERM uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(NAMED_ENTITY)<<" NAMED_ENTITY uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(ALL)<<" termes uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(SIMPLE_TERM)<<" occurrences de SIMPLE_TERM"<<endl;  //3.7 getTermOccurrences()
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(MULTI_TERM)<<" occurrences de MULTI_TERM"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(NAMED_ENTITY)<<" occurrences de NAMED_ENTITY"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(ALL)<<" occurrences"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindLocalAmose.getDocCount()<<" documents indexés"<<endl;                          //3.8 getDocCount()

        while (true) {
            char str [80];
            cout<<endl<<BLUE<<"Entrez le terme à rechercher (ex \"bleu\") : "<<OFF;
            cin.getline(str, 80, '\n');
            const string word = string(str);
            if (word.empty()) break;
            //le terme
            string lemma;
            AmoseTypes type;
            string entitejNommeje;
            analyzeWord(word, lemma, type, entitejNommeje);
            //trouve son identifiant
            const unsigned int ident = nindLexicon.getWordId(lemma, type, entitejNommeje);
            if (ident == 0) {
                cout<<"INCONNU"<<endl;
                continue;
            }
            cout<<"identifiant pour le terme '" <<word<<"' : "<<ident<<endl;
            //recupere le nombre d'occurences pour ce terme
            //const unsigned int nbOcc = nindTermAmose.getTermFreq(ident);
            //cout<<nbOcc<<" occurences trouvées"<<endl;
            //recupere le nombre de documents pour ce terme
            const unsigned int nbDocs = nindTermAmose.getDocFreq(ident);                        //3.5 getDocFreq()
            cout<<nbDocs<<" documents trouvés"<<endl;
            //recupere l'index inverse pour ce terme
            list<unsigned int> documents;
            nindTermAmose.getDocList(ident, documents);                                         //3.1 getDocList()
            for (list<unsigned int>::const_iterator it2 = documents.begin(); it2 != documents.end(); it2++) {
                cout<<(*it2)<<" ";
            }
            cout<<endl;
            cout<<BLUE<<"Entrez le n° de doc à afficher : "<<OFF;
            cin.getline(str, 80, '\n');
            const unsigned int noDoc = atoi(str);
            //recupere la taille du doc en nombre d'occurences de termes
            const unsigned int nbTerms = nindLocalAmose.getDocLength(noDoc);                    //3.4 getDocLength()
            cout<<nbTerms<<" termes indexés dans ce document"<<endl;
            //recupere l'index local du doc
            list<NindLocalIndex::Term> localDef;
            nindLocalAmose.getLocalDef(noDoc, localDef);
            for (list<NindLocalIndex::Term>::const_iterator it3 = localDef.begin();
                    it3 != localDef.end(); it3++) {
                const NindLocalIndex::Term &term = (*it3);
                if (term.term == ident) {
                    cout<<"<";
                    const list<NindLocalIndex::Localisation> &localisation = term.localisation;
                    string sep = "";
                    for (list<NindLocalIndex::Localisation>::const_iterator it4 = localisation.begin();
                            it4 != localisation.end(); it4++) {
                        cout<<sep<<(*it4).position<<"("<<(*it4).length<<")";
                        sep = " ";
                    }
                    cout<<"> ";
                }
            }
            cout<<endl;
        }
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
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
