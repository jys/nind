#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv, path
#import codecs
#import datetime
from time import ctime
from NindLateconFile import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Programme de test de la classe NindIndex.
La classe NindIndex est dérivée de la classe NindLateconFile,
elle implante les opérations sur les index.
Le programme de test dumpe en octets la définition de l'identifiant spécifié.

usage   : %s <fichier termindex> <identifiant>
exemple : %s amose-dump.localindex 75
"""%(script, script)

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    indexFileName = path.abspath(sys.argv[1])
    ident = int(sys.argv[2])
    
    #la classe
    nindIndex = NindIndex(indexFileName)
    #identification du fichier
    (maxIdentifiant, dateHeure) = nindIndex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    #affiche l'indirection
    (offsetDefinition, longueurDefinition) = nindIndex.getDefinitionAddr(ident)
    print "offset=%d longueur=%d"%(offsetDefinition, longueurDefinition)
    if offsetDefinition == 0: print 'PAS DE DÉFINITION'
    else:
        #lit l'indirection
        nindIndex.seek(offsetDefinition, 0)
        #<flagDefinition> <identifiantTerme> <longueurDonnees>
        flagDefinition = nindIndex.litNombre1()
        print '<flagDefinition>   = %d'%(flagDefinition)
        identifiantTerme = nindIndex.litNombre3()
        print '<identifiant> = %d'%(identifiantTerme)
        longueurDonnees = nindIndex.litNombre3()
        print '<longueurDonnees>  = %d'%(longueurDonnees)
        octets = []
        for i in range(longueurDonnees): octets.append(str(nindIndex.litNombre1()))
        print ', '.join(octets)


#utilitaires communs aux classes NindLexiconindex, NindLexiconindexInverse, NindLocalindex, NindTermindex
# <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
#
# <blocIndirection>       ::= <flagIndirection=47> <addrBlocSuivant> <nombreIndirection> { indirection }
# <flagIndirection=47>    ::= <Integer1>
# <addrBlocSuivant>       ::= <Integer5>
# <nombreIndirection>     ::= <Integer3>
# <indirection>           ::= <offsetDefinition> <longueurDefinition> 
# <offsetDefinition>      ::= <Integer5>
# <longueurDefinition>    ::= <Integer3>
#
# <blocDefinition>        ::= { <definition> | <vide> }
# <definition>            ::= { <Octet> }
# <vide>                  ::= { <Octet> }
#
# <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique>
# <flagIdentification=53> ::= <Integer1>
# <maxIdentifiant>        ::= <Integer3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Integer4>

class NindIndex(NindLateconFile):
    
    def __init__(self, latFileName):
        #en lecture uniquement
        NindLateconFile.__init__(self, latFileName, False)
    
    def getIdentification(self):
        FLAG_IDENTIFICATION = 53
        TAILLE_IDENTIFICATION = 12
        #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
        self.seek(-TAILLE_IDENTIFICATION, 2)
        if self.litNombre1() != FLAG_IDENTIFICATION: raise Exception('pas FLAG_IDENTIFICATION sur %s'%(self.latFileName))
        maxIdentifiant = self.litNombre3()
        dateHeure = self.litNombre4()
        return (maxIdentifiant, dateHeure)
        
    def getDefinitionAddr(self, identifiant):
        FLAG_INDIRECTION = 47  
        TETE_INDIRECTION = 9
        TAILLE_INDIRECTION = 8
        self.seek(0, 0)
        maxIdent = 0
        startIndirection = 0
        while True:
            addrIndirection = self.tell()
            #<flagIndirection=47> <addrBlocSuivant> <nombreIndirection>
            if self.litNombre1() != FLAG_INDIRECTION: raise Exception('%s : pas FLAG_INDIRECTION à %08X'%(self.latFileName, addrIndirection))
            indirectionSuivante = self.litNombre5()
            nombreIndirection = self.litNombre3()
            maxIdent += nombreIndirection
            if maxIdent > identifiant: break
            if indirectionSuivante == 0: break
            startIndirection = indirectionSuivante
            self.seek(startIndirection, 0)
        if maxIdent < identifiant: return (0, 0)          #identifiant hors limite
        #lit l'indirection
        index = identifiant + nombreIndirection - maxIdent
        #lit la définition du terme
        addrIndir = startIndirection + TETE_INDIRECTION + (index * TAILLE_INDIRECTION)
        self.seek(addrIndir, 0)
        #<offsetDefinition> <longueurDefinition>
        offsetDefinition = self.litNombre5()
        longueurDefinition = self.litNombre3()
        return (offsetDefinition, longueurDefinition)
        
if __name__ == '__main__':
    main()
