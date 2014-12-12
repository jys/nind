//
// C++ Implementation: NindIndexTest
//
// Description: Utilitaires pour les tests du fichier inverse
// Étude de la représentation du fichier inversé et des index locaux LAT2014.JYS.440
//
// Cette classe gere les utilitaires necessaires aux programmes de tests
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindIndexTest.h"
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
NindIndexTest::NindIndexTest():
    m_cgId2Str(),
    m_cgStr2Id()
{
    //les id sont ceux de la base de donnees
    //1:L_ADJ, 2:L_ADV, 3:L_CONJ, 4:L_DET, 5:L_DETERMINEUR, 6:L_DIVERS, 7:L_DIVERS_DATE, 
    //8:L_EXCLAMATION, 9:L_INTERJ, 10:L_NC, 11:L_NOMBRE, 12:L_NP, 17:L_PART, 13:L_PONCTU, 
    //14:L_PREP, 15:L_PRON, 16:L_V, 18:L_DIVERS_PARTICULE, 19:L_CLASS, 21:L_AFFIX
    //le vecteur pour id -> str
    m_cgId2Str = {"", "ADJ", "ADV", "CONJ", "DET", "DETERMINEUR", "DIVERS", "DIVERS_DATE", 
        "EXCLAMATION", "INTERJ", "NC", "NOMBRE", "NP", "PART", "PONCTU", "PREP", "PRON", "V", 
        "DIVERS_PARTICULE", "CLASS", "AFFIX"};
    //la map pour str -> id
    for (unsigned char it = 0; it != m_cgId2Str.size(); it++) m_cgStr2Id[m_cgId2Str[it]] = it;
}
////////////////////////////////////////////////////////////
NindIndexTest::~NindIndexTest()
{
}
////////////////////////////////////////////////////////////
//brief Get ident number of supplied cg string
//param cg grammatical category as string
//return ident number */
unsigned char NindIndexTest::getCgIdent(const string &cg)
    throw(EncodeErrorException)
{
    const map<string, unsigned char>::const_iterator it = m_cgStr2Id.find(cg);
    if (it == m_cgStr2Id.end()) throw EncodeErrorException("getCgIdent"); 
    return (*it).second;
}
////////////////////////////////////////////////////////////
//brief Get cg string of supplied ident number
//param ident ident number
//return grammatical category as string */
string NindIndexTest::getCgStr(const unsigned char ident)
    throw(DecodeErrorException)
{
    if (ident >= m_cgId2Str.size()) throw EncodeErrorException("getCgStr");
    return m_cgId2Str[ident];
}
////////////////////////////////////////////////////////////
//brief Extract informations from line of dump file
//param dumpLine line from dump file
//param noDoc return document ident
//param wordsList return words, cg and pos*/
void NindIndexTest::getWords(const string &dumpLine, 
                                 unsigned int &noDoc, 
//                                 list<pair<string, string> > &wordsList)
                                 list<struct WordDesc> &wordsList)
{
    //cerr<<dumpLine<<endl;
    wordsList.clear();
    //27072 <=> 1 len=12  ::  famille (NC), famille#heureux (NC), heureux (ADJ), se_ressembler (V), ..., façon (NC)
    if (dumpLine.empty()) return;   //evacue ainsi les lignes vides
    //extrait le n° de doc
    size_t posDeb = dumpLine.find("<=> ", 0);
    if (posDeb == string::npos) throw FormatFileException();


    posDeb += 4;      //avec la numerotation des docs Firebird
//    posDeb = 0;         //avec la numerotation des docs ANT'box
    size_t posSep = dumpLine.find(' ', posDeb);
    noDoc = atoi((dumpLine.substr(posDeb, posSep-posDeb)).c_str());
    posDeb = dumpLine.find("::  ", 0);
    if (posDeb == string::npos) throw FormatFileException();
    posDeb += 4;
    while (true) {
        const unsigned int pos = posDeb;
        posSep = dumpLine.find(' ', posDeb);
        const string word = dumpLine.substr(posDeb, posSep-posDeb);
        posDeb = dumpLine.find('(', posSep+1);
        posSep = dumpLine.find(')', posDeb+1);
        const string cg =  dumpLine.substr(posDeb+1, posSep-posDeb-1);
        const struct WordDesc wordDesc(word, cg, pos);
//        wordsList.push_back(make_pair(word, cg));
        wordsList.push_back(wordDesc);
        posDeb = dumpLine.find(", ", posSep + 1);
        if (posDeb == string::npos) break;
        posDeb +=2;
    }
}
////////////////////////////////////////////////////////////
//brief split words into single words
//param word composed word with "#"
//param simpleWords return list of single words */
void NindIndexTest::split(const string &word, 
                              list<string> &simpleWords)
{
    simpleWords.clear();
    size_t posDeb = 0;
    while (true) {
        const size_t posSep = word.find('#', posDeb);
        if (posSep == string::npos) {
            //separateur pas trouve
            if (posDeb != word.size()) simpleWords.push_back(word.substr(posDeb));
            break;
        }
        //separateur trouve
        simpleWords.push_back(word.substr(posDeb, posSep-posDeb));
        posDeb = posSep + 1;
    }
}
////////////////////////////////////////////////////////////
