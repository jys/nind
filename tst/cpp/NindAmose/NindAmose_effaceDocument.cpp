//
// C++ Implementation: NindAmose_effaceDocument
//
// Description: un programme pour effacer un document dans un corpus indexej
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: 2014-2016 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindAmose/NindTermAmose.h"
#include "NindAmose/NindLocalAmose.h"
#include "NindAmose/NindLexiconAmose.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'effacement d'un document dans un corpus indexé"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier lexique> <identifiant externe du doc>"<<endl;
    cout<<"ex :   "<<arg0<<" amose-dump.lexiconindex  10170385"<<endl;
}
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<3) {displayHelp(argv[0]); return false;}
    const string lexiconFileName = argv[1];
    if (lexiconFileName == "--help") {displayHelp(argv[0]); return true;}
    const string docIdentStr = argv[2];
    const unsigned int docIdent = atoi(docIdentStr.c_str());

    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const string incompleteFileName = lexiconFileName.substr(0, lexiconFileName.find('.'));
        const string termindexFileName = incompleteFileName + ".termindex";
        const string localindexFileName = incompleteFileName + ".localindex";
        //le lexique pour son identification
        NindLexiconAmose nindLexicon(lexiconFileName);
        NindIndex::Identification identification;
        nindLexicon.getIdentification(identification);
        //le fichier inverse ecrivain
        NindTermAmose nindTermAmose(termindexFileName, true, identification);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(localindexFileName, true, identification);
        //rejcupehre tous les identifiants de termes contenus dans le document
        set<unsigned int> termIdents;
        const bool trouvej = nindLocalAmose.getTermIdents(docIdent, termIdents);
        //si le doc n'existe pas, retour false
        if (!trouvej) return false;
        cout<<"effacement référence à "<<docIdent<<" dans "<<termIdents.size()<<" définitions de termes"<<endl;
        //examine chaque occurrence de terme
        for (set<unsigned int>::const_iterator itterm = termIdents.begin(); itterm != termIdents.end(); itterm++) {
            //rejcupehre le terme en clair pour avoir le type
            string lemma, namedEntity;
            AmoseTypes type;
            const bool trouvej = nindLexicon.getWord(*itterm, lemma, type, namedEntity);
            //efface le doc de la dejfinition du terme
            nindTermAmose.removeDocFromTerm(*itterm, type, docIdent, identification);
        }
        //effacement du document
        cout<<"effacement de la définition du document "<<docIdent<<endl;
        list<NindLocalIndex::Term> localDef;
        nindLocalAmose.setLocalDef(docIdent, localDef, identification);
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////
