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
Programme de test de la classe NindLexiconindex.
Cette classe gère le lexique contenu dans le fichier lexique spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche l'identifiant du terme spécifié,
simple ou composé (avec notation dièsée).

usage   : %s <fichier lexiconindex> <terme cherché>
exemple : %s box/dumps/boxon/FRE.lexiconindex syntagme#nominal
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = os.path.abspath(sys.argv[1])
    terme = sys.argv[2].decode('utf-8').strip()
    
    #la classe
    nindLexiconindex = NindLexiconindex(lexiconindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure) = nindLexiconindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    #trouve l'identifiant du terme
    #trouve la clef des termes simples
    termesSimples = terme.split('#')
    sousMotId = 0
    for mot in termesSimples:
        sousMotId = nindLexiconindex.getIdent(mot, sousMotId)
        if sousMotId == 0: break
    if sousMotId == 0: print 'INCONNU'
    else: print 'identifiant=',sousMotId

#<definition>            ::= <flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
#<flagDefinition>        ::= <Integer1>
#<identifiantHash>       ::= <Integer3>
#<longueurDonnees>       ::= <Integer3>
#<donneesHash>           ::= { <terme> }
#<terme>                 ::= <termeSimple> <identifiantS> <nbreComposes> <composes>
#<termeSimple>           ::= <longueurTerme> <termeUtf8>
#<longueurTerme>         ::= <Integer1>
#<termeUtf8>             ::= { <Octet> }
#<identifiantS>          ::= <Integer3>
#<nbreComposes>          ::= <IntegerULat>
#<composes>              ::= { <compose> } 
#<compose>               ::= <identifiantA> <identifiantRelC>
#<identifiantA>          ::= <Integer3>
#<identifiantRelC>       ::= <IntegerSLat>

FLAG_INDIRECTION = 47
FLAG_DEFINITION = 17
TETE_INDIRECTION = 9
TAILLE_INDIRECTION = 8
TETE_DEFINITION = 7

class NindLexiconindex:
    def __init__(self, lexiconindexFileName):
        self.lexiconindexFileName = lexiconindexFileName
        #ouvre le fichier en lecture
        self.lexiconindexFile = NindLateconFile.NindLateconFile(self.lexiconindexFileName)
        #trouve le modulo = nombreIndirection
        #<flagIndirection> <indirectionSuivante> <nombreIndirection>
        self.lexiconindexFile.seek(0, 0)
        if self.lexiconindexFile.litNombre1() != FLAG_INDIRECTION: 
            raise Exception('%s : pas FLAG_INDIRECTION à 0'%(self.lexiconindexFileName))
        self.lexiconindexFile.litNombre5()
        self.nombreIndirection = self.lexiconindexFile.litNombre3()

    def getIdentification(self):
        return NindIndex.getIdentification(self.lexiconindexFile, self.lexiconindexFileName)
    
    def getIdent(self, mot, sousMotId):
        clefB = NindLateconFile.clefB(mot)
        index = clefB % self.nombreIndirection
        #lit la définition du terme
        addrIndir = TETE_INDIRECTION + (index * TAILLE_INDIRECTION)
        self.lexiconindexFile.seek(addrIndir, 0)
        #<offsetDefinition> <longueurDefinition>
        offsetDefinition = self.lexiconindexFile.litNombre5()
        longueurDefinition = self.lexiconindexFile.litNombre3()
        if offsetDefinition == 0: return 0      #identifiant pas trouve
        self.lexiconindexFile.seek(offsetDefinition, 0)
        #<flagDefinition> <identifiantHash> <longueurDonnees> 
        if self.lexiconindexFile.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.lexiconindexFileName, offsetDefinition))
        if self.lexiconindexFile.litNombre3() != index: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.lexiconindexFileName, index, offsetDefinition+1))
        longueurDonnees = self.lexiconindexFile.litNombre3()
        finDonnees = offsetDefinition + longueurDonnees + TETE_DEFINITION
        while self.lexiconindexFile.tell() < finDonnees:
            #<termeSimple> <identifiantS> <nbreComposes> <composes>
            termeSimple = self.lexiconindexFile.litString()
            if termeSimple != mot: 
                #pas le terme cherche, on continue
                self.lexiconindexFile.litNombre3()      #identifiantS
                nbreComposes = self.lexiconindexFile.litNombreULat()
                for i in range(nbreComposes):
                    self.lexiconindexFile.litNombre3()          #identifiantA
                    self.lexiconindexFile.litNombreSLat()       #identifiantRelC
                continue
            #c'est le terme cherche
            identifiantS = self.lexiconindexFile.litNombre3()
            if sousMotId == 0: return identifiantS        #identifiant simple trouve
            nbreComposes = self.lexiconindexFile.litNombreULat()
            identifiantC = identifiantS
            for i in range(nbreComposes):
                #<identifiantA> <identifiantRelC>
                identifiantA = self.lexiconindexFile.litNombre3()
                identifiantC += self.lexiconindexFile.litNombreSLat()
                if sousMotId == identifiantA: return identifiantC    #identifiant compose trouve
        #pas trouve
        return 0

        
if __name__ == '__main__':
    main()
