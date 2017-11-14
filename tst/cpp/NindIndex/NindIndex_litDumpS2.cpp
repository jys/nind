//
// C++ Implantation: NindIndex_litDumpS2
//
// Description: Utilitaires pour lire le fichier du texte analysej par S2 et dumpej 
// une ligne par document au format :
// <n°doc ANT'box> " <=> " <n°doc Firebird> " len=" <nb termes> "  ::  " <lemme> " (" <cg> ") " { "," <lemme> " (" <cg> ") " }
// "27072 <=> 1 len=12  ::  famille (NC), famille#heureux (NC), heureux (ADJ), se_ressembler (V), ..., façon (NC)"
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
#include "NindIndex_litDumpS2.h"
#include "NindExceptions.h"
#include <iostream>
////////////////////////////////////////////////////////////
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creje NindIndex_litDumpS2 
NindIndex_litDumpS2::NindIndex_litDumpS2():
    m_nomFichierDumpS2(),
    m_fichierDumpS2(),
    m_sligne(),
    m_cgId2Str(),
    m_cgStr2Id()
{
    initConversion();
}
////////////////////////////////////////////////////////////
//brief Creje NindIndex_litDumpS2 
//param nomFichierDumpS2 nom du fichier contenant le texte dumpej de S2*/
NindIndex_litDumpS2::NindIndex_litDumpS2(const string &nomFichierDumpS2):
    m_nomFichierDumpS2(nomFichierDumpS2),
    m_fichierDumpS2(),
    m_sligne(),
    m_cgId2Str(),
    m_cgStr2Id()
{
    m_fichierDumpS2.open(m_nomFichierDumpS2.c_str(), ifstream::in);
    if (m_fichierDumpS2.fail()) throw OpenFileException(m_nomFichierDumpS2);
    initConversion();
}
////////////////////////////////////////////////////////////
NindIndex_litDumpS2::~NindIndex_litDumpS2()
{
}
////////////////////////////////////////////////////////////
//brief Initialise la map de conversion
void NindIndex_litDumpS2::initConversion()
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
//brief Lit le document suivant
//param noDocAnt rejceptacle du n° de document Ant'box
//param noDocFb rejceptacle du n° de document Firebird
//return true si le document existe, sinon false*/
bool NindIndex_litDumpS2::documentSuivant(unsigned int &noDocAnt,
                                          unsigned int &noDocFb)
{
    //le programme de construction atteste de la validitej du format, pas la peine de controsler
    string ligne;
    string sejparateur;
    while (getline(m_fichierDumpS2, ligne)) {
        if (m_fichierDumpS2.fail()) throw FormatFileException(m_nomFichierDumpS2);
        if (ligne.empty()) continue;   //evacue ainsi les lignes vides
        m_sligne = std::stringstream(ligne);    //m_sligne.str(ligne) ne fonctionne que la 1ehere fois
        //"27072 <=> 1 len=12  ::  famille (NC), famille#heureux (NC), heureux (ADJ), se_ressembler (V), ..., façon (NC)"
        m_sligne >> noDocAnt;
        m_sligne >> sejparateur;        //<=>
        m_sligne >> noDocFb;  
        m_sligne >> sejparateur;        //len=12
        m_sligne >> sejparateur;        //::
        return true;
    }
    //si fin de fichier, ferme et retourne false
    m_fichierDumpS2.close();
    return false;   
}
////////////////////////////////////////////////////////////
//brief Lit le mot suivant
//param composants rejceptacle du tableau des composants du lemme
//param cg rejceptacle de la catejgorie grammaticale
//param position rejceptacle de la position
//param taille rejceptacle de la taille
//return true si le mot existe, sinon false*/
bool NindIndex_litDumpS2::motSuivant(list<string> &composants,
                                     unsigned char &cg, 
                                     list<pair<unsigned int, unsigned int> > &localisation)
{
    //le programme de construction atteste de la validitej du format, pas la peine de controsler
    //la position est inventeje parce que nous n'avons pas le texte d'origine
    //elle est arbitrairement la position de fin dans la chaisne
    const unsigned int position = m_sligne.tellg();
    string mot;
    string catejcorie;
    while (m_sligne >> mot >> catejcorie) {
        //sejpare le mot en ses composants simples
        composants.clear();
        split(mot, composants);
        //trouve la cg
        catejcorie = catejcorie.substr(1, catejcorie.find(')')-1);
        const map<string, unsigned char>::const_iterator it = m_cgStr2Id.find(catejcorie);
        if (it == m_cgStr2Id.end()) throw EncodeErrorException("Catejgorie inconnue : " +  catejcorie); 
        cg = (*it).second;
        //la taille est celle du mot d'origine
        const unsigned int taille = mot.size();
        //cerr<<"mot=/"<<mot<<"/ cg="<<(unsigned int)cg<<" position="<<position<<" taille="<<taille<<endl;
        //simulation de cas reel de termes en plusieurs parties (c'est tout a fait arbitraire)
        //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
        //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
        //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
        //autres : (pos, taille)
        localisation.clear();
        const unsigned int nbComposants =  composants.size();
        list<string>::const_iterator compIt = composants.begin();
        const string &firstComp = (*compIt++);
        const char firstChar = firstComp.front();
        if (nbComposants == 2 && firstChar == 'T') {
            //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
            const unsigned int lenAA = firstComp.size();
            const unsigned int lenBB = (*compIt++).size();
            localisation.push_back(make_pair(position, lenAA));
            localisation.push_back(make_pair(position + lenAA, lenBB));
        }
        else if (nbComposants == 3 && firstChar == 'k') {
            //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
            const unsigned int lenAA = firstComp.size();
            const unsigned int lenBB = (*compIt++).size();
            const unsigned int lenCC = (*compIt++).size();
            localisation.push_back(make_pair(position, lenAA));
            localisation.push_back(make_pair(position + lenAA + lenBB + 1, lenCC));
            localisation.push_back(make_pair(position + lenAA, lenBB));        
        }
        else if (nbComposants == 4 && firstChar == 'B') {
            //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
            const unsigned int lenAA = firstComp.size();
            const unsigned int lenBB = (*compIt++).size();
            const unsigned int lenCC = (*compIt++).size();
            const unsigned int lenDD = (*compIt++).size();
            localisation.push_back(make_pair(position - 10, 10));
            localisation.push_back(make_pair(position, lenAA));
            localisation.push_back(make_pair(position + lenAA + lenBB + lenCC + 2, lenDD));
            localisation.push_back(make_pair(position + lenAA + lenBB + 1, lenCC));
        }
        else {
            //autres : (pos, taille)
            localisation.push_back(make_pair(position, taille));
        }
        return true;
    }
    //si fin de document, retourne false
    return false;   
}
////////////////////////////////////////////////////////////
//brief Get ident number of supplied cg string
//param cg grammatical category as string
//return ident number */
unsigned char NindIndex_litDumpS2::getCgIdent(const string &cg)
{
    const map<string, unsigned char>::const_iterator it = m_cgStr2Id.find(cg);
    if (it == m_cgStr2Id.end()) throw EncodeErrorException("getCgIdent"); 
    return (*it).second;
}
////////////////////////////////////////////////////////////
//brief Get cg string of supplied ident number
//param ident ident number
//return grammatical category as string */
string NindIndex_litDumpS2::getCgStr(const unsigned char ident)
{
    if (ident >= m_cgId2Str.size()) throw EncodeErrorException("getCgStr");
    return m_cgId2Str[ident];
}
////////////////////////////////////////////////////////////
//brief split words into single words
//param word composed word with "#"
//param simpleWords return list of single words */
void NindIndex_litDumpS2::split(const string &word, 
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
