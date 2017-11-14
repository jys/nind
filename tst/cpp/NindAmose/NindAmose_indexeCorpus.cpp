//
// C++ Implementation: NindAmose_indexeCorpus
//
// Description: un programme pour remplir le lexique, le fichier inverse et le fichier des index locaux
// a partir d'un corpus deja syntaxiquement analyse issu d'un dump Lucene.
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
#include <glob.h>
#include <string>
#include <list>
#include <iostream>
#include <iomanip> 
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
#define NO_CG 0
#define TERMS_BUFFER_SIZE 200000
//#define TERMS_BUFFER_SIZE 0
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATEJCON"<<endl;
    cout<<"Programme d'indexation d'un corpus déjà syntaxiquement analysé par Lima."<<endl;
    cout<<"Le corpus est un ensemble de fichiers texte avec une ligne par document :"<<endl;
    cout<<"<n° document>  { <terme> <localisation>,<taille> }"<<endl;
    cout<<"Si aucun fichier n'existe, les fichiers sont créés."<<endl;
    cout<<"Si les fichiers existent et sont cohérents, ils sont mis à jour"<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le lexique,"<<endl;
    cout<<"le fichier inversé et le fichier des index locaux."<<endl;
    cout<<"Les termes sont bufferisés par paquets de "<<TERMS_BUFFER_SIZE<<" avant écriture."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <documents analysés> <racine fichiers> [ <taille lexique> <taille inverse> <taille locaux> ]"<<endl;
    cout<<"ex :   "<<arg0<<" \"clef/ger/*.xml.mult.xml.txt\" \"clef/ger\" 1999993 2000000 100000"<<endl;
}
////////////////////////////////////////////////////////////
class Indexe {
public:
    Indexe (NindLexiconAmose &nindLexiconAmose,
            NindTermAmose &nindTermAmose,
            NindLocalAmose &nindLocalAmose);
    virtual ~Indexe();
    void fichier (const string &docsFileName);
    void purge();
    unsigned int m_docsNb;
    unsigned int m_termsNb;
    unsigned int m_nbMajTerm;
    unsigned int m_nbMajLex;
private:
    NindLexiconAmose &m_nindLexiconAmose;
    NindTermAmose &m_nindTermAmose;
    NindLocalAmose &m_nindLocalAmose;
    map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > > m_bufferTermes;
};
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<3) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string incompleteFileName = argv[2];
    unsigned int lexiconEntryNb = 0;
    unsigned int termindexEntryNb = 0;
    unsigned int localindexEntryNb = 0;
    if (argc>3) {
        if (argc<6) {displayHelp(argv[0]); return false;}
        const string lexiconEntryNbStr = argv[3];
        const string termindexEntryNbStr = argv[4];
        const string localindexEntryNbStr = argv[5];
        lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
        termindexEntryNb = atoi(termindexEntryNbStr.c_str());
        localindexEntryNb = atoi(localindexEntryNbStr.c_str());
    }

    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        //vejrifie que le systehme de fichiers est cohejrent
        if (!NindFichiers::fichiersCohejrents(incompleteFileName, true, false)) {
            cout<<"Des anciens fichiers existent et sont incohérents!"<<endl;
            cout<<"Veuillez les effacer par la commande : rm "<<incompleteFileName + ".nind*"<<endl;
            return false;
        }
        cout<<"Les termes sont bufferisés par paquets de "<<TERMS_BUFFER_SIZE<<" avant écriture."<<endl;
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        /////////////////////////////////////
        start = clock();
        //le lexique ecrivain avec retro lexique (meme taille d'indirection que le fichier inverse)
        NindLexiconAmose nindLexiconAmose(incompleteFileName, true, lexiconEntryNb, termindexEntryNb);
        NindIndex::Identification identification = nindLexiconAmose.getIdentification();
        cout<<"identification : "<<identification.lexiconWordsNb<<" termes, "<<identification.lexiconTime;
        cout<<" ("<<NindDate::date(identification.lexiconTime)<<")"<<endl;
        //le fichier inverse ecrivain
        NindTermAmose nindTermAmose(incompleteFileName, true, identification, termindexEntryNb);
        //le fichier des index locaux
        NindLocalAmose nindLocalAmose(incompleteFileName, true, identification, localindexEntryNb);
        //init la classe qui indexe
        Indexe indexe(nindLexiconAmose, nindTermAmose, nindLocalAmose);
        //lit tous les fichiers
        glob_t globbuf;
        const int err = glob(docsFileName.c_str(), 0, NULL, &globbuf);
        if(err != 0) throw OpenFileException(docsFileName);
        for (size_t i = 0; i < globbuf.gl_pathc; i++) indexe.fichier(globbuf.gl_pathv[i]);
        indexe.purge();
        globfree(&globbuf);
        end = clock();
        cout<<setw(8)<<setfill(' ')<<indexe.m_nbMajLex<<" accès / mises à jour sur "<<nindLexiconAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<indexe.m_termsNb<<" occurrences de termes ajoutés sur "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<indexe.m_nbMajTerm<<" mises à jour sur "<<nindTermAmose.getFileName()<<endl;
        cout<<setw(8)<<setfill(' ')<<indexe.m_docsNb<<" mises à jour sur "<<nindLocalAmose.getFileName()<<endl;
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
Indexe::Indexe (NindLexiconAmose &nindLexiconAmose,
                NindTermAmose &nindTermAmose,
                NindLocalAmose &nindLocalAmose):
    m_docsNb(0),
    m_termsNb(0),
    m_nbMajTerm(0),
    m_nbMajLex(0),
    m_nindLexiconAmose(nindLexiconAmose),
    m_nindTermAmose(nindTermAmose),
    m_nindLocalAmose(nindLocalAmose),
    m_bufferTermes()
{
}
////////////////////////////////////////////////////////////
Indexe::~Indexe()
{
}
////////////////////////////////////////////////////////////
void Indexe::fichier(const string &docsFileName)
{
    //lit le fichier dump de documents
    NindAmose_litTexteAnalysej nindAmose_litTexteAnalysej(docsFileName);
    unsigned int noDoc;
    while (nindAmose_litTexteAnalysej.documentSuivant(noDoc)) {
        m_docsNb++;
        cout<<m_docsNb<<"\r"<<flush;
        //la structure d'index locaux se fabrique pour un document complet
        list<NindLocalIndex::Term> localDef;
        //bufferisation des termes pour un mesme document
        map<unsigned int, pair<AmoseTypes, unsigned int> > bufferTermesParDoc;
        //lit tous les termes et leur localisation/taille
        string lemme;
        AmoseTypes type;
        string entitejNommeje;
        unsigned int position, taille;
        while (nindAmose_litTexteAnalysej.motSuivant(lemme, type, entitejNommeje, position, taille)) {
            //recupere l'id du terme dans le lexique, l'ajoute eventuellement
            const unsigned int id = m_nindLexiconAmose.addWord(lemme, type, entitejNommeje);
            m_nbMajLex++;
            //si 0, le lemme n'ejtait pas valide
            if (id == 0) continue;
            m_termsNb++;
            //bufferise le terme
            //cherche s'il existe dejjah dans le buffer
            map<unsigned int, pair<AmoseTypes, unsigned int> >::iterator itterm = bufferTermesParDoc.find(id);
            //s'il n'existe pas, le creje 
            if (itterm == bufferTermesParDoc.end()) bufferTermesParDoc[id] = pair<AmoseTypes, unsigned int>(type, 1);
            //sinon increjmente le compteur
            else { 
                if ((*itterm).second.first != type)
                    cerr<<id<<" lemme="<<lemme<<" type="<<type<<" / "<<(*itterm).second.first<<" ds doc n°:"<<noDoc<<endl;
                (*itterm).second.second +=1;   
            }
            //augmente l'index local 
            localDef.push_back(NindLocalIndex::Term(id, NO_CG));
            NindLocalIndex::Term &term = localDef.back();
            term.localisation.push_back(NindLocalIndex::Localisation(position, taille));               
        }
        //recupere l'identification du lexique
        NindIndex::Identification identification = m_nindLexiconAmose.getIdentification();
        //ecrit la definition sur le fichier des index locaux
        m_nindLocalAmose.setLocalDef(noDoc, localDef, identification);
        //bufferise termes + docs 
        for (map<unsigned int, pair<AmoseTypes, unsigned int> >::const_iterator itterm = bufferTermesParDoc.begin();
                itterm != bufferTermesParDoc.end(); itterm++) {
            const unsigned int &idterm = (*itterm).first;
            const AmoseTypes &type = (*itterm).second.first;
            const unsigned int &freq = (*itterm).second.second;
            //cherche s'il existe dejjah dans le buffer
            map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > >::iterator itterm2 = m_bufferTermes.find(idterm);
            //s'il n'existe pas, le creje 
            if (itterm2 == m_bufferTermes.end()) 
                m_bufferTermes[idterm] = pair<AmoseTypes, list<NindTermIndex::Document> >(type, { NindTermIndex::Document(noDoc,freq) });
            //sinon ajoute le document
            else (*itterm2).second.second.push_back(NindTermIndex::Document(noDoc,freq));          
        }
        //vejrifie si le buffer a dejpassej ses limites
        if (m_bufferTermes.size() > TERMS_BUFFER_SIZE) purge();
    }
}
////////////////////////////////////////////////////////////
void Indexe::purge()
{
    //recupere l'identification du lexique
    NindIndex::Identification identification = m_nindLexiconAmose.getIdentification();
    //ejcrit le buffer sur disque
    for (map<unsigned int, pair<AmoseTypes, list<NindTermIndex::Document> > >::const_iterator itterm2 = m_bufferTermes.begin();
            itterm2 != m_bufferTermes.end(); itterm2++) {
        const unsigned int &termid = (*itterm2).first;
        const AmoseTypes &type = (*itterm2).second.first;
        const list<NindTermIndex::Document> &documents = (*itterm2).second.second;
        m_nindTermAmose.addDocsToTerm(termid, type, documents, identification);                   
        m_nbMajTerm++;
    }
    //raz buffer
    m_bufferTermes.clear();
}
////////////////////////////////////////////////////////////

