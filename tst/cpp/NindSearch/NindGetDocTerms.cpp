//
// C++ Implementation: NindGetDocTerms
//
// Description: Affiche les termes uniques d'un documents classés par type
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2016
//
// Copyright: 2014-2016 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindAmose/NindLexiconAmose.h"
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
    cout<<"Affiche les termes uniques du document spécifié d'une"<<endl; 
    cout<<"base nind préalablement indexée."<<endl;
    cout<<"L'utilisateur indique le n° du document."<<endl;
    cout<<"Tous les termes sont affichés, classés par type :"<<endl;
    cout<<"1) termes simples, 2) termes composés 3) entités nommées"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier lexique> <n° document>"<<endl;
    cout<<"ex :   "<<arg0<<" sample_fre.lexiconindex 3110"<<endl;
}
////////////////////////////////////////////////////////////
#define OFF "\33[m"
#define BLUE "\33[0;34m"
#define BOLD "\033[1;31m"
////////////////////////////////////////////////////////////
static void afficheTermesUniques(NindLocalAmose &nindLocalAmose, 
                                 const unsigned int noDoc,
                                 const AmoseTypes termType, 
                                 const string &titre);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<3) {displayHelp(argv[0]); return false;}
    const string lexiconFileName = argv[1];
    if (lexiconFileName == "--help") {displayHelp(argv[0]); return true;}
    const string docNoStr = argv[2];
    const unsigned int docNo = atoi(docNoStr.c_str());

    try {
        //calcule les noms des fichiers lexique et inverse
        const string incompleteFileName = lexiconFileName.substr(0, lexiconFileName.find('.'));
        const string localindexFileName = incompleteFileName + ".localindex";
        //le lexique lecteur
        NindLexiconAmose nindLexicon(lexiconFileName, false);
        NindIndex::Identification identification;
        nindLexicon.getIdentification(identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(localindexFileName, false, identification);
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //recupere la taille du doc en nombre d'occurences de termes
        const unsigned int nbTerms = nindLocalAmose.getDocLength(docNo);                    //3.4 getDocLength()
        cout<<nbTerms<<" termes indexés dans ce document"<<endl;
        //affiche les termes uniques dans le document en fonction du type 
        afficheTermesUniques(nindLocalAmose, docNo, SIMPLE_TERM, "SIMPLE_TERM");
        afficheTermesUniques(nindLocalAmose, docNo, MULTI_TERM, "MULTI_TERM");
        afficheTermesUniques(nindLocalAmose, docNo, NAMED_ENTITY, "NAMED_ENTITY");
 
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static void afficheTermesUniques(NindLocalAmose &nindLocalAmose, 
                                 const unsigned int docNo,
                                 const AmoseTypes termType, 
                                 const string &titre)
{
    //rejcupehre la liste des termes uniques
    set<string> termsSet;
    bool trouvej = nindLocalAmose.getDocTerms(docNo, termType, termsSet);                             //3.3 getDocTerms()
    if (!trouvej) cout<<"document "<<docNo<<" INCONNU !"<<endl;
    cout<<BOLD<<termsSet.size()<<" "<<titre<<" uniques"<<OFF<<endl;
    string sep = "";
    for (set<string>::const_iterator itterms = termsSet.begin(); itterms != termsSet.end(); itterms++) {
        cout<<sep<<(*itterms);
        sep = ", ";
    }
    cout<<endl;
}
////////////////////////////////////////////////////////////
