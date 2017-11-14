//
// C++ Implementation: Nind_testSynchro
//
// Description: un test basique pour tester la syncho entre ejcrivain et lecteurs
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
#include "NindBasics/NindFile.h"
#include <string>
#include <list>
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Teste la synchro entre un écrivain et ses lecteurs"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <nom de fichier>"<<endl;
    cout<<"ex :   "<<arg0<<" toto.bin"<<endl;
}
////////////////////////////////////////////////////////////
#define TAILLE_IDENTIFICATION 12
#define FLAG_IDENTIFICATION 53
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string fileName = argv[1];
    if (fileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        NindFile m_fileEcr(fileName);
        //si le fichier n'existe pas, le creje vide en ejcriture + lecture
        bool isOpened = m_fileEcr.open("w+b");
        if (!isOpened) throw OpenFileException("ERREUR OUVERTURE ÉCRIVAIN");
        
        NindFile m_fileLect1(fileName);
        //si fichier lecteur, ouvre en lecture seule
        isOpened = m_fileLect1.open("rb");
        if (!isOpened) throw OpenFileException("ERREUR OUVERTURE LECTEUR 1");
        
        //ejcritures et lectures
        for (unsigned int compt=0; compt != 100; compt++) {
            //ejcriture
            cerr<<"écriture compt="<<compt<<endl;
            m_fileEcr.setPos(0, SEEK_SET);    //se positionne en teste
            m_fileEcr.createBuffer(TAILLE_IDENTIFICATION);
            m_fileEcr.putInt1(FLAG_IDENTIFICATION);
            m_fileEcr.putInt3(10);
            m_fileEcr.putInt4(1505317962);
            m_fileEcr.putInt4(compt);
            m_fileEcr.writeBuffer();                               //ecriture effective sur le fichier
            //lecture
            cerr<<"lecture compt="<<compt<<endl;
            m_fileLect1.flush();
            m_fileLect1.setPos(-TAILLE_IDENTIFICATION, SEEK_END);       //se positionne sur l'identification
            m_fileLect1.readBuffer(TAILLE_IDENTIFICATION);
            const unsigned int flag = m_fileLect1.getInt1();
            if (flag != FLAG_IDENTIFICATION) throw InvalidFileException("ERREUR LECTURE 1");
            const unsigned int c1 = m_fileLect1.getInt3();
            if (c1 != 10) throw InvalidFileException("ERREUR LECTURE 2");
            const unsigned int c2 = m_fileLect1.getInt4();
            if (c2 != 1505317962) throw InvalidFileException("ERREUR LECTURE 3");
            const unsigned int c3 = m_fileLect1.getInt4();
            cerr<<"c3="<<c3<<endl;
            if (c3 != compt) throw InvalidFileException("ERREUR LECTURE 4");
        }
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////

