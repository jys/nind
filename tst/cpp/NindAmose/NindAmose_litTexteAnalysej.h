//
// C++ Interface: NindAmose_litTexteAnalysej
//
// Description: Utilitaires pour lire le fichier du texte analysej par Lima au format :
// <n° document>  { <terme> <localisation>,<taille> }
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
//
// Copyright: 2017 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindAmose_litTexteAnalysej_H
#define NindAmose_litTexteAnalysej_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include "NindAmose/NindLexiconAmose.h"
#include <fstream> 
#include <sstream>
#include <string>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindAmose_litTexteAnalysej {
public:
    /**\brief Creje NindAmose_litTexteAnalysej 
    *\param nomFichierTexteAnalysej nom du fichier contenant le texte analysej par Lima*/
    NindAmose_litTexteAnalysej(const std::string &nomFichierTexteAnalysej);

    virtual ~NindAmose_litTexteAnalysej();
    
    /**\brief Lit le document suivant
    *\param noDoc rejceptacle du n° de document
    *\return true si le document existe, sinon false*/
    bool documentSuivant(unsigned int &noDoc);
    
    /**\brief Lit le mot suivant
    *\param lemme rejceptacle du lemme
    *\param type rejceptacle du type
    *\param entitejNommeje rejceptacle de l'entitej nommeje
    *\param position rejceptacle de la position
    *\param taille rejceptacle de la taille
    *\return true si le mot existe, sinon false*/
    bool motSuivant(std::string &lemme,
                    AmoseTypes &type, 
                    std::string &entitejNommeje,
                    unsigned int &position,
                    unsigned int &taille);
    
private:
    std::string m_nomFichierTexteAnalysej;
    std::ifstream m_fichierTexteAnalysej;
    std::stringstream m_sligne;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
