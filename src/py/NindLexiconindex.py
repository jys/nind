#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv, path
from time import ctime
import NindLateconFile
from NindIndex import NindIndex

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Programme de test de la classe NindLexiconindex.
Cette classe gère le lexique contenu dans le fichier lexique spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche l'identifiant du mot spécifié,
simple ou composé (avec notation dièsée).

usage   : %s <fichier lexiconindex> <mot cherché>
exemple : %s box/dumps/boxon/FRE.lexiconindex syntagme#nominal
"""%(script, script)

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    motCherchej = sys.argv[2].decode('utf-8').strip()
    
    #la classe
    nindLexiconindex = NindLexiconindex(lexiconindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure) = nindLexiconindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    #trouve l'identifiant du mot
    #trouve la clef des mots simples
    motsSimples = motCherchej.split('#')
    sousMotId = 0
    for mot in motsSimples:
        sousMotId = nindLexiconindex.getIdent(mot, sousMotId)
        if sousMotId == 0: break
    if sousMotId == 0: print 'INCONNU'
    else: print 'identifiant=',sousMotId

#// <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
#//
#// <blocIndirection>       ::= <flagIndirection=47> <addrBlocSuivant> <nombreIndirection> { indirection }
#// <flagIndirection=47>    ::= <Integer1>
#// <addrBlocSuivant>       ::= <Integer5>
#// <nombreIndirection>     ::= <Integer3>
#// <indirection>           ::= <offsetDefinition> <longueurDefinition> 
#// <offsetDefinition>      ::= <Integer5>
#// <longueurDefinition>    ::= <Integer3>

#// <definition>            ::= <flagDefinition=17> <identifiantHash> <longueurDonnees> <donneesHash>
#// <flagDefinition=17>     ::= <Integer1>
#// <identifiantHash>       ::= <Integer3>
#// <longueurDonnees>       ::= <Integer3>
#// <donneesHash>           ::= { <mot> }
#// <mot>                   ::= <motSimple> <identifiantS> <nbreComposes> <composes>
#// <motSimple>             ::= <longueurMot> <motUtf8>
#// <longueurMot>           ::= <Integer1>
#// <motUtf8>               ::= { <Octet> }
#// <identifiantS>          ::= <Integer3>
#// <nbreComposes>          ::= <IntegerULat>
#// <composes>              ::= { <compose> } 
#// <compose>               ::= <identifiantA> <identifiantRelC>
#// <identifiantA>          ::= <Integer3>
#// <identifiantRelC>       ::= <IntegerSLat>

FLAG_INDIRECTION = 47
FLAG_DEFINITION = 17
TETE_INDIRECTION = 9
TAILLE_INDIRECTION = 8
TETE_DEFINITION = 7

class NindLexiconindex(NindIndex):
    def __init__(self, lexiconindexFileName):
        NindIndex.__init__(self, lexiconindexFileName)
        #trouve le modulo = nombreIndirection
        #<flagIndirection=47> <addrBlocSuivant> <nombreIndirection>
        self.seek(0, 0)
        if self.litNombre1() != FLAG_INDIRECTION: 
            raise Exception('%s : pas FLAG_INDIRECTION à 0'%(self.latFileName))
        self.litNombre5()
        self.nombreIndirection = self.litNombre3()

    def getIdent(self, mot, sousMotId):
        clefB = NindLateconFile.clefB(mot)
        index = clefB % self.nombreIndirection
        #lit la définition du mot
        addrIndir = TETE_INDIRECTION + (index * TAILLE_INDIRECTION)
        self.seek(addrIndir, 0)
        #<offsetDefinition> <longueurDefinition>
        offsetDefinition = self.litNombre5()
        longueurDefinition = self.litNombre3()
        if offsetDefinition == 0: return 0      #identifiant pas trouve
        self.seek(offsetDefinition, 0)
        #<flagDefinition=17> <identifiantHash> <longueurDonnees> 
        if self.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.latFileName, offsetDefinition))
        if self.litNombre3() != index: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, index, offsetDefinition+1))
        longueurDonnees = self.litNombre3()
        finDonnees = offsetDefinition + longueurDonnees + TETE_DEFINITION
        while self.tell() < finDonnees:
            #<motSimple> <identifiantS> <nbreComposes> <composes>
            motSimple = self.litString()
            if motSimple != mot: 
                #pas le mot cherche, on continue
                self.litNombre3()      #identifiantS
                nbreComposes = self.litNombreULat()
                for i in range(nbreComposes):
                    self.litNombre3()          #identifiantA
                    self.litNombreSLat()       #identifiantRelC
                continue
            #c'est le mot cherche
            identifiantS = self.litNombre3()
            if sousMotId == 0: return identifiantS        #identifiant simple trouve
            nbreComposes = self.litNombreULat()
            identifiantC = identifiantS
            for i in range(nbreComposes):
                #<identifiantA> <identifiantRelC>
                identifiantA = self.litNombre3()
                identifiantC += self.litNombreSLat()
                if sousMotId == identifiantA: return identifiantC    #identifiant compose trouve
        #pas trouve
        return 0

        
if __name__ == '__main__':
    main()
