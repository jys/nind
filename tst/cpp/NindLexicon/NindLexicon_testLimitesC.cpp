//
// C++ Implementation: NindLexicon_testLimites
//
// Description: un test pour remplir le lexique aux limites et faire differentes mesures.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexicon/NindLexicon.h"
#include "NindExceptions.h"
#include <time.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
#include <string.h>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
static void displayHelp(char* arg0) {
    cout<<"© l'ATÉCON"<<endl;
    cout<<"Programme de test de NindLexicon."<<endl;
    cout<<"Charge un lexique vide avec un fichier de mots simples"<<endl;
    cout<<"et les multiplie en changeant la dernière lettre"<<endl;
    cout<<"et les combine pour en faire des mots composés."<<endl;
    cout<<"Il y a environ 10 mots composés pour un mot simple."<<endl;
    cout<<"Le fichier lexique s'appelle limites.lexicon."<<endl;
    cout<<"Permet de mesurer l'occupation mémoire du lexique"<<endl;
    cout<<"en fonction du nombre de mots."<<endl;

    cout<<"usage: "<<arg0<<" --help"<< endl;
    cout<<"       "<<arg0<<" <fichier de mots simples>"<<endl;
    cout<<"ex :   "<<arg0<<" frelemmes.csv"<<endl;
}
////////////////////////////////////////////////////////////
#define LINE_SIZE 128
#define LEXICON_NAME "limites.lexicon"
////////////////////////////////////////////////////////////
static void displayChar(const struct NindLexicon::LexiconChar &lexiconChar, const string &title);
static string asciiDate(const time_t date);
static long process_size_in_pages(void);
static int parseLine(char* line);
static int getVmSize();
////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    setlocale( LC_ALL, "French" );
    if (argc<2) {displayHelp(argv[0]); return false;}
    const string wordsFileName = argv[1];
    if (wordsFileName == "--help") {displayHelp(argv[0]); return true;}

    try {
        //ouvre le lexique en ecrivain
        NindLexicon nindLexicon(LEXICON_NAME, true);
        struct NindLexicon::LexiconChar lexiconChar;
        bool isOk = nindLexicon.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "VIDE ");
        unsigned int wordsNb = 0;
        list<string> last10;
        char charBuff[LINE_SIZE];
        ifstream wordsFile(wordsFileName.c_str(), ifstream::in);
        if (wordsFile.fail()) throw OpenFileException(wordsFileName);
        //lit le fichier des mots simples
        while (wordsFile.good()) {
            wordsFile.getline(charBuff, LINE_SIZE);
            wordsNb++;
            list<string> componants;
            //memorise
            last10.push_back(string(charBuff));
            //tous les 10 mots simples, fabrique 1000 mots composes
            if (wordsNb%10 == 0) {
                //const string terminaisons ("abcdefghijklmnopqrstuvwxyz1234567890éèêàâôîĵ");
                const string terminaisons ("abcdefghijk");
                for (string::const_iterator it1 = terminaisons.begin(); it1 != terminaisons.end(); ++it1) {
                    //change la dernière lettre de tous les mots
                    list<string> words10;
                    for (list<string>::const_iterator it2 = last10.begin(); it2 != last10.end(); it2++) {
                        string word = (*it2);
                        words10.push_back(word.replace(word.end()-1, word.end(), 1, (*it1)));
                    }
                    for (list<string>::const_iterator it3 = words10.begin(); it3 != words10.end(); it3++) {
                        //ecrit d'abord le mot simple
                        componants.push_back(*it3);					//10 mots de 1
                        const unsigned int id = nindLexicon.addWord(componants);
//                         for (list<string>::const_iterator it4 = words10.begin(); it4 != words10.end(); it4++) {
//                             componants.push_back(*it4);
//                             const unsigned int id = nindLexicon.addWord(componants); 	//100 mots de 2
// //                             for (list<string>::const_iterator it5 = words10.begin(); it5 != words10.end(); it5++) {
// //                                 componants.push_back(*it5);
// //                                 const unsigned int id = nindLexicon.addWord(componants); //1000 mots de 3
// //                                 componants.pop_back();
// //                             }
//                             componants.pop_back();
//                         }
                        componants.pop_back();
                    }
                }
                last10.clear();
            }
            if (wordsNb%1000 == 0) { cout<<wordsNb<<"\r"; cout.flush(); }
            if (wordsNb%100000 == 0) {
                bool isOk = nindLexicon.integrityAndCounts(lexiconChar);
                displayChar(lexiconChar, "VIDE ");
                //return true;
            }
        }
        isOk = nindLexicon.integrityAndCounts(lexiconChar);
        displayChar(lexiconChar, "PLEIN");
        
        return true;
    }
    catch (FileException &exc) {cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; return false;}
    catch (exception &exc) {cerr<<"EXCEPTION :"<<exc.what()<< endl; return false;}
    catch (...) {cerr<<"EXCEPTION unknown"<< endl; return false; }
}
////////////////////////////////////////////////////////////
static void displayChar(const struct NindLexicon::LexiconChar &lexiconChar, const string &title)
{
    cout<<lexiconChar.isOk<<" <"<<lexiconChar.swNb<<", "<<lexiconChar.cwNb;
    cout<<"> mots, <"<<lexiconChar.wordsNb<<", "<<lexiconChar.identification;
    //cout<<">("<<asciiDate((time_t)lexiconChar.identification)<<")  ";
    cout<<">  ";
    cout<<process_size_in_pages()<<" pages, ";
    cout<<getVmSize()<<" ko"<<endl;
}
////////////////////////////////////////////////////////////
static string asciiDate(const time_t date)
{
    const string ascDate(ctime(&date));
    const string::size_type  nowStop = ascDate.rfind(' ');
    return ascDate.substr(0,nowStop+5);
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
