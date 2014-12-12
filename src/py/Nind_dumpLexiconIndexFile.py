#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import NindLateconFile

def usage():
    print """© l'ATÉCON.
Analyse un fichier lexique index et l'écrit en clair sur un fichier texte. 
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier lexiconindex>-dump.txt
Donne des informations sur la composition du fichier 
et donne quelques statistiques

usage   : %s <fichier lexiconindex>
exemple : %s box/dumps/boxon/FRE.lexiconindex
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    inFileName = os.path.abspath(sys.argv[1])
    outFileName = '%s-dump.txt'%(inFileName)
        
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
    TAILLE_IDENTIFICATION = 8
    TETE_INDIRECTION = 9
    TAILLE_INDIRECTION = 8
    TETE_DEFINITION = 7
    
    inFile = NindLateconFile.NindLateconFile(inFileName)
    inFile2 = NindLateconFile.NindLateconFile(inFileName)
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        nbreTermesS = nbreTermesC = nbreDef = 0
        noDef = 0
        repartDef = {}
        inFile.seek(0, 0)
        addrIndirection = inFile.tell()
        #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
        if inFile.litNombre1() != FLAG_INDIRECTION: raise Exception('pas FLAG_INDIRECTION à %08X'%(addrIndirection))
        if inFile.litNombre5() != 0: raise Exception("plusieurs blocs d'indirection")
        nombreIndirection = inFile.litNombre3()
        for i in range(nombreIndirection):
            #<offsetDefinition> <longueurDefinition>
            offsetDefinition = inFile.litNombre5()
            longueurDefinition = inFile.litNombre3()
            nbreTermes = 0
            if offsetDefinition != 0: 
                nbreDef +=1
                inFile2.seek(offsetDefinition, 0)
                #<flagDefinition> <identifiantHash> <longueurDonnees> 
                if inFile2.litNombre1() != FLAG_DEFINITION: raise Exception('pas FLAG_DEFINITION à %08X'%(offsetDefinition))
                if inFile2.litNombre3() != noDef: raise Exception('%d pas trouvé à %08X'%(noDef, offsetDefinition+1))
                longueurDonnees = inFile2.litNombre3()
                nbreDefinition +=1
                tailleDefinition += longueurDonnees + TETE_DEFINITION
                extension = longueurDefinition - longueurDonnees - TETE_DEFINITION
                if extension > 0: 
                    nbreExtension +=1
                    tailleExtension += extension
                elif extension < 0: raise Exception('%d incohérent à %08X'%(noDef, offsetDefinition+5))
                #examine les données
                finDonnees = offsetDefinition + longueurDonnees + TETE_DEFINITION
                while inFile2.tell() < finDonnees:
                    nbreTermesS +=1
                    nbreTermes +=1
                    #<termeSimple> <identifiantS> <nbreComposes> <composes>
                    termeSimple = inFile2.litString()
                    identifiantS = inFile2.litNombre3()
                    nbreComposes = inFile2.litNombreULat()
                    outFile.write('[%s] %06d (%d) '%(termeSimple, identifiantS, nbreComposes))
                    identifiantC = identifiantS
                    composes = []
                    for i in range(nbreComposes):
                        nbreTermesC +=1
                        #<identifiantA> <identifiantRelC>
                        identifiantA = inFile2.litNombre3()
                        identifiantC += inFile2.litNombreSLat()
                        composes.append('%06d %06d'%(identifiantA, identifiantC))
                    outFile.write(' <%s>\n'%(', '.join(composes)))
            noDef +=1
            if nbreTermes not in repartDef: repartDef[nbreTermes] = 0
            repartDef[nbreTermes] +=1
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    outFile.close()
    print "Dump écrit sur %s"%(outFileName)
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print "%d définitions"%(nbreDef)
    print "%d termes simples"%(nbreTermesS)
    print "%d termes composés"%(nbreTermesC)
    inFile.seek(0, 2)
    offsetFin = inFile.tell()
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    print "%0.2f octets / terme"%(float(offsetFin)/(nbreTermesS+nbreTermesC))
    inFile.close()
    inFile2.close()
    print
    keys = repartDef.keys()
    keys.sort()
    resultRepart = []
    for key in keys: resultRepart.append('%d:%d'%(key, repartDef[key]))
    print 'répartition des %d termes sur les %d indirections'%(nbreTermesS, nombreIndirection)
    print ', '.join(resultRepart)
    
if __name__ == '__main__':
        main()
