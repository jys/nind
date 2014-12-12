#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import datetime
import time
import NindLateconFile

def usage():
    print """© l'ATÉCON.
Trouve la définition du terme spécifié dans le fichier lexique spécifié.  
Le format du fichier est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier lexiconindex> <terme>
exemple : %s box/dumps/boxon/FRE.lexiconindex prestation#maintenance#associé
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    inFileName = os.path.abspath(sys.argv[1])
    terme = sys.argv[2].decode('utf-8').strip()
    
    #<fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 

    #<blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
    #<flagIndirection>       ::= <Integer1>
    #<addrBlocSuivant>       ::= <Integer5>
    #<nombreIndirection>     ::= <Integer3>
    #<indirection>           ::= <offsetDefinition> <longueurDefinition> 
    #<offsetDefinition>      ::= <Integer5>
    #<longueurDefinition>    ::= <Integer3>

    #<blocDefinition>        ::= { <definition> | <vide> }
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
    #<vide>                  ::= { <Octet> }

    #<blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
    #<flagIdentification>    ::= <Integer1>
    #<maxIdentifiant>        ::= <Integer3>
    #<identifieurUnique>     ::= <dateHeure>
    #<dateHeure >            ::= <Integer4>
    
    FLAG_INDIRECTION = 47
    FLAG_DEFINITION = 17
    FLAG_IDENTIFICATION = 53
    #<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
    TAILLE_IDENTIFICATION = 8
    #<flagIndirection> <addrBlocSuivant> <nombreIndirection> = 9
    TETE_INDIRECTION = 9
    #<offsetDefinition> <longueurDefinition> = 8
    TAILLE_INDIRECTION = 8
    #<flagDefinition> <identifiantTerme> <longueurDonnees> = 7
    #<flagDefinition> <identifiantDoc> <longueurDonnees> = 7
    TETE_DEFINITION = 7
    
    inFile = NindLateconFile.NindLateconFile(inFileName)
    
    #1) trouve le modulo = nombreIndirection
    #<flagIndirection> <indirectionSuivante> <nombreIndirection>
    flagIndirection = inFile.litNombre1()
    if flagIndirection != FLAG_INDIRECTION: raise Exception('pas FLAG_INDIRECTION à 0')
    inFile.litNombre5()
    nombreIndirection = inFile.litNombre3()
    print 'nombreIndirection=', nombreIndirection
    
    termesSimples = terme.split('#')
    sousMotId = 0
    for mot in termesSimples:
        #2)trouve la clef des termes simples
        clefB = NindLateconFile.clefB(mot)
        index = clefB % nombreIndirection
        print '[%08X]  %06X  %s'%(clefB, index, mot)
        
        #3)lit la définition du terme
        addrIndir = TETE_INDIRECTION + (index * TAILLE_INDIRECTION)
        inFile.seek(addrIndir, 0)
        #<offsetDefinition> <longueurDefinition>
        offsetDefinition = inFile.litNombre5()
        longueurDefinition = inFile.litNombre3()
        print '    %08X (%d)'%(offsetDefinition, longueurDefinition)
        if offsetDefinition != 0: 
            inFile.seek(offsetDefinition, 0)
            #<flagDefinition> <identifiantHash> <longueurDonnees> 
            if inFile.litNombre1() != FLAG_DEFINITION: raise Exception('pas FLAG_DEFINITION à %08X'%(offsetDefinition))
            if inFile.litNombre3() != index: raise Exception('%d pas trouvé à %08X'%(index, offsetDefinition+1))
            longueurDonnees = inFile.litNombre3()
            finDonnees = inFile.tell() + longueurDonnees
            mapTermes = []
            mapComposes = []
            while inFile.tell() < finDonnees:
                #<termeSimple> <identifiantS> <nbreComposes> <composes>
                termeSimple = inFile.litString()
                cestLui = (termeSimple == mot)
                identifiantS = inFile.litNombre3()
                nbreComposes = inFile.litNombreULat()
                mapTermes.append('[%s] %06d (%d)'%(termeSimple, identifiantS, nbreComposes))
                identifiantC = identifiantS
                ident = 0
                for i in range(nbreComposes):
                    #<identifiantA> <identifiantRelC>
                    identifiantA = inFile.litNombre3()
                    identifiantC += inFile.litNombreSLat()
                    if cestLui & (sousMotId == identifiantA): 
                        ident = identifiantC
                        mapComposes.append('%06d + %06d -> %06d'%(identifiantS, identifiantA, ident))
                if cestLui & (sousMotId == 0): 
                    ident = identifiantS                
                    mapComposes.append('%06d'%(ident))
                if ident != 0: sousMotId = ident
            print '        (%d)  %s'%(longueurDonnees, ', '.join(mapTermes))
            print '            %s'%(', '.join(mapComposes))
        
if __name__ == '__main__':
        main()
    
