#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-
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

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Analyse un fichier retrolexicon du système nind et affiche les stats. 
Le format du fichier est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier>
exemple : %s FRE.termindex
"""%(script, script))

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    
    #la classe
    nindRetrolexicon = NindRetrolexicon(lexiconindexFileName)
    nindRetrolexicon.analyseFichierRetrolexicon(True)

#####################################################
# <donnejesIndexejes>     ::= <dejfinitionMot>
# <blocEnVrac>            ::= <blocUtf8>
#
# <dejfinitionMot>        ::= <motComposej> | <motSimple>
# <motComposej>           ::= <flagComposej=31> <identifiantA> <identifiantS>
# <flagComposej=31>       ::= <Integer1>
# <identifiantA>          ::= <Integer3>
# <identifiantS>          ::= <Integer3>
# <motSimple>             ::= <flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
# <flagSimple=37>         ::= <Integer1>
# <longueurMotUtf8>       ::= <Integer1>
# <adresseMotUtf8>        ::= <Integer5>
#
# <blocUtf8>              ::= { <motUtf8> }
# <motUtf8>               ::= { <Octet> }
#
# <blocEnVrac>            ::= { <Octet> }
##############################
# <spejcifique>           ::= <vide>
#####################################################

FLAG_COMPOSEJ = 31
FLAG_SIMPLE = 37
TAILLE_COMPOSEJ_MAXIMUM = 30

class NindRetrolexicon(NindPadFile):
    def __init__(self, lexiconindexFileName):
        retrolexiconFileName = '.'.join(lexiconindexFileName.split('.')[:-1])+'.retrolexiconindex'
        #si le lexique inverse n'existe pas, on ne fait rien
        if not path.isfile(retrolexiconFileName): raise Exception("%s n'existe pas"%(retrolexiconFileName))
        #on initialise la classe mehre en lecture uniquement
        NindPadFile.__init__(self, retrolexiconFileName)

    def createFile(self):
        return
               
    def getWord (self, ident):
        mot = []
        (trouvej, motSimple, identifiantA, identifiantS) = self.getWordDef(ident)
        #si pas trouvej, retourne chaisne vide
        if not trouvej: return ''
        while True:
            #si c'est un mot simple, c'est la fin 
            if identifiantA == 0:
                mot.insert(0, motSimple)
                break
            #si c'est un mot composej, recupehre le mot simple du couple
            (trouvej, motSimple, identifiantA2, identifiantS2) = self.getWordDef(identifiantS)
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantS, self.latFileName))
            if identifiantA2 != 0: raise Exception("%d pas terminal %s"%(identifiantS, self.latFileName))
            mot.insert(0, motSimple)
            #recupere l'autre mot du couple
            (trouvej, motSimple, identifiantA, identifiantS) = self.getWordDef(identifiantA)
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantA, self.latFileName))
            #pour detecter les bouclages induits par un fichier bouclant
            if len(mot) == TAILLE_COMPOSEJ_MAXIMUM: 
                raise Exception("%d bouclage dans %s"%(ident, self.latFileName))
        #retourne la chaisne
        return '_'.join(mot)

    def getWordDef(self, ident):
        #trouve l'adresse des donnees dans le fichier
        offsetDejfinition = self.donnePositionEntreje(ident)
        #si pas trouve, le mot est inconnu
        if offsetDejfinition == 0: return (False, "", 0, 0)          #mot inconnu
        #lit la dejfinition
        self.seek(offsetDejfinition, 0)
        flag = self.litNombre1()
        if flag == 0: return (False, "", 0, 0)          #mot inconnu
        if flag == FLAG_SIMPLE:
            #<flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
            longueurMotUtf8 = self.litNombre1()
            adresseMotUtf8 = self.litNombre5()
            #lit la chaisne
            self.seek(adresseMotUtf8, 0)
            return (True, self.litChaine(longueurMotUtf8), 0, 0)
        if flag == FLAG_COMPOSEJ:
            #<flagComposej=31> <identifiantA> <identifiantS>
            return (True, '', self.litNombre3(), ident + self.litNombreS3())
        raise Exception("%d mauvaise définition %s"%(ident, self.latFileName))
        
    #analyse complehtement le fichier et retourne True si ok
    def analyseFichierRetrolexicon(self, trace):
        cestbon = self.analyseFichierPadFile(trace)
        try:
            totalUtf8 = 0
            nbreSimples = nbreComposejs = 0
            #met en place la carte des non-vides
            maxIdent, nonVidesList = self.donneCarteNonVides()
            for identifiant in range(maxIdent):
                position = self.donnePositionEntreje(identifiant)
                if position == 0: 
                    raise Exception('%s : identifiant %d sans indirection '%(self.latFileName, identifiant))
                self.seek(position, 0)
                flag = self.litNombre1()
                if flag == 0: continue
                if flag == FLAG_SIMPLE:
                    #<flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
                    longueurMotUtf8 = self.litNombre1()
                    adresseMotUtf8 = self.litNombre5()
                    nonVidesList.append((adresseMotUtf8, longueurMotUtf8))
                    totalUtf8 += longueurMotUtf8
                    nbreSimples += 1
                elif flag == FLAG_COMPOSEJ:
                    #<flagComposej=31> <identifiantA> <identifiantS>
                    identifiantA = self.litNombre3()
                    identifiantS = self.litNombre3()
                    nbreComposejs += 1
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
        if trace:
            print ("=============")
            print ("%d / %d index utilisés"%(nbreSimples + nbreComposejs, maxIdent))
            print ("=============")
            
        try:
            (nbreVides, tailleVides) = chercheVides(nonVidesList)
            total = tailleVides + totalUtf8
            if trace:
                print ("CHAÎNES UTF-8  % 10d (%6.2f %%) % 9d occurrences"%(totalUtf8, float(100)*totalUtf8/total, nbreSimples))
                print ("VIDES          % 10d (%6.2f %%) % 9d occurrences"%(tailleVides, float(100)*tailleVides/total, nbreVides))
                print ("TOTAL          % 10d %08X"%(total, total))
                print ("=============")
                total = nbreSimples + nbreComposejs
                print ("MOTS SIMPLES   % 10d (%6.2f %%)"%(nbreSimples, float(100)*nbreSimples/total))
                print ("MOTS COMPOSÉS  % 10d (%6.2f %%)"%(nbreComposejs, float(100)*nbreComposejs/total))
                print ("TOTAL          % 10d"%(total))
                print ("=============")
                print ("%0.2f octets / mot"%(float(self.donneTailleFichier())/total))
                print ("=============")
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
           
                       
if __name__ == '__main__':
    main()
