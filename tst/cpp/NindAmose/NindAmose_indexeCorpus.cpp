//
// C++ Implementation: NindAmose_indexeCorpus
//
// Description: un programme pour remplir le lexique, le fichier inverse et le fichier des index locaux
// a partir d'un corpus deja syntaxiquement analyse issu d'un dump Lucene.
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
#include <vector>
#include <iostream>
#include <iomanip> 
#include <fstream>
#include <sstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'indexation d'un corpus déjà syntaxiquement analysé par Amose."<<endl;
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
static void analyzeWord(const string &word, 
                        string &lemma, 
                        AmoseTypes &type, 
                        string &entitejNommeje);
static string date(const unsigned int identification);
static bool fileSystemIsCoherent(const list<string> &fileNames);
////////////////////////////////////////////////////////////
#define NO_CG 0
#define TERMS_BUFFER_SIZE 200000
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
        const string lexiconFileName = incompleteFileName + ".lexiconindex";
        const string retrolexiconFileName = incompleteFileName + ".retrolexiconindex";
        const string termindexFileName = incompleteFileName + ".termindex";
        const string localindexFileName = incompleteFileName + ".localindex";
        //vejrifie que le systehme de fichiers est cohejrent
        if (!fileSystemIsCoherent(list<string>({ lexiconFileName, retrolexiconFileName, termindexFileName, localindexFileName }))) {
            cout<<"Des anciens fichiers existent et sont incohérents!"<<endl;
            cout<<"Veuillez les effacer par la commande : rm "<<incompleteFileName + ".*index"<<endl;
            return false;
        }
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        /////////////////////////////////////
        cout<<"Forme le lexique, le fichier inversé et le fichier des index locaux avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain avec retro lexique (meme taille d'indirection que le fichier inverse)
        NindLexiconAmose nindLexicon(lexiconFileName, true, lexiconEntryNb, termindexEntryNb);
        NindIndex::Identification identification = nindLexicon.getIdentification();
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse ecrivain
        NindTermAmose nindTermAmose(termindexFileName, true, identification, termindexEntryNb);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(localindexFileName, true, identification, localindexEntryNb);
        //lit le fichier dump de documents
        unsigned int docsNb =0;
        unsigned int termsNb =0;
        unsigned int nbMajTerm =0;
        unsigned int nbMajLex = 0;
        string dumpLine;
        //buferisation des termes
        map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > > bufferTermes;
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (getline(docsFile, dumpLine)) {
            //lit 1 ligne = 1 document
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            if (dumpLine.empty()) continue;   //evacue ainsi les lignes vides
            stringstream sdumpLine(dumpLine);
            //10170346  Location.LOCATION:Italie 280,6 création 288,8 création_parti 288,19
            docsNb++;
            unsigned int noDoc;
            string word;
            unsigned int position, taille;
            char comma;
            sdumpLine >> noDoc;
            cout<<noDoc<<"\r"<<flush;
            //la structure d'index locaux se fabrique pour un document complet
            list<NindLocalIndex::Term> localDef;
            //bufferisation des termes pour un mesme document
            map<unsigned int, pair<AmoseTypes, unsigned int> > bufferTermesParDoc;
            //lit tous les termes et leur localisation/taille
            //le python de construction atteste de la validite du format, pas la peine de controler
            while (sdumpLine >> word >> position >> comma >> taille) {
                //le terme
                string lemma;
                AmoseTypes type;
                string entitejNommeje;
                analyzeWord(word, lemma, type, entitejNommeje);
                //si le lemme est vide, raf
                if (lemma.empty()) continue;
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexicon.addWord(lemma, type, entitejNommeje);
                nbMajLex++;
                //si 0, le lemme n'ejtait pas valide
                if (id == 0) continue;
                termsNb++;
                //bufferise le terme
                //cherche s'il existe dejjah dans le buffer
                map<unsigned int, pair<AmoseTypes, unsigned int> >::iterator itterm = bufferTermesParDoc.find(id);
                //s'il n'existe pas, le creje 
                if (itterm == bufferTermesParDoc.end()) bufferTermesParDoc[id] = pair<AmoseTypes, unsigned int>(type, 1);
                //sinon increjmente le compteur
                else (*itterm).second.second +=1;             
                //augmente l'index local 
                localDef.push_back(NindLocalIndex::Term(id, NO_CG));
                NindLocalIndex::Term &term = localDef.back();
                term.localisation.push_back(NindLocalIndex::Localisation(position, taille));               
            }
            //recupere l'identification du lexique
            identification = nindLexicon.getIdentification();
            //ecrit la definition sur le fichier des index locaux
            nindLocalAmose.setLocalDef(noDoc, localDef, identification);
            //bufferise termes + docs 
            for (map<unsigned int, pair<AmoseTypes, unsigned int> >::const_iterator itterm = bufferTermesParDoc.begin();
                 itterm != bufferTermesParDoc.end(); itterm++) {
                const unsigned int &idterm = (*itterm).first;
                const AmoseTypes &type = (*itterm).second.first;
                const unsigned int &freq = (*itterm).second.second;
                //cherche s'il existe dejjah dans le buffer
                map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > >::iterator itterm2 = bufferTermes.find(idterm);
                //s'il n'existe pas, le creje 
                if (itterm2 == bufferTermes.end()) 
                    bufferTermes[idterm] = pair<AmoseTypes, list<NindTermIndex::Document> >(type, { NindTermIndex::Document(noDoc,freq) });
                //sinon ajoute le document
                else (*itterm2).second.second.push_back(NindTermIndex::Document(noDoc,freq));          
            }
            //vejrifie si le buffer a dejpassej ses limites
            if (bufferTermes.size() < TERMS_BUFFER_SIZE) continue;
            //ejcrit le buffer sur disque
            for (map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > >::const_iterator itterm2 = bufferTermes.begin();
                 itterm2 != bufferTermes.end(); itterm2++) {
                const unsigned int &termid = (*itterm2).first;
                const AmoseTypes &type = (*itterm2).second.first;
                const list<NindTermIndex::Document> &documents = (*itterm2).second.second;
                nindTermAmose.addDocsToTerm(termid, type, documents, identification);                   
                nbMajTerm++;
            }
            //raz buffer
            bufferTermes.clear();
        }
        //ejcrit le buffer sur disque
        for (map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > >::const_iterator itterm2 = bufferTermes.begin();
                itterm2 != bufferTermes.end(); itterm2++) {
            const unsigned int &termid = (*itterm2).first;
            const AmoseTypes &type = (*itterm2).second.first;
            const list<NindTermIndex::Document> &documents = (*itterm2).second.second;
            nindTermAmose.addDocsToTerm(termid, type, documents, identification);                   
            nbMajTerm++;
        }
        docsFile.close();
        end = clock();
        cout<<setw(8)<<setfill(' ')<<nbMajLex<<" accès / mises à jour sur "<<lexiconFileName<<endl;
        cout<<setw(8)<<setfill(' ')<<termsNb<<" occurrences de termes ajoutés sur "<<termindexFileName<<endl;
        cout<<setw(8)<<setfill(' ')<<nbMajTerm<<" mises à jour sur "<<termindexFileName<<endl;
        cout<<setw(8)<<setfill(' ')<<docsNb<<" mises à jour sur "<<localindexFileName<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
        cout<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(SIMPLE_TERM)<<" SIMPLE_TERM uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(MULTI_TERM)<<" MULTI_TERM uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(NAMED_ENTITY)<<" NAMED_ENTITY uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getUniqueTermCount(ALL)<<" termes uniques"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(SIMPLE_TERM)<<" occurrences de SIMPLE_TERM"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(MULTI_TERM)<<" occurrences de MULTI_TERM"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(NAMED_ENTITY)<<" occurrences de NAMED_ENTITY"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindTermAmose.getTermOccurrences(ALL)<<" occurrences"<<endl;
        cout<<setw(8)<<setfill(' ')<<nindLocalAmose.getDocCount()<<" documents indexés"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; throw; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; throw; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; throw; return false; }
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
static string date(const unsigned int identification)
{
    const time_t time = (time_t) identification;
    struct tm * timeinfo = localtime (&time);
    char buffer [80];
    strftime (buffer,80,"%Y-%m-%d %X",timeinfo);
    return string(buffer);
}
////////////////////////////////////////////////////////////
static bool fileSystemIsCoherent(const list<string> &fileNames)
{
    bool allFiles = true;
    bool noneFile = true;
    for (list<string>::const_iterator itname = fileNames.begin(); itname != fileNames.end(); itname++) {
        FILE *file =  fopen((*itname).c_str(), "rb");
        if (file) {
            fclose(file);
            noneFile = false;
        }
        else allFiles = false;
    }
    //les fichiers doivent tous exister ou tous ne pas exister
    return (allFiles || noneFile);
}
////////////////////////////////////////////////////////////

