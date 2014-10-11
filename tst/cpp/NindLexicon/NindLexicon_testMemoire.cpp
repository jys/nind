//
// C++ Implementation: NindLexicon_testMemoire
//
// Description: un test pour lire le lexique fichier et mesurer la consommation memoire.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) L'ATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <string.h>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"Programme de test de NindLexicon en mode lecteur (pour la recherche)."<<endl;
    cout<<"Charge un lexique avec le fichier lexique et mesure le temps"<<endl;
    cout<<"de chargement et la mémoire consommée"<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier lexique>"<<endl;
    cout<<"ex :   "<<arg0<<" fre-boxon.lexicon"<<endl;
}
////////////////////////////////////////////////////////////
static long process_size_in_pages(void);
static int parseLine(char* line);
static int getVmSize();
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string lexiconFileName = argv[1];
    if (lexiconFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //pour calculer le temps consomme
        clock_t start, end;
        double cpuTimeUsed;
        start = clock();
        //affiche la memoire avant
        cout<<"Mémoire virtuelle avant : "<<process_size_in_pages()<<" pages, "<<getVmSize()<<" ko"<<endl;
        //le lexique lecteur
        NindLexicon nindLexicon(lexiconFileName, false);
        cout<<"Mémoire virtuelle après : "<<process_size_in_pages()<<" pages, "<<getVmSize()<<" ko"<<endl;
        /////////////////////////////////////
        cout<<"1) vérifie l'intégrité du lexique"<<endl;
        struct NindLexicon::LexiconChar lexiconSizes;
        const bool isOk = nindLexicon.integrityAndCounts(lexiconSizes);
        end = clock();
        cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //integrite
        if (isOk) cout<<"lexique OK ";
        else cout<<"lexique NOK ";
        //nombres de mots
        cout<<lexiconSizes.swNb<<" mots simples, ";
        cout<<lexiconSizes.cwNb<<" mots composés"<<endl;
        cout<<cpuTimeUsed<<" secondes CPU"<<endl;
        return true;
    }
    catch (LexiconException &exc) {cerr<<"EXCEPTION :"<<exc.m_word<<" "<<exc.what()<<endl; return false;}
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static long process_size_in_pages(void)
{
   long s = -1;
   FILE *f = fopen("/proc/self/statm", "r");
   if (!f) return -1;
   // if for any reason the fscanf fails, s is still -1,
   //      with errno appropriately set.
   fscanf(f, "%ld", &s);
   fclose (f);
   return s;
}
////////////////////////////////////////////////////////////
static int parseLine(char* line){
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
}
static int getVmSize(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
////////////////////////////////////////////////////////////
