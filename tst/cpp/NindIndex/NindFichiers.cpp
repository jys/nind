//
// C++ Implantation: NindFichiers
//
// Description: Utilitaires pour afficher les dates en clair
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
#include "NindFichiers.h"
////////////////////////////////////////////////////////////
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Teste le systehme de fichiers
//param fileNameExtensionLess absolute path file name without extension
//param avecRetrolexicon vrai si le retrolexicon fait partie des fichiers
//param tousAbsents vrai si les fichiers doivent estre tous absents
//return vrai si le systehme est cohejrent, sinon faux */
bool NindFichiers::fichiersCohejrents(const string &fileNameExtensionLess, 
                                      const bool avecRetrolexicon,
                                      const bool tousAbsents)
{
    bool tous = true;
    bool aucun = true;
    list<string> extensions = {".nindlexiconindex", ".nindtermindex", ".nindlocalindex"};
    if (avecRetrolexicon) extensions.push_back(".nindretrolexicon");
    for (list<string>::const_iterator itext = extensions.begin(); itext != extensions.end(); itext++) {
        const string fileName = fileNameExtensionLess + (*itext);
        FILE *fichier = fopen(fileName.c_str(), "rb");
        if (fichier) {
            fclose(fichier);
            aucun = false;
        }
        else tous = false;
    }
    //les fichiers doivent tous ne pas exister
    if (tousAbsents) return aucun;
    //les fichiers doivent tous exister ou tous ne pas exister
    else return (tous || aucun);
}
////////////////////////////////////////////////////////////
