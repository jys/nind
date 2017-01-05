#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import path, getenv
import codecs
import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un fichier lexique index et écrit en clair sa structure 
sur un fichier texte. 
(Ne pas confondre avec Nind_dumpLexicon.py qui dumpe le lexique en clair)
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier lexiconindex>-dump.txt
Donne des informations sur la composition du fichier 
et donne quelques statistiques

usage   : %s <fichier lexiconindex>
exemple : %s FRE.lexiconindex
"""%(script, script)

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    inFileName = path.abspath(sys.argv[1])
    outFileName = '%s-dump.txt'%(inFileName)
        
    #<fichier>               ::= { <blocIndirection> <blocDefinition> } <blocIdentification> 

    #<blocIndirection>       ::= <flagIndirection=47> <addrBlocSuivant> <nombreIndirection> { indirection }
    #<flagIndirection=47>    ::= <Integer1>
    #<addrBlocSuivant>       ::= <Integer5>
    #<nombreIndirection>     ::= <Integer3>
    #<indirection>           ::= <offsetDefinition> <longueurDefinition> 
    #<offsetDefinition>      ::= <Integer5>
    #<longueurDefinition>    ::= <Integer3>

    #<blocDefinition>        ::= { <definition> | <vide> }
    #<definition>            ::= <flagDefinition=17> <identifiantHash> <longueurDonnees> <donneesHash>
    #<flagDefinition=17>     ::= <Integer1>
    #<identifiantHash>       ::= <Integer3>
    #<longueurDonnees>       ::= <Integer3>
    #<donneesHash>           ::= { <mot> }
    #<mot>                 ::= <motSimple> <identifiantS> <nbreComposes> <composes>
    #<motSimple>           ::= <longueurMot> <motUtf8>
    #<longueurMot>         ::= <Integer1>
    #<motUtf8>             ::= { <Octet> }
    #<identifiantS>          ::= <Integer3>
    #<nbreComposes>          ::= <IntegerULat>
    #<composes>              ::= { <compose> } 
    #<compose>               ::= <identifiantA> <identifiantRelC>
    #<identifiantA>          ::= <Integer3>
    #<identifiantRelC>       ::= <IntegerSLat>
    #<vide>                  ::= { <Octet> }

    #<blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique> <identifieurSpecifique>
    #<flagIdentification=53> ::= <Integer1>
    #<maxIdentifiant>        ::= <Integer3>
    #<identifieurUnique>     ::= <dateHeure>
    #<dateHeure >            ::= <Integer4>
    #<identifieurSpecifique> ::= <Integer4>
    
    FLAG_INDIRECTION = 47
    FLAG_DEFINITION = 17
    FLAG_IDENTIFICATION = 53
    TAILLE_IDENTIFICATION = 12
    TETE_INDIRECTION = 9
    TAILLE_INDIRECTION = 8
    TETE_DEFINITION = 7
    
    inFile = NindLateconFile.NindLateconFile(inFileName)
    inFile2 = NindLateconFile.NindLateconFile(inFileName)
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        nbreMotsS = nbreMotsC = nbreDef = 0
        noDef = 0
        repartDef = {}
        maxComposejs = []
        inFile.seek(0, 0)
        addrIndirection = inFile.tell()
        #<flagIndirection=47> <addrBlocSuivant> <nombreIndirection>
        if inFile.litNombre1() != FLAG_INDIRECTION: raise Exception('pas FLAG_INDIRECTION à %08X'%(addrIndirection))
        if inFile.litNombre5() != 0: raise Exception("plusieurs blocs d'indirection")
        nombreIndirection = inFile.litNombre3()
        for i in range(nombreIndirection):
            #<offsetDefinition> <longueurDefinition>
            offsetDefinition = inFile.litNombre5()
            longueurDefinition = inFile.litNombre3()
            nbreMots = 0
            if offsetDefinition != 0: 
                nbreDef +=1
                inFile2.seek(offsetDefinition, 0)
                #<flagDefinition=17> <identifiantHash> <longueurDonnees> 
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
                    nbreMotsS +=1
                    nbreMots +=1
                    #<motSimple> <identifiantS> <nbreComposes> <composes>
                    motSimple = inFile2.litString()
                    identifiantS = inFile2.litNombre3()
                    nbreComposes = inFile2.litNombreULat()
                    maxComposejs.append((nbreComposes, motSimple))
                    maxComposejs.sort()
                    maxComposejs.reverse()
                    if len(maxComposejs) > 10: maxComposejs.pop()
                    outFile.write('[%s] %06d (%d) '%(motSimple, identifiantS, nbreComposes))
                    identifiantC = identifiantS
                    composes = []
                    for i in range(nbreComposes):
                        nbreMotsC +=1
                        #<identifiantA> <identifiantRelC>
                        identifiantA = inFile2.litNombre3()
                        identifiantC += inFile2.litNombreSLat()
                        composes.append('%06d %06d'%(identifiantA, identifiantC))
                    outFile.write(' <%s>\n'%(', '.join(composes)))
            noDef +=1
            if nbreMots not in repartDef: repartDef[nbreMots] = 0
            repartDef[nbreMots] +=1
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    outFile.close()
    print "Dump écrit sur %s"%(outFileName)
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print "%d définitions"%(nbreDef)
    print "%d mots simples"%(nbreMotsS)
    print "%d mots composés"%(nbreMotsC)
    inFile.seek(0, 2)
    offsetFin = inFile.tell()
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    print "%0.2f octets / mot"%(float(offsetFin)/(nbreMotsS+nbreMotsC))
    inFile.close()
    inFile2.close()
    print
    keys = repartDef.keys()
    keys.sort()
    resultRepart = []
    for key in keys: resultRepart.append('%d:%d'%(key, repartDef[key]))
    print 'répartition des %d mots sur les %d indirections :'%(nbreMotsS, nombreIndirection)
    print ', '.join(resultRepart)
    print
    print 'top 10 des listes de mots composés les plus longues avec le mot terminal:'
    for (nbreComposes, motSimple) in maxComposejs:
        print u'% 5d composés pour "%s"'%(nbreComposes, motSimple)
    
if __name__ == '__main__':
        main()
