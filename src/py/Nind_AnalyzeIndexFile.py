#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import path, getenv
import codecs
import datetime
import time
import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un fichier index (lexique, inversé ou d'index locaux) 
et affiche les stats. 
Le format du fichier est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier termindex>
exemple : %s box/dumps/boxon/FRE.termindex
"""%(script, script)

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    termindexFileName = path.abspath(sys.argv[1])

    #// <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
    #//
    #// <blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
    #// <flagIndirection>       ::= <Integer1>
    #// <addrBlocSuivant>       ::= <Integer5>
    #// <nombreIndirection>     ::= <Integer3>
    #// <indirection>           ::= <offsetDefinition> <longueurDefinition> 
    #// <offsetDefinition>      ::= <Integer5>
    #// <longueurDefinition>    ::= <Integer3>
    #//
    #// <blocDefinition>        ::= { <definition> | <vide> }
    #// <definition>            ::= { <octet> }
    #// <definition>            ::= <flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
    #// <definition>            ::= <flagDefinition> <identifiantDoc> <longueurDonnees> <donneesDoc>
    #//<definition>             ::= <flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
    #// <flagDefinition>        ::= <Integer1>
    #// <identifiantTerme>      ::= <Integer3>
    #// <identifiantDoc>        ::= <Integer3>
    #// <longueurDonnees>       ::= <Integer3>
    #// <vide>                  ::= { <octet> }
    #//
    #// <blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
    #// <flagIdentification>    ::= <Integer1>
    #// <maxIdentifiant>        ::= <Integer3>
    #// <identifieurUnique>     ::= <dateHeure>
    #// <dateHeure >            ::= <Integer4>
    
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
  
    termindexFile = NindLateconFile.NindLateconFile(termindexFileName)
    termindexFile2 = NindLateconFile.NindLateconFile(termindexFileName)
    
    print "1) vérifie les zones d'indirection" 
    try:
        nonVidesList = []
        blocIndirection = 0
        tailleIndirection = 0
        while True:
            #<flagIndirection> <indirectionSuivante> <nombreIndirection>
            addrIndirection = termindexFile.tell()
            flagIndirection = termindexFile.litNombre1()
            if flagIndirection != FLAG_INDIRECTION: raise Exception('pas FLAG_INDIRECTION à %08X'%(addrIndirection))
            indirectionSuivante = termindexFile.litNombre5()
            nombreIndirection = termindexFile.litNombre3()
            blocIndirection +=1
            tailleBloc = TETE_INDIRECTION + nombreIndirection * TAILLE_INDIRECTION
            tailleIndirection += tailleBloc
            #le bloc d'indirection dans les non vides
            nonVidesList.append((addrIndirection, tailleBloc))
            indirectionsUtilisees = 0
            for i in range(nombreIndirection):
                #<offsetDefinition> <longueurDefinition>
                offsetDefinition = termindexFile.litNombre5()
                longueurDefinition = termindexFile.litNombre3()
                if offsetDefinition != 0: 
                    indirectionsUtilisees +=1
                    nonVidesList.append((offsetDefinition, longueurDefinition))
                #else:
                    #if blocIndirection < 7: print '%d : %d'%(blocIndirection, i)
            print "%08X: Bloc indirections n° %d : %d / %d indirections"%(addrIndirection, blocIndirection, indirectionsUtilisees, nombreIndirection)
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
        #raise
    print "%d blocs d'indirection  de taille totale %d octets"%(blocIndirection, tailleIndirection)
    print
    
    print "2) établit la cartographie des vides"
    try:
        #ajoute l'identification dans les non-vides
        #<flagIdentification> <maxIdentifiant> <identifieurUnique>
        termindexFile.seek(0, 2)
        nonVidesList.append((termindexFile.tell() - TAILLE_IDENTIFICATION, TAILLE_IDENTIFICATION))
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
            if longueurVide < 0: raise Exception('chevauchement %08X-%d et %08X-%d'%(addressePrec, longueurPrec, addresse, longueur))
            if longueurVide > 0:
                nbreVides +=1
                tailleVides += longueurVide
                if longueurVide not in typesVides: typesVides[longueurVide] = 0
                typesVides[longueurVide] +=1
            addressePrec = addresse
            longueurPrec = longueur        
    except Exception as exc: print 'ERREUR :', exc.args[0]  
    
    #affiche(typesNonVides)
    print "%d zones vides de taille totale %d octets"%(nbreVides, tailleVides)  
    #affiche(typesVides)
    print
    
    print "3) vérifie les zones d'index et les zones d'extension"
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        nbreHapax = 0
        noDefinition = 0
        termindexFile.seek(0, 0)
        while True:
            addrIndirection = termindexFile.tell()
            #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
            if termindexFile.litNombre1() != FLAG_INDIRECTION: raise Exception('pas FLAG_INDIRECTION à %08X'%(addrIndirection))
            indirectionSuivante = termindexFile.litNombre5()
            nombreIndirection = termindexFile.litNombre3()
            for i in range(nombreIndirection):
                #<offsetDefinition> <longueurDefinition>
                offsetDefinition = termindexFile.litNombre5()
                longueurDefinition = termindexFile.litNombre3()
                if offsetDefinition != 0: 
                    termindexFile2.seek(offsetDefinition, 0)
                    #<flagDefinition> <identifiantTerme> <longueurDonnees>
                    #<flagDefinition> <identifiantDoc> <longueurDonnees>
                    #<flagDefinition> <identifiantHash> <longueurDonnees> 
                    if termindexFile2.litNombre1() != FLAG_DEFINITION: raise Exception('pas FLAG_DEFINITION à %08X'%(offsetDefinition))
                    if termindexFile2.litNombre3() != noDefinition: raise Exception('%d pas trouvé à %08X'%(noDefinition, offsetDefinition+1))
                    longueurDonnees = termindexFile2.litNombre3()
                    nbreDefinition +=1
                    tailleDefinition += longueurDonnees + TETE_DEFINITION
                    extension = longueurDefinition - longueurDonnees - TETE_DEFINITION
                    #print "offsetDefinition=%d longueurDefinition=%d longueurDonnees=%d"%(offsetDefinition, longueurDefinition, longueurDonnees)
                    if extension > 0: 
                        nbreExtension +=1
                        tailleExtension += extension
                    elif extension < 0: raise Exception('%d incohérent à %08X'%(noDefinition, offsetDefinition+5))
                noDefinition +=1
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print
    
    print "4) vérifie l'identification"
    try:
        #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
        termindexFile.seek(-TAILLE_IDENTIFICATION, 2)
        addrIdentification = termindexFile.tell()
        if termindexFile.litNombre1() != FLAG_IDENTIFICATION: raise Exception('pas FLAG_IDENTIFICATION à %08X'%(addrIdentification))
        maxIdentifiant = termindexFile.litNombre3()
        dateHeure = termindexFile.litNombre4()
        print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
        
    termindexFile.seek(0, 2)
    offsetFin = termindexFile.tell()
    total = tailleIndirection + tailleDefinition + tailleVides + tailleExtension + TAILLE_IDENTIFICATION
    print
    
    print "5) récapitulatif :"
    print "INDIRECTION    % 10d (%0.2f %%)"%(tailleIndirection, float(100)*tailleIndirection/total)
    print "DEFINITION     % 10d (%0.2f %%)"%(tailleDefinition, float(100)*tailleDefinition/total)
    print "EXTENSION      % 10d (%0.2f %%)"%(tailleExtension, float(100)*tailleExtension/total)
    print "VIDE           % 10d (%0.2f %%)"%(tailleVides, float(100)*tailleVides/total)
    print "IDENTIFICATION % 10d (%0.2f %%)"%(TAILLE_IDENTIFICATION, float(100)*TAILLE_IDENTIFICATION/total)
    print "TOTAL          % 10d %08X"%(total, total)
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    termindexFile.close()
    termindexFile2.close()
    
def affiche(types):
        keys = types.keys()
        keys.sort()
        typesText = []
        for key in keys: typesText.append('%d x %d'%(key, types[key]))
        print ", ".join(typesText)
        print


if __name__ == '__main__':
        main()
    
        