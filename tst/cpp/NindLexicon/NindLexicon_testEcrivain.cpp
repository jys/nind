//
// C++ Implementation: NindLexicon_testEcrivain
//
// Description: un test pour remplir le lexique et faire differentes mesures.
// idem NindLexicon_test2 mais avec fichier lexique
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
#include "NindIndex/NindIndex_litDumpS2.h"
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
    cout<<"Programme de test de NindLexicon en mode écrivain"<<endl;
    cout<<"avec le dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Si le fichier lexique n'existe pas, charge un lexique vide"<<endl;
    cout<<"Si le fichier lexique existe, charge le lexique avec le fichier lexique."<<endl;
    cout<<"Teste l'écriture puis la lecture et affiche résultats et mesures."<<endl;
    cout<<"Pour mesurer les vrais temps d'écriture et de lecture du lexique,"<<endl;
    cout<<"ce programme utilise le principe de la double pesée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
void formeLexique(const bool pourDeVrai,
                  const string &docsFileName,
                  NindLexicon &nindLexicon,
                  list<pair<unsigned int, list<string> > > &allWords,
                  unsigned int &docsNb);
void interrogeLexique(const bool pourDeVrai,
                      const list<pair<unsigned int, list<string> > > &allWords,
                      NindLexicon &nindLexicon);
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

        //le lexique ecrivain
        NindLexicon nindLexicon(lexiconFileName, true);
        //la correspondance de tous les mots avec leur identifiant
        list<pair<unsigned int, list<string> > > allWords;
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
        cout<<"3) vérifie l'intégrité"<<endl;
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
        cout<<"4) redemande les "<<allWords.size()<<" mots soumis et vérifie leur identifiant pour de faux"<<endl;
        start = clock();
        interrogeLexique(false, allWords, nindLexicon);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"OK en "<<cpuTimeUsed<<" secondes"<<endl;

        cout<<"5) redemande les "<<allWords.size()<<" mots soumis et vérifie leur identifiant pour de vrai"<<endl;
        start = clock();
        interrogeLexique(true, allWords, nindLexicon);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"OK en "<<cpuTimeUsed<<" secondes"<<endl;
        /////////////////////////////////////

        return true;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
void formeLexique(const bool pourDeVrai,
                  const string &docsFileName,
                  NindLexicon &nindLexicon,
                  list<pair<unsigned int, list<string> > > &allWords,
                  unsigned int &docsNb)
{
    //lit le fichier dump de documents
    docsNb = 0;
    allWords.clear();
    //lit le fichier dump de documents
    NindIndex_litDumpS2 nindIndex_litDumpS2(docsFileName);
    unsigned int noDocAnt, noDocFb;
    while (nindIndex_litDumpS2.documentSuivant(noDocAnt, noDocFb)) {
        docsNb++;
        cout<<docsNb<<"\r"<<flush;
        //lit tous les termes et leur localisation/taille
        list<string> composants;
        unsigned char cg;
        list<pair<unsigned int, unsigned int> > localisation;
        while (nindIndex_litDumpS2.motSuivant(composants, cg, localisation)) {
            unsigned int id = 0;
            if (pourDeVrai) id = nindLexicon.addWord(composants);
            allWords.push_back(pair<unsigned int, list<string> >(id, composants));
        }
        
    }
}
////////////////////////////////////////////////////////////
void interrogeLexique(const bool pourDeVrai,
                      const list<pair<unsigned int, list<string> > > &allWords,
                      NindLexicon &nindLexicon)
{
    for (list<pair<unsigned int, list<string> > >::const_iterator wordIt = allWords.begin(); 
            wordIt != allWords.end(); wordIt++) {
        const list<string> &componants = (*wordIt).second;
        if (pourDeVrai) {
            const unsigned int id = nindLexicon.getId(componants);
            if (id != wordIt->first) throw IntegrityException(wordIt->second.front());
        }
    }
}
////////////////////////////////////////////////////////////
