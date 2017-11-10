#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2017 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
# Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
# This file is part of NIND (as "nouvelle indexation").
# NIND is free software: you can redistribute it and/or modify it under the terms of the 
# GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
# (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
# NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Less General Public License for more details.
import sys
from os import getenv, path
import NindLateconFile
from NindRetrolexicon import NindRetrolexicon
from NindLocalindex import NindLocalindex
import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Dumpe en clair un document préalablement indexé dans une base nind.
Les termes apparaissent dans l'ordre, les localisations ne sont pas affichées.
Le système nind est expliqué dans le document LAT2017.JYS.470.

usage   : %s <fichier lexiconindex> <n° doc>
exemple : %s FRE.lexiconindex 9546
"""%(script, script))

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    noDoc = int(sys.argv[2])
    
    #calcul des noms de fichiers (remplace l'extension)
    nn = lexiconindexFileName.split('.')
    localindexFileName = '.'.join(nn[:-1])+'.nindlocalindex'
    
    #ouvre les classes
    nindRetrolexiconindex = NindRetrolexicon(lexiconindexFileName)
    nindLocalindex = NindLocalindex(localindexFileName)

    #trouve l'identifiant interne
    if noDoc not in nindLocalindex.docIdTradExtInt: 
        print ("doc inconnu")
        sys.exit()
    identInterne = nindLocalindex.docIdTradExtInt[noDoc]
    print ('%d -> %d'%(noDoc, identInterne))
    #trouve les termes dans le fichier des index locaux et les affiche sans leurs localisations
    termList = nindLocalindex.donneListeTermes(noDoc)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        terme = nindRetrolexiconindex.donneMot(noTerme)
        resultat.append('%s [%s]'%(terme, NindLateconFile.catNb2Str(categorie)))
    print (', '.join(resultat))
   

if __name__ == '__main__':
        main()
