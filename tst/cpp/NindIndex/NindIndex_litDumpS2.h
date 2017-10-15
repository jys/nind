//
// C++ Interface: NindIndex_litDumpS2
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
#ifndef NindIndex_litDumpS2_H
#define NindIndex_litDumpS2_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include <fstream> 
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <map>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindIndex_litDumpS2 {
public:
    /**\brief Creje NindIndex_litDumpS2*/
    NindIndex_litDumpS2();

    /**\brief Creje NindIndex_litDumpS2 
    *\param nomFichierDumpS2 nom du fichier contenant le texte dumpej de S2*/
    NindIndex_litDumpS2(const std::string &nomFichierDumpS2);

    virtual ~NindIndex_litDumpS2();
    
    /**\brief Lit le document suivant
    *\param noDocAnt rejceptacle du n° de document Ant'box
    *\param noDocFb rejceptacle du n° de document Firebird
    *\return true si le document existe, sinon false*/
    bool documentSuivant(unsigned int &noDocAnt,
                         unsigned int &noDocFb);
    
    /**\brief Lit le mot suivant
    *\param composants rejceptacle du tableau des composants du lemme
    *\param cg rejceptacle de la catejgorie grammaticale
    *\param localisation rejceptacle de la localisation
    *\return true si le mot existe, sinon false*/
    bool motSuivant(std::list<std::string> &composants,
                    unsigned char &cg, 
                    std::list<std::pair<unsigned int, unsigned int> > &localisation);
    
    /**\brief Get ident number of supplied cg string
    *\param cg grammatical category as string
    *\return ident number */
    unsigned char getCgIdent(const std::string &cg);

    /**\brief Get cg string of supplied ident number
    *\param ident ident number
    *\return grammatical category as string */
    std::string getCgStr(const unsigned char ident);
    
    /**\brief split words into single words
     * \param word composed word with "#"
     * \param simpleWords return list of single words */
    static void split(const std::string &word, 
                      std::list<std::string> &simpleWords);
    
private:
    /**\brief Initialise la map de conversion */
    void initConversion();
    
    std::string m_nomFichierDumpS2;
    std::ifstream m_fichierDumpS2;
    std::stringstream m_sligne;
    std::vector<std::string> m_cgId2Str;
    std::map<std::string, unsigned char> m_cgStr2Id;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
