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
from time import ctime
from NindPadFile import NindPadFile
from NindPadFile import chercheVides
from NindPadFile import calculeRejpartition

def usage():
    #print(sys.version)
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
o Analyse un fichier NindIndex du système nind et affiche les statistiques. 
o Peut donner l'offset dans le fichier et la longueur des données 
  correspondant à un identifiant.
Les types de fichiers : nindlexiconindex, nindtermindex, nindlocalindex
Le format du fichier est défini dans le document LAT2017.JYS.470.

usage   : %s <fichier> [ <analyse> | <indirection> <index> ]
exemple : %s FRE.termindex
exemple : %s FRE.termindex indir 398892
"""%(script, script, script))

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        indexFileName = path.abspath(sys.argv[1])
        action = 'analyse' 
        if len(sys.argv) > 2 : action = sys.argv[2]
        index = 0
        if len(sys.argv) > 3 : index = int(sys.argv[3])
        
        #la classe
        nindIndex = NindIndex(indexFileName)
        if action.startswith('anal'): nindIndex.analyseFichierIndex(True)
        elif action.startswith('ind'):
            (offsetDejfinition, longueurDejfinition) = nindIndex.donneAdresseDejfinition(index)
            print ('offset   : ', offsetDejfinition)
            print ('longueur : ', longueurDejfinition)
        else: raise Exception()
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()

############################################################
# <donnejesIndexejes>     ::= <indirection>
# <blocEnVrac>            ::= <blocDejfinition>
#
# <indirection>           ::= <offsetDejfinition> <longueurDejfinition> 
# <offsetDejfinition>     ::= <Entier5>
# <longueurDejfinition>   ::= <Entier3>
#
# <blocDejfinition>       ::= { <dejfinition> | <vide> }
# <dejfinition>           ::= { <Octet> }
# <vide>                  ::= { <Octet> }
############################################################
#<offsetDejfinition>(5) <longueurDejfinition>(3) = 8
TAILLE_INDIRECTION = 8
############################################################

class NindIndex(NindPadFile):
    
    def __init__(self, indexFileName):
        #en lecture uniquement
        NindPadFile.__init__(self, indexFileName)
        
    #donne l'adresse et la longueur de la dejfinition
    def donneAdresseDejfinition(self, identifiant):
        position = self.donnePositionEntreje(identifiant)
        if position == 0: return (0, 0)          #identifiant hors limite
        self.seek(position, 0)
        #<offsetDejfinition> <longueurDejfinition>
        offsetDejfinition = self.litNombre5()
        longueurDejfinition = self.litNombre3()
        return (offsetDejfinition, longueurDejfinition)

    #analyse complehtement le fichier et retourne True si ok
    def analyseFichierIndex(self, trace):
        cestbon = self.analyseFichierPadFile(trace)
        if not cestbon: return False
        if trace: print ("======INDEX=======")
        try:
            #met en place la carte des non-vides
            maxIdent, nonVidesList = self.donneCarteNonVides()
            #chaque indirection utiliséeje est mise dans la carte
            dejfinitions = []
            for identifiant in range(maxIdent):
                try:
                    position = self.donnePositionEntreje(identifiant)
                    if position == 0: 
                        raise Exception('%s : identifiant %d sans indirection '%(self.latFileName, identifiant))
                    self.seek(position, 0)
                    #<offsetDejfinition> <longueurDejfinition>
                    offsetDejfinition = self.litNombre5()
                    longueurDejfinition = self.litNombre3()
                    if offsetDejfinition != 0:
                        nonVidesList.append((offsetDejfinition, longueurDejfinition))
                        dejfinitions.append(longueurDejfinition)
                except:
                    if trace: print ('*******ERREUR SUR IDENTIFIANT :', identifiant)
                    raise
        except Exception as exc: 
            cestbon = False
            if trace: print ('*******ERREUR :', exc.args[0])
        if trace:
            indirectionsUtilisejes, tailleMin, tailleMax, tailleDejfinitions, moyenne, ejcartType = calculeRejpartition(dejfinitions)
            print ("%d / %d indirections utilisées"%(indirectionsUtilisejes, maxIdent))
            print ("=============")
        try:
            nbreVides, tailleVides, typesVides, typesNonVides = chercheVides(nonVidesList)
            if trace:
                total = tailleVides + tailleDejfinitions
                print ("DÉFINITIONS    % 10d (%6.2f %%) % 9d occurrences"%(tailleDejfinitions, float(100)*tailleDejfinitions/total, indirectionsUtilisejes))
                print ("VIDES          % 10d (%6.2f %%) % 9d occurrences"%(tailleVides, float(100)*tailleVides/total,nbreVides))
                print ("TOTAL          % 10d %08X"%(total, total))
                print ("=============")
                typesVidesList = list(typesVides.items())
                typesNonVidesList = list(typesNonVides.items())
                typesVidesList.sort()
                typesNonVidesList.sort()
                print ("VIDES     de ", typesVidesList[:3], " à ", typesVidesList[-1:])
                print ("NON VIDES de ", typesNonVidesList[:3], " à ", typesNonVidesList[-1:])
        except Exception as exc: 
            cestbon = False
            if trace: print ('ERREUR :', exc.args[0])
        if trace:
            print ("=============")
            print ("DÉFINITIONS MAX% 10d octets"%(tailleMax))
            print ("DÉFINITIONS MIN% 10d octets"%(tailleMin))
            print ("MOYENNE        % 10d octets"%(moyenne))
            print ("ÉCART-TYPE     % 10d octets"%(ejcartType))
        return cestbon
        
if __name__ == '__main__':
    main()
