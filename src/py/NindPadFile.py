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
from os import path, getenv
import codecs
import datetime
import time
import math
from NindFile import NindFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
o Analyse un fichier plat du système nind et affiche les stats. 
Les types de fichiers : nindlexiconindex, nindtermindex, nindlocalindex,
nindretrolexicon
Le format du fichier est défini dans le document LAT2017.JYS.470.

usage   : %s <fichier>
exemple : %s fre.nindtermindex
"""%(script, script))


def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    padFileName = path.abspath(sys.argv[1])
    #la classe
    nindPadFile = NindPadFile(padFileName)
    nindPadFile.analyseFichierPadFile(True)

#############################################################
# <fichier>               ::= <tailleEntreje> <tailleSpejcifiques> { <blocIndexej> <blocEnVrac> } 
#                             <blocSpejcifique> <blocIdentification> 
#
# <tailleEntreje>         ::= <Entier1>
# <tailleSpejcifiques>    ::= <Entier3>
#
# <blocIndexej>           ::= <flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
# <flagIndexej=47>        ::= <Entier1>
# <addrBlocSuivant>       ::= <Entier5>
# <nombreIndex>           ::= <Entier3>
#
# <donnejesIndexejes>     ::= { <Octet> }
# <blocEnVrac>            ::= { <Octet> }
#
# <blocSpejcifique>       ::= <flagSpecifique=57> <spejcifiques> 
# <flagSpecifique=57>     ::= <Entier1>
# <spejcifique>           ::= { <Octet> }
#
# <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> 
# <flagIdentification=53> ::= <Entier1>
# <maxIdentifiant>        ::= <Entier3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Entier4>
#############################################################""    
FLAG_INDEXEJ = 47
FLAG_SPEJCIFIQUE = 57
FLAG_IDENTIFICATION = 53
#<flagIndexej=47>(1) <addrBlocSuivant>(5) <nombreIndex>(3) = 9
TAILLE_TETE_INDEXEJ = 9
#<flagIdentification=53>(1) <maxIdentifiant>(4) <identifieurUnique>(4) = 9
TAILLE_IDENTIFICATION = 9
#<flagSpecifique=57>(1)
TAILLE_ENTETE_SPEJCIFIQUE = 1
#<tailleEntreje>(1) <tailleSpejcifiques>(3) = 4
TAILLE_FIXES = 4
    
#############################################################
class NindPadFile(NindFile):
    
    def __init__(self, padFileName, enEjcriture = False, tailleEntreje = 0, tailleSpejcifiques = 0):
        #en lecture uniquement
        NindFile.__init__(self, padFileName, enEjcriture)
        self.entriesBlocksMap = []
        if enEjcriture:
            self.tailleEntreje = tailleEntreje
            self.tailleSpejcifiques = tailleSpejcifiques
            #ejcrit la partie fixe
            self.seek(0, 0)
            self.ejcritNombre1(tailleEntreje)
            self.ejcritNombre3(tailleSpejcifiques)
            #ejcrit les spejcifiques et l'identification ah 0
            self.ejcritZejros(TAILLE_IDENTIFICATION + TAILLE_ENTETE_SPEJCIFIQUE + tailleSpejcifiques)
        else:   
            #lit la partie fixe
            #<tailleEntreje> <tailleSpejcifiques>
            self.seek(0, 0)
            self.tailleEntreje = self.litNombre1()
            self.tailleSpejcifiques = self.litNombre3()
        
    #############################################################   
    #retourne l'identification du fichier
    def donneIdentificationFichier(self):
        #<flagIdentification=53> <maxIdentifiant> <identifieurUnique>
        self.seek(-TAILLE_IDENTIFICATION, 2)
        if self.litNombre1() != FLAG_IDENTIFICATION: 
            raise Exception('%s : pas FLAG_IDENTIFICATION à %08X'%(self.latFileName, self.tell() -1))
        maxIdentifiant = self.litNombre4()
        identifieurUnique = self.litNombre4()
        return (maxIdentifiant, identifieurUnique)
    
    #############################################################   
    #retourne l'adresse et la longueur des spejcifiques
    def donneSpejcifiques(self):
        #<flagSpecifique=57> <spejcifiques>
        self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE - self.tailleSpejcifiques , 2)
        if self.litNombre1() != FLAG_SPEJCIFIQUE: 
            raise Exception('%s : pas FLAG_SPEJCIFIQUE à %08X'%(self.latFileName, self.tell() -1))
        offsetSpejcifiques = self.tell()
        return (offsetSpejcifiques, self.tailleSpejcifiques)

    #############################################################   
    #donne l'adresse de l'index correspondant ah l'ident spejcifiej, 0 si hors limite
    def donnePositionEntreje(self, ident):
        position = self.__donneJustePositionEntreje(ident)
        #si pas trouvej, recharge la map une fois (au cas ouh le fichier aurait changej)
        if position == 0: 
            self.__ejtablitCarteIndex()
            position = self.__donneJustePositionEntreje(ident)
        return position
    
    #############################################################   
    #donne la taille du fichier
    def donneTailleFichier(self):
       self.seek(0, 2)
       return self.tell()
    
    #############################################################   
    #donne l'adresse de l'index correspondant ah l'ident spejcifiej, 0 si hors limite
    def __donneJustePositionEntreje(self, ident):
        firstIdent = 0
        for (addrPremierIndex, nombreIndex) in self.entriesBlocksMap:
            if ident < firstIdent + nombreIndex : 
                return (ident - firstIdent) * self.tailleEntreje + addrPremierIndex
            firstIdent += nombreIndex
        return 0

    #############################################################   
    #ejtablit la carte des index sur les diffejrents blocs
    def __ejtablitCarteIndex(self):
        self.entriesBlocksMap = []
        self.seek(TAILLE_FIXES, 0)
        while True:
            #<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
            flagIndexej = self.litNombre1()
            if flagIndexej != FLAG_INDEXEJ: 
                raise Exception('%s : pas FLAG_INDEXEJ à %08X'%(self.latFileName, self.tell()))
            addrBlocSuivant = self.litNombre5()
            nombreIndex = self.litNombre3()
            addrPremierIndex = self.tell()
            self.entriesBlocksMap.append((addrPremierIndex, nombreIndex))
            if addrBlocSuivant == 0: break
            #saute au bloc d'indirection suivant
            self.seek(addrBlocSuivant, 0)
            
    #############################################################   
    #retourne l'identifiant maximum possible avec le systehme d'index du fichier
    def donneMaxIdentifiant(self):
        self.seek(TAILLE_FIXES, 0)
        maxIdent = 0
        while True:
            addrBloc = self.tell()
            #<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
            if self.litNombre1() != FLAG_INDEXEJ: 
                raise Exception('%s : pas FLAG_INDEXEJ à %08X'%(self.latFileName, self.tell()))
            addrBlocSuivant = self.litNombre5()
            maxIdent += self.litNombre3()
            if addrBlocSuivant == 0: break
            #saute au bloc d'indirection suivant
            self.seek(addrBlocSuivant, 0)
        return maxIdent
    
    #############################################################   
    #retourne la liste des occupations des fixes, des blocs indexejs, des spejcifiques et de l'identification
    def donneCarteNonVides(self):
        #les fixes
        nonVidesList = [ (0, TAILLE_FIXES) ]
        self.seek(TAILLE_FIXES, 0)
        #les blocs indexejs
        maxIdent = 0
        while True:
            addrBloc = self.tell()
            #<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
            if self.litNombre1() != FLAG_INDEXEJ: 
                raise Exception('%s : pas FLAG_INDEXEJ à %08X'%(self.latFileName, self.tell()))
            addrBlocSuivant = self.litNombre5()
            nombreIndex = self.litNombre3()
            maxIdent += nombreIndex
            tailleBloc = TAILLE_TETE_INDEXEJ + nombreIndex * self.tailleEntreje
            nonVidesList.append((addrBloc, tailleBloc))
            if addrBlocSuivant == 0: break
            #saute au bloc d'indirection suivant
            self.seek(addrBlocSuivant, 0)
        #les spejcifiques et l'identification
        tailleSpejcifIdent = TAILLE_ENTETE_SPEJCIFIQUE + TAILLE_IDENTIFICATION + self.tailleSpejcifiques
        self.seek(-tailleSpejcifIdent, 2)
        nonVidesList.append((self.tell(), tailleSpejcifIdent))
        return maxIdent, nonVidesList
    
    #############################################################   
    #ejcrit un bloc d'index vide

    
    #############################################################""    
    #analyse complehtement le fichier et retourne True si ok
    def analyseFichierPadFile(self, trace):
        cestBon = True
        if trace: print ("======PADFILE=======")
        blocIndex = 0
        tailleIndex = 0 
        tailleEnVrac = 0
        try:
            self.seek(0, 0)
            tailleEntreje = self.litNombre1()
            tailleSpejcifique = self.litNombre3()
            if trace: 
                print('TAILLE ENTRÉES             : ', tailleEntreje)
                print('TAILLE DONNÉES SPÉCIFIQUES : ', tailleSpejcifique)
                print ("=============")
            addrIndex = TAILLE_FIXES
            while True:
                self.seek(addrIndex, 0)
                #<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
                flagIndexej = self.litNombre1()
                if flagIndexej != FLAG_INDEXEJ: 
                    raise Exception('%s : pas FLAG_INDEXEJ à %08X'%(self.latFileName, addrIndex))
                addrBlocSuivant = self.litNombre5()
                nombreIndex = self.litNombre3()
                blocIndex +=1
                tailleBloc = TAILLE_TETE_INDEXEJ + nombreIndex * tailleEntreje
                tailleIndex += tailleBloc
                entrejesUtilisejes = 0
                for i in range(nombreIndex):
                    #<donnejesIndexejes>     ::= { <Octet> }
                    #une entreje vide a tous ses octets ah 0
                    somme = 0
                    for j in range(tailleEntreje): somme += self.litNombre1()
                    if somme != 0: entrejesUtilisejes +=1
                if trace: 
                    print ("%08X: Bloc indexé  n° % 2d :    % 10d / %d entrées utilisées"%(addrIndex, blocIndex, entrejesUtilisejes, nombreIndex))
                
                addrEnVrac = self.tell()
                if addrBlocSuivant != 0: tailleBloc = addrBlocSuivant - addrEnVrac
                else:
                    self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE -tailleSpejcifique, 2)
                    tailleBloc = self.tell() - addrEnVrac
                tailleEnVrac += tailleBloc
                if trace: 
                    print ("%08X: Bloc en vrac n° % 2d :    % 10d octets"%(addrEnVrac, blocIndex, tailleBloc))

                if addrBlocSuivant == 0: break
                addrIndex = addrBlocSuivant
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
            #raise
        if trace: 
            print ("%d blocs indexés  de taille totale % 10d octets"%(blocIndex, tailleIndex))
            print ("%d blocs en vrac  de taille totale % 10d octets"%(blocIndex, tailleEnVrac))
            print ("=============")
        try:
            self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE -tailleSpejcifique, 2)
            addrSpejcifiques = self.tell()
            #<flagSpecifique=57> <spejcifiques> 
            if self.litNombre1() != FLAG_SPEJCIFIQUE: 
                raise Exception('%s : pas FLAG_SPEJCIFIQUE à %08X'%(self.latFileName, addrSpejcifiques))
            for i in range(tailleSpejcifique): self.litNombre1()
            if self.litNombre1() != FLAG_IDENTIFICATION: 
                raise Exception('%s : taille des spécifiques incorrecte'%(self.latFileName))
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
        tailleSpejcifique += TAILLE_ENTETE_SPEJCIFIQUE
        try:
            #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4> <identifieurSpecifique_4>
            self.seek(-TAILLE_IDENTIFICATION, 2)
            addrIdentification = self.tell()
            if self.litNombre1() != FLAG_IDENTIFICATION: 
                raise Exception('%s : pas FLAG_IDENTIFICATION à %08X'%(self.latFileName, addrIdentification))
            maxIdentifiant = self.litNombre4()
            dateHeure = self.litNombre4()
            print ("max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure))))
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
            
        self.seek(0, 2)
        offsetFin = self.tell()
        total = tailleIndex + tailleEnVrac + tailleSpejcifique + TAILLE_IDENTIFICATION + TAILLE_FIXES
        if trace: 
            print ("=============")
            print ("FIXES          % 10d (%6.2f %%)"%(TAILLE_FIXES, float(100)*TAILLE_FIXES/total))
            print ("INDEXÉS        % 10d (%6.2f %%)"%(tailleIndex, float(100)*tailleIndex/total))
            print ("EN VRAC        % 10d (%6.2f %%)"%(tailleEnVrac, float(100)*tailleEnVrac/total))
            print ("SPÉCIFIQUES    % 10d (%6.2f %%)"%(tailleSpejcifique, float(100)*tailleSpejcifique/total))
            print ("IDENTIFICATION % 10d (%6.2f %%)"%(TAILLE_IDENTIFICATION, float(100)*TAILLE_IDENTIFICATION/total))
            print ("TOTAL          % 10d %08X"%(total, total))
            print ("taille fichier % 10d %08X"%(offsetFin, offsetFin))
        return cestBon
    
#############################################################""    
#ah partir de la carte des non-vides, trouve le nombre et la taille des vides
def chercheVides(nonVidesList):
    #ordonne les indirections
    nonVidesList.sort()
    addressePrec = longueurPrec = 0
    nbreVides = tailleVides = 0
    typesVides = {}
    typesNonVides = {}
    for (addresse, longueur) in nonVidesList:
        if longueur not in typesNonVides: typesNonVides[longueur] = 0
        typesNonVides[longueur] +=1
        longueurVide = addresse - addressePrec - longueurPrec
        if longueurVide < 0: 
            raise Exception('%s : chevauchement %08X-%d et %08X-%d'%(self.latFileName, addressePrec, longueurPrec, addresse, longueur))
        if longueurVide > 0:
            #print 'addresse=%d, addressePrec=%d, longueurPrec=%d'%(addresse, addressePrec, longueurPrec)
            nbreVides +=1
            tailleVides += longueurVide
            if longueurVide not in typesVides: typesVides[longueurVide] = 0
            typesVides[longueurVide] +=1
        addressePrec = addresse
        longueurPrec = longueur 
    return nbreVides, tailleVides, typesVides, typesNonVides

#############################################################   
#ah partir d'une liste calcule le max, le min, la moyenne et l'ejcart-type 
def calculeRejpartition(nombres):
    if len(nombres) == 0: return 0, 0, 0, 0, 0.0, 0.0
    somme = somme2 = 0
    min = max = nombres[0]
    for nombre in nombres :
        somme += nombre
        somme2 += nombre**2
        if min > nombre: min = nombre
        if max < nombre: max = nombre
    moyenne = float(somme) / len(nombres)
    variance = float(somme2) / len(nombres) - moyenne**2
    ecartType = math.sqrt(variance)
    return len(nombres), min, max, somme, moyenne, ecartType


if __name__ == '__main__':
        main()
    
        
