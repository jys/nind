#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import path, getenv
import codecs
import datetime
import time
import math
from NindLateconFile import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un fichier plat du système nind et affiche les stats. 
Les types de fichiers : lexiconindex, termindex, localindex, retrolexicon
Le format du fichier est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier>
exemple : %s FRE.termindex
"""%(script, script)


def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    padFileName = path.abspath(sys.argv[1])
    #la classe
    nindPadFile = NindPadFile(padFileName)
    nindPadFile.analyseFichierPadFile(True)

#############################################################""    
# <fichier>               ::= <tailleEntreje> <tailleSpejcifiques> { <blocIndexej> <blocEnVrac> } 
#                             <blocSpejcifique> <blocIdentification> 
#
# <tailleEntreje>         ::= <Integer1>
# <tailleSpejcifiques>    ::= <Integer3>
#
# <blocIndexej>           ::= <flagIndexej=47> <addrBlocSuivant> <nombreIndex> { <donnejesIndexejes> }
# <flagIndexej=47>        ::= <Integer1>
# <addrBlocSuivant>       ::= <Integer5>
# <nombreIndex>           ::= <Integer3>
#
# <donnejesIndexejes>     ::= { <Octet> }
# <blocEnVrac>            ::= { <Octet> }
#
# <blocSpejcifique>       ::= <flagSpecifique=57> <spejcifiques> 
# <flagSpecifique=57>     ::= <Integer1>
# <spejcifique>           ::= { <Octet> }
#
# <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> 
# <flagIdentification=53> ::= <Integer1>
# <maxIdentifiant>        ::= <Integer3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Integer4>
FLAG_INDEXEJ = 47
FLAG_SPEJCIFIQUE = 57
FLAG_IDENTIFICATION = 53
#<flagIndexej=47>(1) <addrBlocSuivant>(5) <nombreIndex>(3) = 9
TAILLE_TETE_INDEXEJ = 9
#<flagIdentification=53>(1) <maxIdentifiant>(3) <identifieurUnique>(4) = 8
TAILLE_IDENTIFICATION = 8
#<flagSpecifique=57>(1)
TAILLE_ENTETE_SPEJCIFIQUE = 1
#<tailleEntreje>(1) <tailleSpejcifiques>(3) = 4
TAILLE_FIXES = 4
#<flagSpecifique=57>
TAILLE_ENTETE_SPEJCIFIQUE = 1
    
#############################################################""    
class NindPadFile(NindLateconFile):
    
    def __init__(self, padFileName):
        #en lecture uniquement
        NindLateconFile.__init__(self, padFileName, False)
        self.entriesBlocksMap = []
        #lit la partie fixe
        #<tailleEntreje> <tailleSpejcifiques>
        self.seek(0, 0)
        self.tailleEntreje = self.litNombre1()
        self.tailleSpejcifiques = self.litNombre3()
        
    #retourne l'identification du fichier
    def donneIdentificationFichier(self):
        #<flagIdentification=53> <maxIdentifiant> <identifieurUnique>
        self.seek(-TAILLE_IDENTIFICATION, 2)
        if self.litNombre1() != FLAG_IDENTIFICATION: 
            raise Exception('%s : pas FLAG_IDENTIFICATION à %08X'%(self.latFileName, self.tell() -1))
        maxIdentifiant = self.litNombre3()
        identifieurUnique = self.litNombre4()
        return (maxIdentifiant, identifieurUnique)
    
    #retourne l'adresse et la longueur des spejcifiques
    def donneSpejcifiques(self):
        #<flagSpecifique=57> <spejcifiques>
        self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE - self.tailleSpejcifiques , 2)
        if self.litNombre1() != FLAG_SPEJCIFIQUE: 
            raise Exception('%s : pas FLAG_SPEJCIFIQUE à %08X'%(self.latFileName, self.tell() -1))
        offsetSpejcifiques = self.tell()
        return (offsetSpejcifiques, self.tailleSpejcifiques)

    #donne l'adresse de l'index correspondant ah l'ident spejcifiej, 0 si hors limite
    def donnePositionEntreje(self, ident):
        position = self.__donneJustePositionEntreje(ident)
        #si pas trouvej, recharge la map une fois (au cas ouh le fichier aurait changej)
        if position == 0: 
            self.__ejtablitCarteIndex()
            position = self.__donneJustePositionEntreje(ident)
        return position
    
    #done la taille du fichier
    def donneTailleFichier(self):
       self.seek(0, 2)
       return self.tell()
    
    #donne l'adresse de l'index correspondant ah l'ident spejcifiej, 0 si hors limite
    def __donneJustePositionEntreje(self, ident):
        firstIdent = 0
        for (addrPremierIndex, nombreIndex) in self.entriesBlocksMap:
            if ident < firstIdent + nombreIndex : 
                return (ident - firstIdent) * self.tailleEntreje + addrPremierIndex
            firstIdent += nombreIndex
        return 0

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
    
    #analyse complehtement le fichier et retourne True si ok
    def analyseFichierPadFile(self, trace):
        cestBon = True
        if trace: print "============="
        blocIndex = 0
        tailleIndex = 0 
        tailleEnVrac = 0
        try:
            self.seek(0, 0)
            tailleEntreje = self.litNombre1()
            tailleSpejcifique = self.litNombre3()
            while True:
                #<flagIndexej=47> <addrBlocSuivant> <nombreIndex>
                addrIndex = self.tell()
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
                    print "%08X: Bloc indexé  n° % 2d : % 8d / %d entrées utilisées"%(addrIndex, blocIndex, entrejesUtilisejes, nombreIndex)
                
                addrEnVrac = self.tell()
                if addrBlocSuivant != 0: tailleBloc = addrBlocSuivant - addrEnVrac
                else:
                    self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE -tailleSpejcifique, 2)
                    tailleBloc = self.tell() - addrEnVrac
                tailleEnVrac += tailleBloc
                if trace: 
                    print "%08X: Bloc en vrac n° % 2d : % 8d octets"%(addrEnVrac, blocIndex, tailleBloc)

                if addrBlocSuivant == 0: break
                self.seek(addrBlocSuivant, 0)
        except Exception as exc: 
            cestBon = False
            if trace: print 'ERREUR :', exc.args[0]
            #raise
        if trace: 
            print "%d blocs indexés  de taille totale % 9d octets"%(blocIndex, tailleIndex)
            print "%d blocs en vrac  de taille totale % 9d octets"%(blocIndex, tailleEnVrac)
        
            print "============="
        try:
            self.seek(-TAILLE_IDENTIFICATION -TAILLE_ENTETE_SPEJCIFIQUE -tailleSpejcifique, 2)
            #<flagSpecifique=57> <spejcifiques> 
            addrSpejcifiques = self.tell()
            flagSpecifique = self.litNombre1()
            if flagSpecifique != FLAG_SPEJCIFIQUE: 
                raise Exception('%s : pas FLAG_SPEJCIFIQUE à %08X'%(self.latFileName, addrSpejcifiques))
            addrSpejcifiques = self.tell()
            self.seek(-TAILLE_IDENTIFICATION, 2)
            if self.tell() - addrSpejcifiques != tailleSpejcifique: 
                raise Exception('%s : taille des spécifiques incorrecte'%(self.latFileName))
        except Exception as exc: 
            cestBon = False
            if trace: print 'ERREUR :', exc.args[0]
        if trace: 
            print "%d octets de données spécifiques"%(tailleSpejcifique)
        
            print "============="
        tailleSpejcifique += TAILLE_ENTETE_SPEJCIFIQUE
        try:
            #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4> <identifieurSpecifique_4>
            self.seek(-TAILLE_IDENTIFICATION, 2)
            addrIdentification = self.tell()
            if self.litNombre1() != FLAG_IDENTIFICATION: 
                raise Exception('%s : pas FLAG_IDENTIFICATION à %08X'%(self.latFileName, addrIdentification))
            maxIdentifiant = self.litNombre3()
            dateHeure = self.litNombre4()
            print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
        except Exception as exc: 
            cestBon = False
            if trace: print 'ERREUR :', exc.args[0]
            
        self.seek(0, 2)
        offsetFin = self.tell()
        total = tailleIndex + tailleEnVrac + tailleSpejcifique + TAILLE_IDENTIFICATION + TAILLE_FIXES
        if trace: 
            print "============="
            print "FIXES          % 10d (%6.2f %%)"%(TAILLE_FIXES, float(100)*TAILLE_FIXES/total)
            print "INDEXÉS        % 10d (%6.2f %%)"%(tailleIndex, float(100)*tailleIndex/total)
            print "EN VRAC        % 10d (%6.2f %%)"%(tailleEnVrac, float(100)*tailleEnVrac/total)
            print "SPÉCIFIQUES    % 10d (%6.2f %%)"%(tailleSpejcifique, float(100)*tailleSpejcifique/total)
            print "IDENTIFICATION % 10d (%6.2f %%)"%(TAILLE_IDENTIFICATION, float(100)*TAILLE_IDENTIFICATION/total)
            print "TOTAL          % 10d %08X"%(total, total)
            print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
        return cestBon
    
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
    return (nbreVides, tailleVides)

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
    
        