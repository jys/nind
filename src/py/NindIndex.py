#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv, path
from time import ctime
from NindPadFile import NindPadFile
from NindPadFile import chercheVides
from NindPadFile import calculeRejpartition

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un fichier NindIndex du système nind et affiche les stats. 
Les types de fichiers : lexiconindex, termindex, localindex
Le format du fichier est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier>
exemple : %s FRE.termindex
"""%(script, script)

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    indexFileName = path.abspath(sys.argv[1])
    
    #la classe
    nindIndex = NindIndex(indexFileName)
    nindIndex.analyseFichierIndex(True)

############################################################
# <donnejesIndexejes>     ::= <indirection>
# <blocEnVrac>            ::= <blocDejfinition>
#
# <indirection>           ::= <offsetDejfinition> <longueurDejfinition> 
# <offsetDejfinition>     ::= <Integer5>
# <longueurDejfinition>   ::= <Integer3>
#
# <blocDejfinition>       ::= { <dejfinition> | <vide> }
# <dejfinition>           ::= { <Octet> }
# <vide>                  ::= { <Octet> }
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
        if trace:
            print "============="
        try:
            #met en place la carte des non-vides
            maxIdent, nonVidesList = self.donneCarteNonVides()
            #chaque indirection utiliséeje est mise dans la carte
            dejfinitions = []
            for identifiant in range(maxIdent):
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
        except Exception as exc: 
            cestBon = False
            if trace: print 'ERREUR :', exc.args[0]
        if trace:
            indirectionsUtilisejes, tailleMin, tailleMax, tailleDejfinitions, moyenne, ejcartType = calculeRejpartition(dejfinitions)
            print "%d / %d indirections utilisées"%(indirectionsUtilisejes, maxIdent)
            print "============="
        try:
            (nbreVides, tailleVides) = chercheVides(nonVidesList)
            if trace:
                total = tailleVides + tailleDejfinitions
                print "DÉFINITIONS    % 10d (%6.2f %%) % 9d occurrences"%(tailleDejfinitions, float(100)*tailleDejfinitions/total, indirectionsUtilisejes)
                print "VIDES          % 10d (%6.2f %%) % 9d occurrences"%(tailleVides, float(100)*tailleVides/total,nbreVides)
                print "TOTAL          % 10d %08X"%(total, total)
        except Exception as exc: 
            cestBon = False
            if trace: print 'ERREUR :', exc.args[0]  
        if trace:
            print "============="
            print "DÉFINITIONS MAX% 10d octets"%(tailleMax)
            print "DÉFINITIONS MIN% 10d octets"%(tailleMin)
            print "MOYENNE        % 10d octets"%(moyenne)
            print "ÉCART-TYPE     % 10d octets"%(ejcartType)
           
       
        
if __name__ == '__main__':
    main()
