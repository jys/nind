//
// C++ Implementation: NindIndex_testEcrivain
//
// Description: un test pour remplir le lexique, le fichier inverse et le fichier des index locaux
// et faire differentes mesures.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
//#include "NindLexicon/NindLexicon.h"
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindIndexTest.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindIndex en mode écrivain (pour l'indexation)."<<endl;
    cout<<"Le lexique et les fichiers inverse et d'index locaux sont créés à partir"<<endl;
    cout<<"du dump de documents spécifié."<<endl;
    cout<<"(AntindexDumpBaseByDocuments crée un dump d'une base S2.)"<<endl;
    cout<<"Les fichiers lexique, inverse et d'index locaux doivent être absents."<<endl;
    cout<<"Les documents sont indexés par paquets. La taille des paquest est spécifiée"<<endl;
    cout<<"Entre chaque traitement de paquets, les fichiers sont fermés puis rouverts."<<endl;
    cout<<"Le nombre d'entrées des blocs d'indirection est spécifiée pour le fichier inversé"<<endl;
    cout<<"et le fichier des index locaux."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <dump documents> <paquet docs> <taille lexique> <taille inverse> <taille locaux>"<<endl;
    cout<<"ex :   "<<arg0<<" FRE.FDB-DumpByDocuments.txt 1000 100003 100000 5000"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 65536*100
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int id,
                        const unsigned int cg,
                        const unsigned int noDoc,
                        list<NindTermIndex::TermCG> &termIndex); 
//Construit la definition d'un fichier pour l'index local
static void majLocal(const unsigned int id,
                     const unsigned int cg,
                     const unsigned int pos,
                     const unsigned int taille,
                     const list<string> &componants,
                     list<NindLocalIndex::Term> &localIndex);
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<6) {displayHelp(argv[0]); return false;}
    const string docsFileName = argv[1];
    if (docsFileName == "--help") {displayHelp(argv[0]); return true;}
    const string blocSizeStr = argv[2];
    const string lexiconEntryNbStr = argv[3];
    const string inverseEntryNbStr = argv[4];
    const string localEntryNbStr = argv[5];
    
    const unsigned int blocSize = atoi(blocSizeStr.c_str());
    const unsigned int lexiconEntryNb = atoi(lexiconEntryNbStr.c_str());
    const unsigned int inverseEntryNb = atoi(inverseEntryNbStr.c_str());
    const unsigned int localEntryNb = atoi(localEntryNbStr.c_str());
    
    try {
        //calcule les noms des fichiers lexique et inverse et index locaux
        const size_t pos = docsFileName.find('.');
//        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexicon";
        const string lexiconFileName = docsFileName.substr(0, pos) + ".lexiconindex";
        const string termindexFileName = docsFileName.substr(0, pos) + ".termindex";
        const string localindexFileName = docsFileName.substr(0, pos) + ".localindex";
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
        file =  fopen(localindexFileName.c_str(), "rb");
        if (file) {
            fclose(file);
            cout<<localindexFileName<<" existe !"<<endl;
            cout<<"Veuillez l'effacer par la commande : rm "<<localindexFileName<<endl;
            return false;
        }
        /////////////////////////////////////
        cout<<"Forme le lexique, le fichier inversé et le fichier des index locaux avec "<<docsFileName<<endl;
        start = clock();
        //le lexique ecrivain
//        NindLexicon nindLexicon(lexiconFileName, true);
        NindLexiconIndex nindLexicon(lexiconFileName, true, lexiconEntryNb);
        unsigned int wordsNb, identification;
        nindLexicon.getIdentification(wordsNb, identification);
        //le fichier inverse ecrivain
        NindTermIndex *nindTermIndex = new NindTermIndex(termindexFileName, 
                                                         true, 
                                                         wordsNb, 
                                                         identification,
                                                         inverseEntryNb);
        //le fichier des index locaux
        NindLocalIndex *nindLocalIndex = new NindLocalIndex(localindexFileName, 
                                                            true, 
                                                            wordsNb, 
                                                            identification,
                                                            localEntryNb);
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
            //la structure d'index locaux se fabrique pour un document complet
            list<NindLocalIndex::Term> localIndex;
            //la position absolue dans le fichier source du terme precedent
            //(le dump est considere comme fichier source parce que nous n'avons pas les vrais fichiers sources)
            //prend tous les mots à la suite et dans l'ordre
            for (list<NindIndexTest::WordDesc>::const_iterator wordIt = wordsList.begin(); 
                 wordIt != wordsList.end(); wordIt++) {
                //le mot 
                const string word = (*wordIt).word;
                //le terme
                list<string> componants;
                nindIndexTest.split(word, componants);
                //la cg
                const unsigned char cg = nindIndexTest.getCgIdent((*wordIt).cg);
                //recupere l'id du terme dans le lexique, l'ajoute eventuellement
                const unsigned int id = nindLexicon.addWord(componants);
                //recupere l'index inverse pour ce terme
                list<NindTermIndex::TermCG> termIndex;
                //met a jour la definition du terme
                nindTermIndex->getTermIndex(id, termIndex);
                //si le terme n'existe pas encore, la liste reste vide
                majInverse(id, cg, noDoc, termIndex); 
                //recupere l'identification du lexique
                nindLexicon.getIdentification(wordsNb, identification);
                //ecrit sur le fichier inverse
                nindTermIndex->setTermIndex(id, termIndex, wordsNb, identification);
                nbMaj +=1;
                //augmente l'index local 
                const unsigned int pos = (*wordIt).pos;
                const unsigned int taille = word.size();
                majLocal(id, cg, pos, taille, componants, localIndex);
            }
            //ecrit la definition sur le fichier des index locaux
            nindLocalIndex->setLocalIndex(noDoc, localIndex, wordsNb, identification);
            //ferme et rouvre
            if (docsNb % blocSize == 0) {
                //nindTermIndex->dumpEmptyAreas();
                //nindLocalIndex->dumpEmptyAreas();
                delete nindTermIndex;
                delete nindLocalIndex;
                nindLexicon.getIdentification(wordsNb, identification);
                nindTermIndex = new NindTermIndex(termindexFileName, 
                                                  true, 
                                                  wordsNb, 
                                                  identification,
                                                  inverseEntryNb);
                nindLocalIndex = new NindLocalIndex(localindexFileName, 
                                                    true, 
                                                    wordsNb, 
                                                    identification,
                                                    localEntryNb);
            }
            cout<<docsNb<<"\r"<<flush;
            //nindTermIndex->dumpEmptyAreas();
            //if (docsNb == 1) break;
        }
        //nindTermIndex->dumpEmptyAreas();
        docsFile.close();
        end = clock();
        cout<<nbMaj<<" mises à jour sur "<<termindexFileName<<endl;
        cout<<docsNb<<" mises à jour sur "<<localindexFileName<<endl;
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<cpuTimeUsed<<" secondes"<<endl;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int id,
                        const unsigned int cg,
                        const unsigned int noDoc,
                        list<NindTermIndex::TermCG> &termIndex) 
{
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
}
////////////////////////////////////////////////////////////
//Construit la definition d'un fichier pour l'index local
static void majLocal(const unsigned int id,
                     const unsigned int cg,
                     const unsigned int pos,
                     const unsigned int taille,
                     const list<string> &componants,
                     list<NindLocalIndex::Term> &localIndex)
{
    //simulation de cas reel de termes en plusieurs parties (c'est tout a fait arbitraire)
    //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
    //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
    //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
    //autres : (pos, taille)
    localIndex.push_back(NindLocalIndex::Term(id, cg));
    NindLocalIndex::Term &term = localIndex.back();
    const unsigned int nbComposants =  componants.size();
    list<string>::const_iterator compIt = componants.begin();
    const string &firstComp = (*compIt++);
    const char firstChar = firstComp.front();
    if (nbComposants == 2 && firstChar == 'T') {
        //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA, lenBB));
    }
    else if (nbComposants == 3 && firstChar == 'k') {
        //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + 1, lenCC));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA, lenBB));        
    }
    else if (nbComposants == 4 && firstChar == 'B') {
        //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        const unsigned int lenDD = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos - 10, 10));
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + lenCC + 2, lenDD));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + 1, lenCC));
    }
    else 
        //autres : (pos, taille)
        term.localisation.push_back(NindLocalIndex::Localisation(pos, taille));
}
////////////////////////////////////////////////////////////
