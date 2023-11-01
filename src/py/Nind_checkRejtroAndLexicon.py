#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2023 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
# Copyright: 2014-2023 LATEJCON. See LICENCE.md file that comes with this distribution
# This file is part of NIND (as "nouvelle indexation").
# NIND is free software: you can redistribute it and/or modify it under the terms of the 
# GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
# (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
# NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Less General Public License for more details.
import sys
from os import getenv, path
from NindLexiconindex import NindLexiconindex
from NindRetrolexicon import NindRetrolexicon

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print (f"""© l'ATEJCON.
Dans une base nind spécifiée par un de ses fichiers, vérifie que le lexique
et le rétro-lexique sont en cohérence.
Le système nind est expliqué dans le document LAT2017.JYS.470.
Le principe du lexique et rétrolexique dans LAT2020.JYS.491

usage   : {script} <fichier nind> 
exemple : {script} FRE.lexiconindex 
exemple : {script} FRE.lexiconindex > FRE-dislexique.txt
""")

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        nindFileName = path.abspath(sys.argv[1])
        checkRejtroAndLexicon(nindFileName)
        
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()
  
##############################
def checkRejtroAndLexicon(nindFileName):
    #calcul des noms de fichiers (remplace l'extension)
    nn = nindFileName.split('.')
    nindlexiconindexName = '.'.join(nn[:-1])+'.nindlexiconindex'
    nindretrolexiconName = '.'.join(nn[:-1]) + '.nindretrolexicon'
    #les classes
    nindLexiconindex = NindLexiconindex(nindlexiconindexName)
    nindRetrolexicon = NindRetrolexicon(nindretrolexiconName)
    # dumpe le rejtrolexique
    cptMotsVides = 0
    cptDiscordances = 0
    #trouve le max des identifiants
    (maxLexicon, dateHeureLexicon) = nindLexiconindex.donneIdentificationFichier()
    print(f'{maxLexicon} identifiants à examiner')
    for index in range(1, maxLexicon+1):
        motsSimples = nindRetrolexicon.donneMot(index)
        if len(motsSimples) == 0: 
            cptMotsVides +=1
            continue
        # retrouve l'identifiant par le lexique
        motId = nindLexiconindex.donneIdentifiant(motsSimples)
        if motId != index:
            print(motsSimples)
            print('{:06d} {:s} ({:d})'.format(index, '#'.join(motsSimples), tailleMot))
            cptDiscordances +=1
        if (index%10000) == 0:
            sys.stdout.write('%d\r'%(index))
            sys.stdout.flush()        
    nindLexiconindex.close()
    nindRetrolexicon.close()
    print(f'{maxLexicon} identifiants testés')
    print(f'{cptDiscordances} discordances')
    print(f'{cptMotsVides} mots vides')


    
if __name__ == '__main__':
        main()
