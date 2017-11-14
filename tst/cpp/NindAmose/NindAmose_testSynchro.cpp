//
// C++ Implementation: NindAmose_testSynchro
//
// Description: Un corpus dejjah analysej par Lima est indexej. Intercallejs dans l'indexation des 
// fichiers ejcrivains, des lectures sont faites par des fichiers lecteurs pour vejrifier que la
// synchronicitej des lecteurs sur l'ejcrivain est bien assureje par le fichier physique.
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
#include "NindAmose/NindTermAmose.h"
#include "NindAmose/NindLocalAmose.h"
#include "NindAmose/NindLexiconAmose.h"
#include "NindAmose_litTexteAnalysej.h"
#include "NindIndex/NindDate.h"
#include "NindIndex/NindFichiers.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <iostream>
#include <iomanip> 
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'indexation d'un corpus déjà syntaxiquement analysé par Lima."<<endl;
    cout<<"À chaque indexation par un fichier écrivain, une lecture est faite sur"<<endl;
    cout<<"le fichier lecteur associé pour valider la synchronisation écrivain-lecteurs"<<endl;
    cout<<"Le corpus est un fichier texte avec une ligne par document :"<<endl;
    cout<<"<n° document>  { <terme> <localisation>,<taille> }"<<endl;
    cout<<"Si aucun fichier n'existe, les fichiers sont créés."<<endl;
    cout<<"Si les fichiers existent et sont cohérents, ils sont mis à jour"<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le lexique,"<<endl;
    cout<<"le fichier inversé et le fichier des index locaux."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <taille lexique> <taille inverse> <taille locaux>"<<endl;
    cout<<"ex :   "<<arg0<<" sample_fre.xml.mult.xml.txt 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
#define NO_CG 0
#define RED "\033[1;31m"
#define BLA "\033[0m"
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<4) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string lexiconEntryNbStr = argv[2];
    const string termindexEntryNbStr = argv[3];
    const string localindexEntryNbStr = argv[4];
    
    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    const unsigned int termindexEntryNb = atoi(termindexEntryNbStr.c_str());
    const unsigned int localindexEntryNb = atoi(localindexEntryNbStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const string incompleteFileName = docsFileName.substr(0, docsFileName.find('.'));
        //vejrifie que le systehme de fichiers est cohejrent
        if (!NindFichiers::fichiersCohejrents(incompleteFileName, true, true)) {
            cout<<"Des anciens fichiers existent!"<<endl;
            cout<<"Veuillez les effacer par la commande : rm "<<incompleteFileName + ".nind*"<<endl;
            return false;
        }
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        /////////////////////////////////////
        cout<<"Forme le sytème de fichiers nind avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain avec retro lexique (meme taille d'indirection que le fichier inverse)
        NindLexiconAmose nindLexiconAmose(incompleteFileName, true, lexiconEntryNb, termindexEntryNb);
        NindIndex::Identification identification = nindLexiconAmose.getIdentification();
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //les fichiers ejcrivains des termes et des index locaux 
        NindTermAmose nindTermAmose(incompleteFileName, true, identification, termindexEntryNb);
        NindLocalAmose nindLocalAmose(incompleteFileName, true, identification, localindexEntryNb);
        //les fichiers lecteurs lexique, des termes et des index locaux
        NindLexiconAmose nindLexiconAmoseLect(incompleteFileName, false);
        NindTermAmose nindTermAmoseLect(incompleteFileName, false, identification);
        NindLocalAmose nindLocalAmoseLect(incompleteFileName, false, identification);
        //lit le fichier dump de documents
        NindAmose_litTexteAnalysej nindAmose_litTexteAnalysej(docsFileName);
        unsigned int docsNb =0, termsNb =0, nbMajTerm =0, nbMajLex = 0;
        unsigned int nbLexNok = 0, nbLexOk = 0, nbTermNok = 0, nbTermOk = 0, nbLocalNok = 0, nbLocalOk = 0;
        unsigned int nbRetrolexNok = 0, nbRetrolexOk = 0, nbComptNok = 0, nbComptOk = 0;
        unsigned int nbNbdocNok = 0, nbNbdocOk = 0;
        unsigned int noDoc;
        while (nindAmose_litTexteAnalysej.documentSuivant(noDoc)) {
            docsNb++;
            cout<<noDoc<<"\r"<<flush;
            //la structure d'index locaux se fabrique pour un document complet
            list<NindLocalIndex::Term> localDef;
            //le no de doc avec la freq ah 1
            const list<NindTermIndex::Document> newDocuments ={ NindTermIndex::Document(noDoc, 1) };
            //lit tous les termes et leur localisation/taille
            string lemme, entitejNommeje;
            AmoseTypes type;
            unsigned int position, taille;
            while (nindAmose_litTexteAnalysej.motSuivant(lemme, type, entitejNommeje, position, taille)) {
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexiconAmose.addWord(lemme, type, entitejNommeje);
                nbMajLex++;
                //si 0, le lemme n'ejtait pas valide
                if (id == 0) continue;
                termsNb++;
                //vejrifie que ce qui a ejtej indexej est immejdiatement disponible
                string lemmeLect, entitejNommejeLect;
                AmoseTypes typeLect;
                if (nindLexiconAmoseLect.getWord(id, lemmeLect, typeLect, entitejNommejeLect)) {
                    if ((lemmeLect == lemme) && (typeLect == type) && (entitejNommejeLect == entitejNommeje)) nbRetrolexOk++;
                    else nbRetrolexNok++;
                }
                else nbRetrolexNok++;
               //recupere l'identification du lexique
                identification = nindLexiconAmose.getIdentification();
                nindTermAmose.addDocsToTerm(id, type, newDocuments, identification);                   
                nbMajTerm++;
                //augmente l'index local 
                localDef.push_back(NindLocalIndex::Term(id, NO_CG));
                NindLocalIndex::Term &term = localDef.back();
                term.localisation.push_back(NindLocalIndex::Localisation(position, taille));  
                //vejrifie que ce qui a ejtej indexej est immejdiatement disponible
                //vejrifie que les compteurs du lecteur sont ok
                if (nindTermAmose.getUniqueTermCount(ALL) != nindTermAmoseLect.getUniqueTermCount(ALL)) nbComptNok++;
                else nbComptOk++;
                //rejcupehre l'id du terme
                const unsigned int idlect = nindLexiconAmoseLect.getWordId(lemme, type, entitejNommeje);
                if (idlect == 0) { nbLexNok++; continue; }
                nbLexOk++;
                //rejcupehre la liste des documents ouh est indexej ce terme
                list<unsigned int> documentIds;
                const bool trouvej = nindTermAmoseLect.getDocList(idlect, documentIds);
                if (!trouvej) { nbTermNok++; continue; }
                //le doc doit estre dans la liste
                list<unsigned int>::const_iterator itdoc = documentIds.begin();
                while (itdoc != documentIds.end()) {
                    if (*itdoc == noDoc) break;
                    itdoc++;
                }
                if (itdoc == documentIds.end()) {nbTermNok++; continue; }
                if (nindTermAmoseLect.getDocFreq(idlect) != documentIds.size()) {nbTermNok++; continue; }
                nbTermOk++;
            }
            //recupere l'identification du lexique
            identification = nindLexiconAmose.getIdentification();
            //ecrit la definition sur le fichier des index locaux
            nindLocalAmose.setLocalDef(noDoc, localDef, identification);
            //vejrifie que tout est bien ejcrit et immejdiatement disponible
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localDefLect;
            nindLocalAmoseLect.getLocalDef(noDoc, localDefLect);
            if (localDef.size() == localDefLect.size()) nbLocalOk++;
            else nbLocalNok++;
            if (nindLocalAmose.getDocCount() == nindLocalAmoseLect.getDocCount()) nbNbdocOk++;
            else nbNbdocNok++;
        }
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nbMajLex<<" accès / mises à jour sur "<<nindLexiconAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<termsNb<<" occurrences de termes ajoutés sur "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbMajTerm<<" mises à jour sur "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<docsNb<<" mises à jour sur "<<nindLocalAmose.getFileName()<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
        cout<<endl;
        cout<<setw(8)<<setfill(' ')<<nbLexOk<<" occurrences consultées avec succès dans "<<nindLexiconAmose.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbLexNok<<BLA<<" occurrences consultées en échec dans "<<nindLexiconAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbRetrolexOk<<" occurrences consultées avec succès dans "<<nindLexiconAmose.getRetrolexiconFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbRetrolexNok<<BLA<<" occurrences consultées en échec dans "<<nindLexiconAmose.getRetrolexiconFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbTermOk<<" occurrences consultées avec succès dans "<<nindTermAmose.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbTermNok<<BLA<<" occurrences consultées en échec dans "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbComptOk<<" compteurs consultés avec succès dans "<<nindTermAmose.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbComptNok<<BLA<<" compteurs consultés en échec dans "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbLocalOk<<" occurrences consultées avec succès dans "<<nindLocalAmose.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbLocalNok<<BLA<<" occurrences consultées en échec dans "<<nindLocalAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<nbNbdocOk<<" compteurs consultés avec succès dans "<<nindLocalAmose.getFileName()<<endl;
        cout<<RED<<setw(8)<<setfill(' ')<<nbNbdocNok<<BLA<<" compteurs consultés en échec dans "<<nindLocalAmose.getFileName()<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
}
////////////////////////////////////////////////////////////
