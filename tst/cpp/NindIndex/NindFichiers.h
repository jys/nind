//
// C++ Interface: NindFichiers
//
// Description: Utilitaires pour afficher les dates en clair
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
#ifndef NindFichiers_H
#define NindFichiers_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include <list>
#include <string>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindFichiers {
public:
    /**\brief Teste le systehme de fichiers
    *\param nomsFichiers liste de tous les fichiers à tester la prejsence
    *\param tousAbsents vrai si les fichiers doivent estre tous absents
    *\return vrai si le systehme est cohejrent, sinon faux */
    static bool fichiersCohejrents(const std::list<std::string> &nomsFichiers, 
                                   const bool tousAbsents);
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
