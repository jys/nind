#!/usr/bin/env python3
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
from NindRetrolexicon import NindRetrolexicon
from NindLocalindex import NindLocalindex
import NindFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Dumpe en clair un document préalablement indexé dans une base nind spécifiée
par un de ses fichiers.
Les termes apparaissent dans l'ordre par paquets de 10 sur une ligne.
Les localisations ne sont pas affichées.
Le nombre de termes trouvés est affiché à la fin.
Le système nind est expliqué dans le document LAT2017.JYS.470.

usage   : %s <fichier nind> <n° doc>
exemple : %s FRE.lexiconindex 9546
exemple : %s FRE.lexiconindex 9546 > 9546-dump.txt
"""%(script, script, script))

def main():
    try:
        if len(sys.argv) < 3 : raise Exception()
        lexiconindexFileName = path.abspath(sys.argv[1])
        noDoc = int(sys.argv[2])
        
        #calcul des noms de fichiers (remplace l'extension)
        nn = lexiconindexFileName.split('.')
        nindlocalindexName = '.'.join(nn[:-1]) + '.nindlocalindex'
        nindretrolexiconName = '.'.join(nn[:-1]) + '.nindretrolexicon'
        
        #les classes
        nindLocalindex = NindLocalindex(nindlocalindexName)
        nindRetrolexicon = NindRetrolexicon(nindretrolexiconName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()

    #trouve l'identifiant interne
    if noDoc not in nindLocalindex.docIdTradExtInt: 
        print ("doc inconnu")
        sys.exit()
    identInterne = nindLocalindex.docIdTradExtInt[noDoc]
    print ('noDoc -> identInterne : %d -> %d'%(noDoc, identInterne))
    #trouve les termes dans le fichier des index locaux et les affiche sans leurs localisations
    termList = nindLocalindex.donneListeTermes(noDoc)
    resultat = []
    cmptTermes = 0
    for (noTerme, categorie, localisationsList) in termList:
        terme = nindRetrolexicon.donneMot(noTerme)[0]
        if categorie == 0:
            resultat.append('%s'%(terme))
        else:
            resultat.append('%s [%s]'%(terme, NindFile.catNb2Str(categorie)))
        cmptTermes +=1
        if cmptTermes %10 == 0: 
            print (', '.join(resultat) + ',')
            resultat.clear()
    print (', '.join(resultat))
    print ('\n {} occurrences de termes trouvées'.format(cmptTermes))
   

if __name__ == '__main__':
        main()
