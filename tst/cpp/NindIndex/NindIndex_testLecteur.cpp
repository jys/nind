//
// C++ Implementation: NindIndex_testLecteur
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndexTest.h"
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
    cout<<"Programme de test de NindLexicon, NindTermIndex et NindLocalIndex en mode lecteur."<<endl;
    cout<<"La consultation est guidée par le dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
static bool docTrouve(const unsigned int noDoc, 
                      const unsigned int cg, 
                      const list<NindTermIndex::TermCG> &termIndex);
static bool termeTrouve(const unsigned int id, 
                        const unsigned int cg, 
                        const unsigned int pos, 
                        const unsigned int taille, 
                        const list<string> &componants,
                        list<NindLocalIndex::Term>::const_iterator &localIndexIt);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    
    try {
        //calcule les noms des fichiers lexique et inverse
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        const string localindexFileName = docsFileName.substr(0, pos) + ".localindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        
        //le lexique lecteur
        NindLexicon nindLexicon(lexiconFileName, false);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse lecteur
        NindTermIndex nindTermIndex(termindexFileName, false,  wordsNb, identification);
        //le fichier des index locaux
        NindLocalIndex nindLocalIndex(localindexFileName, false, wordsNb, identification);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        unsigned int nbInconnus, nbTermNok, nbTermOk, nbLocalNok, nbLocalOk = 0;
        char charBuff[LINE_SIZE];
        ifstream docsFile(docsFileName.c_str(), ifstream::in);
        if (docsFile.fail()) throw OpenFileException(docsFileName);
        while (docsFile.good()) {
            unsigned int noDoc;
            list<NindIndexTest::WordDesc> wordsList;
            docsFile.getline(charBuff, LINE_SIZE);
            if (string(charBuff).empty()) continue;   //evacue ainsi les lignes vides
            docsNb++;
            if (docsFile.fail()) throw FormatFileException(docsFileName);
            nindIndexTest.getWords(string(charBuff), noDoc, wordsList);
            //recupere l'index local du doc             
            list<NindLocalIndex::Term> localIndex;
            nindLocalIndex.getLocalIndex(noDoc, localIndex);
            list<NindLocalIndex::Term>::const_iterator localIndexIt = localIndex.begin();
            //prend tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le terme
                list<string> componants;
                nindIndexTest.split((*wordIt).word, componants);
                //la cg
                const unsigned char cg = nindIndexTest.getCgIdent((*wordIt).cg);
                unsigned int id;
                //recupere l'id du terme dans le lexique
                id = nindLexicon.getId(componants);
                if (id == 0) {
                    nbInconnus +=1;
                    nbTermNok +=1;
                    nbLocalNok +=1;
                    continue;
                }
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termIndex;
                nindTermIndex.getTermIndex(id, termIndex);
                //si le terme n'existe pas encore, la liste reste vide
                if (docTrouve(noDoc, cg, termIndex)) nbTermOk +=1;
                else nbTermNok +=1;
                //verifie l'index local
                const unsigned int pos = (*wordIt).pos;
                const unsigned int taille = (*wordIt).word.size();
                if (termeTrouve(id, cg, pos, taille, componants, localIndexIt)) nbLocalOk +=1;
                else nbLocalNok +=1;
            }
            cout<<docsNb<<"\r"<<flush;
            //if (docsNb ==100) break;
        }        
        docsFile.close();
        cout<<nbInconnus<<" occurrences inconnues du lexique"<<endl;
        cout<<nbTermOk<<" occurrences consultées avec succès dans "<<termindexFileName<<endl;
        cout<<nbTermNok<<" occurrences consultées en échec dans "<<termindexFileName<<endl;
        cout<<nbLocalOk<<" occurrences consultées avec succès dans "<<localindexFileName<<endl;
        cout<<nbLocalNok<<" occurrences consultées en échec dans "<<localindexFileName<<endl;
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;        
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static bool docTrouve(const unsigned int noDoc, 
                      const unsigned int cg, 
                      const list<NindTermIndex::TermCG> &termIndex)
{
    list<NindTermIndex::TermCG>::const_iterator it1 = termIndex.begin(); 
    while (it1 != termIndex.end()) {
        if ((*it1).cg == cg) {
            //c'est la meme cg, on va chercher le doc 
            const list<NindTermIndex::Document> &documents = (*it1).documents;
            //trouve le doc dans la liste ordonnee
            list<NindTermIndex::Document>::const_iterator it2 = documents.begin(); 
            while (it2 != documents.end()) {
                //le doc devrait etre dans la liste ordonnee
                if ((*it2).ident == noDoc) return true;
                it2++;
            }
            if (it2 == documents.end()) return false;
        }
        it1++;
    }
    if (it1 == termIndex.end()) return false;
}
////////////////////////////////////////////////////////////
static bool termeTrouve(const unsigned int id, 
                        const unsigned int cg, 
                        const unsigned int pos, 
                        const unsigned int taille, 
                        const list<string> &componants,
                        list<NindLocalIndex::Term>::const_iterator &localIndexIt)
{
    const NindLocalIndex::Term &term = (*localIndexIt++);
    if (term.term != id && term.cg != cg) return false;
    //simulation de cas reel de termes en plusieurs parties (c'est tout a fait arbitraire)
    //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
    //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
    //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
    //autres : (pos, taille)
    const list<NindLocalIndex::Localisation> &localisation = term.localisation; 
    list<NindLocalIndex::Localisation>::const_iterator locIt = localisation.begin();
    const unsigned int nbComposants =  componants.size();
    list<string>::const_iterator compIt = componants.begin();
    const string &firstComp = (*compIt++);
    const char firstChar = firstComp.front();
    if (nbComposants == 2 && firstChar == 'T') {
        //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        if ((*locIt).position != pos || (*locIt).length != lenAA) return false;
        locIt++;
        if ((*locIt).position != pos + lenAA || (*locIt).length != lenBB) return false;
    }
    else if (nbComposants == 3 && firstChar == 'k') {
        //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        if ((*locIt).position != pos || (*locIt).length != lenAA) return false;
        locIt++;
        if ((*locIt).position != pos + lenAA + lenBB + 1 || (*locIt).length != lenCC) return false;
        locIt++;
        if ((*locIt).position != pos + lenAA || (*locIt).length != lenBB) return false;
    }
    else if (nbComposants == 4 && firstChar == 'B') {
        //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        const unsigned int lenDD = (*compIt++).size();
        if ((*locIt).position != pos - 10 || (*locIt).length != 10) return false;
        locIt++;
        if ((*locIt).position != pos || (*locIt).length != lenAA) return false;
        locIt++;
        if ((*locIt).position != pos + lenAA + lenBB + lenCC + 2 || (*locIt).length != lenDD) return false;
        locIt++;
        if ((*locIt).position != pos + lenAA + lenBB + 1 || (*locIt).length != lenCC) return false;
    }
    else 
        //autres : (pos, taille)
        if ((*locIt).position != pos || (*locIt).length != taille) return false;
    return true;
}
////////////////////////////////////////////////////////////
