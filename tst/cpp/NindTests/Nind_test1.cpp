//
// C++ Implementation: Nind_test1
//
// Description: un test pour tester l'iterateur de liste
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
#include <string>
#include <list>
#include <iostream>
#include <fstream>
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Teste l'impact d'un push_back sur un itérateur de liste"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <n'importe quoi>"<<endl;
    cout<<"ex :   "<<arg0<<" toto"<<endl;
}
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}

    list<string> maListe;
    maListe.push_back("bonjour");
    list<string>::reverse_iterator rit;
    list<string>::iterator it;
    rit = maListe.rbegin();
    it = maListe.begin();
    cerr<<"maListe.size()="<<maListe.size()<<"  (*rit)="<<(*rit)<<"  (*it)="<<(*it)<<endl;
    maListe.push_back("au-revoir");
    cerr<<"maListe.size()="<<maListe.size()<<"  (*rit)="<<(*rit)<<"  (*it)="<<(*it)<<endl;
    rit = maListe.rbegin();
    cerr<<"maListe.size()="<<maListe.size()<<"  (*rit)="<<(*rit)<<"  (*it)="<<(*it)<<endl;
    maListe.push_front("bonne nuit");
    cerr<<"maListe.size()="<<maListe.size()<<"  (*rit)="<<(*rit)<<"  (*it)="<<(*it)<<endl;
    it = maListe.begin();
    cerr<<"maListe.size()="<<maListe.size()<<"  (*rit)="<<(*rit)<<"  (*it)="<<(*it)<<endl;
}
