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
from os import path, getenv
import codecs
import datetime
import time
from NindRetrolexicon import NindRetrolexicon

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Dumpe en clair le lexique d'une base nind (version fichier).
Les mots, simples et composés, apparaissent dans l'ordre d'indexation.
(Ne pas confondre avec NindLexiconindex.py qui dumpe la structure
interne du lexique.)
Le système nind est expliqué dans le document LAT2017.JYS.470.
Le fichier de sortie s'appelle <fichier retrolexicon>-dumpall.txt

usage   : %s <fichier retrolexicon> 
exemple : %s FRE.nindretrolexicon
"""%(script, script))

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    retrolexiconFileName = path.abspath(sys.argv[1])
    outFileName = '%s-dumpall.txt'%(retrolexiconFileName)
    
    #ouvre les classes
    nindRetrolexicon = NindRetrolexicon(retrolexiconFileName)

    (maxIdentifiant, dateHeure) = nindRetrolexicon.donneIdentificationFichier()
    print ("max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure))))
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    noWord = 1
    for noWord in range(1, maxIdentifiant+1):
        word = nindRetrolexicon.donneMot(noWord)
        outFile.write('%06d  %s\n'%(noWord, word))
    outFile.close()
    print ('%d mots écrits sur %s'%(maxIdentifiant, outFileName))

if __name__ == '__main__':
        main()
