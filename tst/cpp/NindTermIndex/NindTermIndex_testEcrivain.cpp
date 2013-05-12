//
// C++ Implementation: NindTermIndex_testEcrivain
//
// Description: un test pour remplir le lexique et le fichier inverse et faire differentes mesures.
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
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"Programme de test de NindTermIndex en mode écrivain (pour l'indexation)."<<endl;
    cout<<"avec le dump de documents spécifié."<<endl;
    cout<<"Si le fichier lexique n'existe pas, charge un lexique vide"<<endl;
    cout<<"Si le fichier lexique existe, charge le lexique avec le fichier lexique."<<endl;
    cout<<"(Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2.)"<<endl;
    cout<<"Teste l'écriture puis la lecture et affiche résultats et mesures."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" fre-theJysBox.fdb-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
