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

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Donne la répartition des mots composés en fonction de leur longueur 
dans un fichier *.nindretrolexicon-dumpall.txt obtenu par la commande
$PY/Nind_dumpLexicon.py.

usage   : %s <fichier dumpall>
exemple : %s clef/fre.nindretrolexicon-dumpall.txt
"""%(script, script))

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        nomFichierLexique = path.realpath(sys.argv[1])
        statistiques(nomFichierLexique)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
    
def statistiques(nomFichierLexique):
    fichierLexique = codecs.open(nomFichierLexique, 'r', 'utf-8')
    compteurs = {}
    for ligne in fichierLexique:
        nbSoulignejs = len(ligne) - len(ligne.replace('_', ''))
        if nbSoulignejs not in compteurs: compteurs[nbSoulignejs] = 0
        compteurs[nbSoulignejs] +=1
    fichierLexique.close()
    clefs = list(compteurs.keys())
    clefs.sort()
    total = 0
    for longueur in clefs: 
        nombre = compteurs[longueur]
        print ('% 3d : % 9d'%(longueur + 1, nombre))
        total += nombre
    print('        -------')
    print('      % 9d'%(total))
        


if __name__ == '__main__':
        main()
        