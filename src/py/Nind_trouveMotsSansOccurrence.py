#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2019 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2019
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
from NindTermindex import NindTermindex
from NindRetrolexicon import NindRetrolexicon

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Trouve les mots du lexique qui n'ont aucune occurrence dans le corpus indexé
et ne sont donc que des composants de mots composés ou alors une rémanence
de documents effacés.
Le corpus indexé est désigné par son fichier lexique.

usage   : %s <fichier>
exemple : %s FRE.nindlexiconindex
"""%(script, script))

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        lexiconindexFileName = path.abspath(sys.argv[1])
        
        #calcul des noms de fichiers (remplace l'extension)
        nn = lexiconindexFileName.split('.')
        nindtermindexName = '.'.join(nn[:-1]) + '.nindtermindex'
        nindretrolexiconName = '.'.join(nn[:-1]) + '.nindretrolexicon'
        
        #les classes
        nindTermindex = NindTermindex(nindtermindexName)
        nindRetrolexicon = NindRetrolexicon(nindretrolexiconName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()
        
    mots = []
    #trouve le max des identifiants possibles
    maxIdent = nindTermindex.donneMaxIdentifiant()
    for ident in range(maxIdent):
        #vejrifie que c'est bien un identifiant de mot 
        mot, tailleMot = nindRetrolexicon.donneMot(ident)
        if tailleMot == 0: continue
        #lit la dejfinition du mot
        (offsetDejfinition, longueurDejfinition) = nindTermindex.donneAdresseDejfinition(ident)
        #si dejfinition existe, raf
        if offsetDejfinition != 0: continue
        #mejmorise le mot
        mots.append(mot)
    #affiche les résultats
    print('%d mots sans occurrence dans le corpus'%(len(mots)))
    print(', '.join(mots))
            
if __name__ == '__main__':
        main()
        
