// C++ Implementation: Nind_testLateconNumber
//
// Description: un test valider l'encodage et le decodage des nombres latecon.
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
#include "NindBasics//NindFile.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <iostream>
#include  <iomanip>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme de test des nombres latecon de NindFile."<<endl;
    cout<<"Crée /tmp/Nind_testLateconNumber.lat."<<endl;
    cout<<"Écrit la taille en tête puis écrit le nombre saisi (N) sous 4 formes :"<<endl;
    cout<<"1) N en non signé, 2) N en signé, 3) -N en signé, 4) -N en non signé,"<<endl;
    cout<<"Relit les 4 nombres sous 3 interprétations :"<<endl;
    cout<<"1) en octets 2) non signée, 3) signée"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <nbre>"<<endl;
    cout<<"ex :   "<<arg0<<" 65"<<endl;
}
////////////////////////////////////////////////////////////
void afficheOctets(list<unsigned char>::const_iterator &itO);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string  NbreStr =argv[1];
    if (NbreStr == "--help") {displayHelp(argv[0]); return true;}
    
    const unsigned int nbre = atoi(NbreStr.c_str());
    const string fileName = "/tmp/Nind_testLateconNumber.lat";
    
    try {
        NindFile file(fileName);          
        bool isOpened = file.open("w+b");
        if (!isOpened) throw OpenFileException(fileName);
        file.createBuffer(3 + 4 * 5);
        file.putInt3(0);
        file.putUIntLat((unsigned int)nbre);
        file.putSIntLat((signed int)nbre);
        file.putSIntLat((signed int)-nbre);
        file.putUIntLat((unsigned int)-nbre);
        const unsigned int size =file.getOutBufferSize()-3;
        file.putInt3(size, 0);        
        file.writeBuffer();
        file.close();
                
        NindFile fileR(fileName);          
        isOpened = fileR.open("rb");
        if (!isOpened) throw OpenFileException(fileName);
        fileR.setPos(0, SEEK_SET);
        fileR.readBuffer(3);
        const unsigned int taille = fileR.getInt3();
        cout<<"taille="<<taille<<endl;
        //lit les octets
        fileR.readBuffer(taille);
        list<unsigned char> octets;
        while (!fileR.endOfInBuffer()) octets.push_back(fileR.getInt1());
        //lit les nombres en positif
        fileR.setPos(3, SEEK_SET);
        fileR.readBuffer(taille);
        list<unsigned int> nombresU;
        while (!fileR.endOfInBuffer()) nombresU.push_back(fileR.getUIntLat());
        //lit les nombres en negatif
        fileR.setPos(3, SEEK_SET);
        fileR.readBuffer(taille);
        list<signed int> nombresS;
        while (!fileR.endOfInBuffer()) nombresS.push_back(fileR.getSIntLat());
        fileR.close();

        //affiche tout
        list<unsigned char>::const_iterator itO = octets.begin();
        list<unsigned int>::const_iterator itU = nombresU.begin();
        list<signed int>::const_iterator itS = nombresS.begin();
        cout<<"U "<<dec<<(unsigned int)nbre<<" => ";
        afficheOctets(itO);
        cout<<"   U: "<<dec<<(*itU++)<<"   S: "<<(*itS++)<<endl;
        cout<<"S "<<dec<<(signed int)nbre<<" => ";
        afficheOctets(itO);
        cout<<"   U: "<<dec<<(*itU++)<<"   S: "<<(*itS++)<<endl;
        cout<<"S "<<dec<<(signed int)-nbre<<" => ";
        afficheOctets(itO);
        cout<<"   U: "<<dec<<(*itU++)<<"   S: "<<(*itS++)<<endl;
        cout<<"U "<<dec<<(unsigned int)-nbre<<" => ";
        afficheOctets(itO);
        cout<<"   U: "<<dec<<(*itU++)<<"   S: "<<(*itS++)<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
void afficheOctets(list<unsigned char>::const_iterator &itO)
{
    cout<<"0x: "<<hex<<uppercase;
    unsigned char octetTete = (*itO++);
    cout<<setfill('0')<<setw(2)<<(int)octetTete;
    while (octetTete&0x80) {
        cout<<setfill('0')<<setw(2)<<(int)(*itO++);
        octetTete = octetTete<<1;
    }
}
   
