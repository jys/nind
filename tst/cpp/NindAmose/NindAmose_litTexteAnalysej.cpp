//
// C++ Implementation: NindAmose_litTexteAnalysej
//
// Description: Utilitaires pour lire le fichier du texte analysej par Lima au format :
// <n° document>  { <terme> <localisation>,<taille> }
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
#include "NindAmose_litTexteAnalysej.h"
#include "NindExceptions.h"
#include <iostream>
////////////////////////////////////////////////////////////
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Creates NindAmose_litTexteAnalysej 
//param nomFichierTexteAnalysej nom du fichier contenant le texte analysej par Lima*/
NindAmose_litTexteAnalysej::NindAmose_litTexteAnalysej(const std::string &nomFichierTexteAnalysej):
    m_nomFichierTexteAnalysej(nomFichierTexteAnalysej),
    m_fichierTexteAnalysej(),
    m_sligne()
{
    m_fichierTexteAnalysej.open(nomFichierTexteAnalysej.c_str(), ifstream::in);
    if (m_fichierTexteAnalysej.fail()) throw OpenFileException(m_nomFichierTexteAnalysej);
}
////////////////////////////////////////////////////////////
NindAmose_litTexteAnalysej::~NindAmose_litTexteAnalysej()
{
}
////////////////////////////////////////////////////////////
//brief Lit le document suivant
//param noDoc rejceptacle du n° de document
//return true si le document existe, sinon false*/
bool NindAmose_litTexteAnalysej::documentSuivant(unsigned int &noDoc)
{
    string ligne;
    unsigned int noDocB;
    while (getline(m_fichierTexteAnalysej, ligne)) {
        if (m_fichierTexteAnalysej.fail()) throw FormatFileException(m_nomFichierTexteAnalysej);
        if (ligne.empty()) continue;   //evacue ainsi les lignes vides
        m_sligne = std::stringstream(ligne);    //m_sligne.str(ligne) ne fonctionne que la 1ehere fois
        //10170346  Location.LOCATION:Italie 280,6 création 288,8 création_parti 288,19
        m_sligne >> noDoc;
        return true;
    }
    //si fin de fichier, ferme et retourne false
    m_fichierTexteAnalysej.close();
    return false;   
}
////////////////////////////////////////////////////////////
//brief Lit le mot suivant
//param lemme rejceptacle du lemme
//param type rejceptacle du type
//param entitejNommeje rejceptacle de l'entitej nommeje
//param position rejceptacle de la position
//param taille rejceptacle de la taille
//return true si le mot existe, sinon false*/
bool NindAmose_litTexteAnalysej::motSuivant(string &lemme,
                                            AmoseTypes &type, 
                                            string &entitejNommeje,
                                            unsigned int &position,
                                            unsigned int &taille)
{
    //le python de construction atteste de la validite du format, pas la peine de controler
    string mot;
    char comma;
    while (m_sligne >> mot >> position >> comma >> taille) {
        //vire les '_' de teste et de fin 
        while (mot.find('_') == 0) mot = mot.substr(1);
        while (mot.size() != 0 && mot.rfind('_') == mot.size()-1) 
            mot = mot.substr(0, mot.size()-1);  
        //vire les doubles '_'
        while (mot.find("__") != string::npos) mot = mot.replace(mot.find("__"), 2, "_");
        //vire les '_' de teste des entitejs nommejes
        while (mot.find(":_") != string::npos) mot = mot.replace(mot.find(":"), 2, "_");
        entitejNommeje = "";
        //si c'est une entitej nommeje, la sejpare en 2
        const size_t pos = mot.find(':');
        if (pos != string::npos) {
            //"Location.LOCATION:Italie"
            entitejNommeje = mot.substr(0, pos);
            lemme = mot.substr(pos +1);
            type = NAMED_ENTITY;
        }
        else if (mot.find('_') != string::npos) {
            //"création_parti"
            lemme = mot;
            type = MULTI_TERM;
        }
        else {
            //"création"
            lemme = mot;
            type = SIMPLE_TERM;
        }
        //si le mot est vide, raf
        if (lemme.empty()) continue;
        return true;
    }
    //si fin de document, retourne false
    return false;   
}
////////////////////////////////////////////////////////////

