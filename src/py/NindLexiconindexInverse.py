#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import datetime
import time
import NindLateconFile
import NindIndex

def usage():
    print """© l'ATÉCON.
Programme de test de la classe NindLexiconindexInverse.
Cette classe gère le lexique inverse du fichier lexique spécifié.
Le lexique inverse permet de trouver un terme à partir de son identifiant.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test crée le fichier lexique inverse s'il n'existe pas
ou s'il n'est pas apairé avec le fichier lexique puis affiche le terme
correspondant à l'identifiant spécifié.

usage   : %s <fichier lexiconindex> <ident terme>
exemple : %s box/dumps/boxon/FRE.lexiconindex 203547
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = os.path.abspath(sys.argv[1])
    ident = int(sys.argv[2])
    
    #la classe
    nindLexiconindexInverse = NindLexiconindexInverse(lexiconindexFileName)
    print nindLexiconindexInverse.getTerme(ident)

# <fichier>               ::= <blocAccesDirect> <blocChaine> <blocIdentification> 

# <blocAccesDirect>       ::= { <terme> }
# <terme>                 ::= <termeCompose> | <termeSimple>
# <termeCompose>          ::= <flagCompose> <identA> <identB>
# <flagCompose>           ::= <Integer1>
# <identA>                ::= <Integer3>
# <identB>                ::= <Integer3>
# <termeSimple>           ::= <flagSimple> <longueurChaine> <addresseChaine>
# <flagSimple>            ::= <Integer1>
# <longueurChaine>        ::= <Integer1>
# <addresseChaine>        ::= <Integer5>

# <blocChaine>            ::= { <chaine> }
# <chaine>                ::= { <Octet> }

# <blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
# <flagIdentification>    ::= <Integer1>
# <maxIdentifiant>        ::= <Integer3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Integer4>

FLAG_INDIRECTION = 47
FLAG_DEFINITION = 17
FLAG_IDENTIFICATION = 53
FLAG_TERME_SIMPLE = 73
FLAG_TERME_COMPOSE = 79
TAILLE_IDENTIFICATION = 8
TAILLE_DEFINITION = 7
TETE_DEFINITION = 7

class NindLexiconindexInverse:
    def __init__(self, lexiconindexFileName):
        self.lexiconindexFileName = lexiconindexFileName
        self.lexiconindexInverseFileName = lexiconindexFileName + 'inverse'
        #si le fichier inverse n'existe pas ou s'il n'est pas coherent avec le fichier lexique, il est calcule
        if not os.path.isfile(self.lexiconindexInverseFileName): 
            print "%s n'existe pas, il est créé"%(self.lexiconindexInverseFileName)
            self.createInverseFile()
        else:
            #ouvre les fichiers en lecture
            lexiconindexFile = NindLateconFile.NindLateconFile(self.lexiconindexFileName)
            lexiconindexInverseFile = NindLateconFile.NindLateconFile(self.lexiconindexInverseFileName)
            #verifie l'identification des fichiers
            (maxIdentifiant, dateHeure) = NindIndex.getIdentification(lexiconindexFile, self.lexiconindexFileName)
            (maxIdentifiant2, dateHeure2) = NindIndex.getIdentification(lexiconindexInverseFile, self.lexiconindexInverseFileName)
            lexiconindexFile.close()
            lexiconindexInverseFile.close()
            if maxIdentifiant2 != maxIdentifiant or dateHeure2 != dateHeure: 
                print "%s n'est pas à jour avec %s, il est recréé"%(self.lexiconindexInverseFileName, self.lexiconindexFileName)
                self.createInverseFile()
        #le fichier inverse est a jour, on l'ouvre en lecture
        self.lexiconindexInverseFile = NindLateconFile.NindLateconFile(self.lexiconindexInverseFileName)

    def getIdentification(self):
        return NindIndex.getIdentification(self.lexiconindexInverseFile, self.lexiconindexInverseFileName)
    
    def createInverseFile(self):
        #ouvre en lecture
        lexiconindexFile = NindLateconFile.NindLateconFile(self.lexiconindexFileName)
        lexiconindexFile2 = NindLateconFile.NindLateconFile(self.lexiconindexFileName)
        #ouvre en ecriture
        lexiconindexInverseFile = NindLateconFile.NindLateconFile(self.lexiconindexInverseFileName, True)
        #lit l'identification du lexique
        (maxIdentifiant, dateHeure) = NindIndex.getIdentification(lexiconindexFile, self.lexiconindexFileName)
        #<blocAccesDirect> <blocChaine> <blocIdentification> 
        #ecrit le <blocAccesDirect>
        for i in range(maxIdentifiant): 
            lexiconindexInverseFile.ecritNombre4(0)
            lexiconindexInverseFile.ecritNombre4(0)
        #peuple le fichier
        lexiconindexFile.seek(0, 0)
        addrIndirection = lexiconindexFile.tell()
        #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
        if lexiconindexFile.litNombre1() != FLAG_INDIRECTION: 
            raise Exception('pas FLAG_INDIRECTION sur %s'%(self.lexiconindexFileName))
        if lexiconindexFile.litNombre5() != 0: 
            raise Exception("plusieurs blocs d'indirection sur %s"%(self.lexiconindexFileName))
        nombreIndirection = lexiconindexFile.litNombre3()
        for noDef in range(nombreIndirection):
            #<offsetDefinition> <longueurDefinition>
            offsetDefinition = lexiconindexFile.litNombre5()
            longueurDefinition = lexiconindexFile.litNombre3()
            if offsetDefinition == 0: continue 
            lexiconindexFile2.seek(offsetDefinition, 0)
            #<flagDefinition> <identifiantHash> <longueurDonnees> 
            if lexiconindexFile2.litNombre1() != FLAG_DEFINITION: 
                raise Exception('pas FLAG_DEFINITION sur %s à %08X'%(self.lexiconindexFileName, offsetDefinition))
            if lexiconindexFile2.litNombre3() != noDef: 
                raise Exception('%d pas trouvé sur %s à %08X'%(noDoc, self.lexiconindexFileName, offsetDefinition+1))
            longueurDonnees = lexiconindexFile2.litNombre3()
            finDonnees = offsetDefinition + longueurDonnees + TETE_DEFINITION
            while lexiconindexFile2.tell() < finDonnees:
                #<termeSimple> <identifiantS> <nbreComposes> <composes>
                termeSimple = lexiconindexFile2.litString()
                identifiantS = lexiconindexFile2.litNombre3()
                #1) ecrit le terme simple 
                if identifiantS > maxIdentifiant:
                    raise Exception('%d > %d dans %s'%(identifiantS, maxIdentifiant, self.lexiconindexFileName))
                #ecrit la chaine a la fin dans <blocChaine>
                lexiconindexInverseFile.seek(0, 2)
                adrChaine = lexiconindexInverseFile.tell()
                lexiconindexInverseFile.ecritChaine(termeSimple)
                #ecrit dans <blocAccesDirect>
                #<flagSimple> <longueurChaine> <addresseChaine>
                lexiconindexInverseFile.seek((identifiantS - 1) * TAILLE_DEFINITION, 0)
                lexiconindexInverseFile.ecritNombre1(FLAG_TERME_SIMPLE)
                #la longueur de la chaine en nombre d'octets utf-8
                lexiconindexInverseFile.ecritNombre1(len(termeSimple.encode('utf-8'))) 
                lexiconindexInverseFile.ecritNombre5(adrChaine)
                #2) ecrit les termes composes
                nbreComposes = lexiconindexFile2.litNombreULat()
                identifiantC = identifiantS
                for noc in range(nbreComposes):
                    #<identifiantA> <identifiantRelC>
                    identifiantA = lexiconindexFile2.litNombre3()
                    identifiantC += lexiconindexFile2.litNombreSLat()
                    if identifiantC > maxIdentifiant:
                        raise Exception('%d > %d dans %s'%(identifiantC, maxIdentifiant, self.lexiconindexFileName))
                    #ecrit dans <blocAccesDirect>
                    #<flagCompose> <identA> <identB>
                    lexiconindexInverseFile.seek((identifiantC - 1) * TAILLE_DEFINITION, 0)
                    lexiconindexInverseFile.ecritNombre1(FLAG_TERME_COMPOSE)
                    lexiconindexInverseFile.ecritNombre3(identifiantA)
                    lexiconindexInverseFile.ecritNombre3(identifiantS)
        #ecrit le <blocIdentification>
        lexiconindexInverseFile.seek(0, 2)     
        lexiconindexInverseFile.ecritNombre1(FLAG_IDENTIFICATION)
        lexiconindexInverseFile.ecritNombre3(maxIdentifiant)
        lexiconindexInverseFile.ecritNombre4(dateHeure)
        lexiconindexFile.close()
        lexiconindexFile2.close()
        lexiconindexInverseFile.close()
               
    def getTerme (self, ident):
        terme = []
        identCourant = ident
        while True:
            #<termeCompose> | <termeSimple>
            self.lexiconindexInverseFile.seek((identCourant - 1) * TAILLE_DEFINITION, 0)
            flag = self.lexiconindexInverseFile.litNombre1()
            if flag == FLAG_TERME_SIMPLE:
                #<flagSimple> <longueurChaine> <addresseChaine>
                longueurChaine = self.lexiconindexInverseFile.litNombre1()
                addresseChaine = self.lexiconindexInverseFile.litNombre5()
                self.lexiconindexInverseFile.seek(addresseChaine, 0)
                terme.insert(0, self.lexiconindexInverseFile.litChaine(longueurChaine))
                break
            if flag == FLAG_TERME_COMPOSE:
                #<flagCompose> <identA> <identB>
                identA = self.lexiconindexInverseFile.litNombre3()
                identB = self.lexiconindexInverseFile.litNombre3()
                #<identB> est obligatoirement un terme simple
                self.lexiconindexInverseFile.seek((identB - 1) * TAILLE_DEFINITION, 0)
                if self.lexiconindexInverseFile.litNombre1() != FLAG_TERME_SIMPLE:
                    raise Exception("%d n'est pas un terme simple sur %s"%(identB, self.lexiconindexFileName))
                longueurChaine = self.lexiconindexInverseFile.litNombre1()
                addresseChaine = self.lexiconindexInverseFile.litNombre5()
                self.lexiconindexInverseFile.seek(addresseChaine, 0)
                terme.insert(0, self.lexiconindexInverseFile.litChaine(longueurChaine))
                identCourant = identA
            else: raise Exception("%d n'a pas de définition sur %s"%(identCourant, self.lexiconindexFileName))
            if len(terme) > 10: raise Exception("(%s) bouclage  sur %s"%('#'.join(terme), self.lexiconindexFileName))
        return '#'.join(terme)
                       
if __name__ == '__main__':
    main()
