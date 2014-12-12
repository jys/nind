//
// C++ Implementation: NindTermIndex_testEcrivain
//
// Description: un test pour remplir le lexique et le fichier inverse et faire differentes mesures.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindTermIndex.h"
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
    cout<<"Programme de test de NindTermIndex en mode écrivain (pour l'indexation)."<<endl;
    cout<<"Le lexique et le fichier inverse sont créés à partir du dump de documents spécifié."<<endl;
    cout<<"(Un dump de documents est obtenu par AntindexDumpBaseByDocuments sur une base S2.)"<<endl;
    cout<<"Les fichiers lexique et inverse doivent être absents."<<endl;
    cout<<"Les documents sont indexés par paquets. La taille des paquest est spécifiée"<<endl;
    cout<<"Entre chaque traitement de paquets, le fichier est fermé puis rouvert."<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier dump documents> <taille paquet docs> <taille indirection>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt 1000 100000"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<4) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string blocSizeStr = argv[2];
    const string indirectionEntryNbStr = argv[3];
    
    const unsigned int blocSize = atoi(blocSizeStr.c_str());
    const unsigned int indirectionEntryNb = atoi(indirectionEntryNbStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique et inverse
        const size_t pos = docsFileName.find('.');
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;

        /////////////////////////////////////
        FILE *file =  fopen(lexiconFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<lexiconFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<lexiconFileName<<endl;
            return false;
        }
        file =  fopen(termindexFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<termindexFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<termindexFileName<<endl;
            return false;
        }
        /////////////////////////////////////
        cout<<"Forme le lexique et le fichier inversé avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain
        NindLexicon nindLexicon(lexiconFileName, true);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse ecrivain
        NindTermIndex *nindTermIndex = new NindTermIndex(termindexFileName, 
                                                         true, 
                                                         wordsNb, 
                                                         identification,
                                                         indirectionEntryNb);
        //la classe d'utilitaires
        NindIndexTest nindIndexTest;
        //lit le fichier dump de documents
        unsigned int docsNb = 0;
        unsigned int nbMaj = 0;
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
            //ajoute tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le terme
                list<string> componants;
                nindIndexTest.split((*wordIt).word, componants);
                //la cg
                const unsigned char cg = nindIndexTest.getCgIdent((*wordIt).cg);
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexicon.addWord(componants);
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termIndex;
                nindTermIndex->getTermIndex(id, termIndex);
                //si le terme n'existe pas encore, la liste reste vide
                list<NindTermIndex::TermCG>::iterator it1 = termIndex.begin(); 
                while (it1 != termIndex.end()) {
                    if ((*it1).cg == cg) {
                        //c'est la meme cg, on ajoute le doc 
                        list<NindTermIndex::Document> &documents = (*it1).documents;
                        //trouve la place dans la liste ordonnee
                        list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
                        while (it2 != documents.end()) {
                            //deja dans la liste, incremente la frequence
                            if ((*it2).ident == noDoc) {
                                (*it2).frequency +=1;  
                                break;
                            }
                            //insere a l'interieur de la liste
                            if ((*it2).ident > noDoc) {
                                documents.insert(it2, NindTermIndex::Document(noDoc, 1));
                                break;
                            }
                            it2++;
                        }
                        //si fin de liste, insere en fin
                        if (it2 == documents.end()) documents.push_back(NindTermIndex::Document(noDoc, 1));
                        //met a jour la frequence globale de la cg
                        (*it1).frequency +=1;
                        break;
                    }
                    it1++;
                }
                //si c'est une nouvelle cg, insere en fin de liste
                if (it1 == termIndex.end()) {
                    termIndex.push_back(NindTermIndex::TermCG(cg, 1));
                    NindTermIndex::TermCG &termCG = termIndex.back();
                    list<NindTermIndex::Document> &documents = termCG.documents;
                    documents.push_back(NindTermIndex::Document(noDoc, 1));
                }
                //ecrit sur le fichier
                nindLexicon.getIdentification(wordsNb, identification);
                nindTermIndex->setTermIndex(id, termIndex, wordsNb, identification);
                nbMaj +=1;
            }
            //ferme et rouvre
            if (docsNb % blocSize == 0) {
                nindTermIndex->dumpEmptyAreas();
                delete nindTermIndex;
                nindLexicon.getIdentification(wordsNb, identification);
                nindTermIndex = new NindTermIndex(termindexFileName, 
                                                  true, 
                                                  wordsNb, 
                                                  identification,
                                                  indirectionEntryNb);
            }
            cout<<docsNb<<"\r"<<flush;
            //nindTermIndex->dumpEmptyAreas();
            //if (docsNb == 1) break;
        }
        nindTermIndex->dumpEmptyAreas();
        docsFile.close();
        end = clock();
        cout<<nbMaj<<" mises à jour pour "<<docsNb<<" documents lus"<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
