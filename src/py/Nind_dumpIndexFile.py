#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """© l'ATÉCON.
Analyse un fichier inversé et l'écrit en clair sur un fichier texte. 
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier termindex>-dump.txt
Donne des informations sur la composition du fichier 
et donne quelques statistiques

usage   : %s <fichier termindex>
exemple : %s box/dumps/boxon/FRE.termindex
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    termindexFileName = os.path.abspath(sys.argv[1])
    outFileName = '%s-dump.txt'%(termindexFileName)

# <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
#
# <blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
# <flagIndirection>       ::= <Integer1>
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
# <blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
# <flagIdentification>    ::= <Integer1>
# <maxIdentifiant>        ::= <Integer3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Integer4>
    
    INDIRECTION_FLAG = 47
    DEFINITION_FLAG = 17
    IDENTIFICATION_FLAG = 53
    CG_FLAG = 61
    IDENTIFICATION_SIZE = 8
    INDIRECTION_HEAD = 9
    ENTREE_SIZE = 8
    DEFINITION_HEAD = 7
    DEF_CG_HEAD = 8
    DOCUMENT_SIZE = 5
  
    termindexFile = open(termindexFileName, 'rb')
    termindexFile2 = open(termindexFileName, 'rb')
    
    print "1) vérifie les zones d'indirection" 
    try:
        nonVidesList = []
        blocIndirection = 0
        tailleIndirection = 0
        termindexFile.seek(0, 0)
        while True:
            #<flagIndirection_1> <indirectionSuivante_5> <nombreIndirection_3>
            addrIndirection = termindexFile.tell()
            flagIndirection = litNombre1(termindexFile)
            if flagIndirection != INDIRECTION_FLAG: raise Exception('pas INDIRECTION_FLAG à %08X'%(addrIndirection))
            indirectionSuivante = litNombre5(termindexFile)
            nombreIndirection = litNombre3(termindexFile)
            blocIndirection +=1
            tailleBloc = INDIRECTION_HEAD + nombreIndirection * ENTREE_SIZE
            tailleIndirection += tailleBloc
            #le bloc d'indirection dans les non vides
            nonVidesList.append((addrIndirection, tailleBloc))
            indirectionsUtilisees = 0
            for i in range(nombreIndirection):
                #<offsetEntree_5> <longueurEntree_3>
                offsetEntree = litNombre5(termindexFile)
                longueurEntree = litNombre3(termindexFile)
                if offsetEntree != 0: 
                    indirectionsUtilisees +=1
                    nonVidesList.append((offsetEntree, longueurEntree))
                #else:
                    #if blocIndirection < 7: print '%d : %d'%(blocIndirection, i)
            print "%08X: Bloc indirections n° %d : %d / %d indirections"%(addrIndirection, blocIndirection, indirectionsUtilisees, nombreIndirection)
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
        #raise
    print "%d blocs d'indirection  de taille totale %d octets"%(blocIndirection, tailleIndirection)
    
    print "2) établit la cartographie des vides"
    try:
        #ajoute l'identification dans les non-vides
        #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
        termindexFile.seek(0, 2)
        nonVidesList.append((termindexFile.tell() - IDENTIFICATION_SIZE, IDENTIFICATION_SIZE))
        #ordonne les indirections
        nonVidesList.sort()
        addressePrec = longueurPrec = 0
        nbreVides = tailleVides = 0
        typesVides = {}
        for (addresse, longueur) in nonVidesList:
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
    print "%d zones vides de taille totale %d octets"%(nbreVides, tailleVides)  
    keys = typesVides.keys()
    keys.sort()
    videsText = []
    #for key in keys: videsText.append('%d x %d'%(key, typesVides[key]))
    #print ", ".join(videsText)
    
    print "3) vérifie les zones d'index et les zones d'extension"
    print "et écrit %s"%(outFileName)
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        occurencesGenerale = occurencesDoc = 0
        nbreHapax = 0
        noTerm = 0
        termindexFile.seek(0, 0)
        while True:
            addrIndirection = termindexFile.tell()
            #<flagIndirection_1> <indirectionSuivante_5> <nombreIndirection_3>
            if litNombre1(termindexFile) != INDIRECTION_FLAG: raise Exception('pas INDIRECTION_FLAG à %08X'%(addrIndirection))
            indirectionSuivante = litNombre5(termindexFile)
            nombreIndirection = litNombre3(termindexFile)
            for i in range(nombreIndirection):
                outFile.write('%07d:\n'%(noTerm))
                #<offsetEntree_5> <longueurEntree_3>
                offsetEntree = litNombre5(termindexFile)
                longueurEntree = litNombre3(termindexFile)
                if offsetEntree != 0: 
                    termindexFile2.seek(offsetEntree, 0)
                    #<flagDefinition_1> <identTerme_3> <longueurDonnees_3>
                    if litNombre1(termindexFile2) != DEFINITION_FLAG: raise Exception('pas DEFINITION_FLAG à %08X'%(offsetEntree))
                    if litNombre3(termindexFile2) != noTerm: raise Exception('%d pas trouvé à %08X'%(noTerm, offsetEntree+1))
                    longueurDonnees = litNombre3(termindexFile2)
                    nbreDefinition +=1
                    tailleDefinition += longueurDonnees + DEFINITION_HEAD
                    extension = longueurEntree - longueurDonnees - DEFINITION_HEAD
                    #print "offsetEntree=%d longueurEntree=%d longueurDonnees=%d"%(offsetEntree, longueurEntree, longueurDonnees)
                    if extension > 0: 
                        nbreExtension +=1
                        tailleExtension += extension
                    elif extension < 0: raise Exception('%d incohérent à %08X'%(noTerm, offsetEntree+5))
                    #examine les données
                    finDonnees = offsetEntree + longueurDonnees + DEFINITION_HEAD
                    while termindexFile2.tell() < finDonnees:
                        #<flagCg_1> <categorie_1> <frequenceTerme> <nbreDocs> <listeDocuments>
                        if litNombre1(termindexFile2) != CG_FLAG: raise Exception('pas CG_FLAG à %d'%(termindexFile2.tell() -1))
                        categorie = litNombre1(termindexFile2)
                        frequenceTerme = litNombreLat(termindexFile2)
                        nbreDocs = litNombreLat(termindexFile2)
                        outFile.write('[%d] (%d) <%d>'%(categorie, frequenceTerme, nbreDocs))
                        #print  "frequenceTerme=%d nbreDocs=%d"%(frequenceTerme, nbreDocs)
                        totalFrequences = 0
                        noDoc = 0
                        docList = []
                        for i in range(nbreDocs):
                            #<identDocument> <frequenceDoc>
                            incrementIdentDoc = litNombreLat(termindexFile2)
                            frequenceDoc = litNombreLat(termindexFile2)
                            totalFrequences += frequenceDoc
                            noDoc += incrementIdentDoc
                            docList.append('%05d (%d)'%(noDoc, frequenceDoc))
                        outFile.write(' :: %s\n'%(', '.join(docList)))
                        if totalFrequences != frequenceTerme: raise Exception('fréquences incomptibles sur terme %d'%(noTerm))
                        occurencesGenerale += totalFrequences
                        occurencesDoc += nbreDocs 
                        if totalFrequences == 1: nbreHapax +=1
                outFile.write('\n')
                noTerm +=1
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    outFile.close()
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print "%d occurrences de couples termes-doc"%(occurencesDoc)
    print "%d occurrences de termes"%(occurencesGenerale)
    print "%d hapax (%0.2f %%)"%(nbreHapax, float(100)*nbreHapax/nbreDefinition)
    
    print "4) vérifie l'identification"
    #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
    termindexFile.seek(-IDENTIFICATION_SIZE, 2)
    addrIdentification = termindexFile.tell()
    if litNombre1(termindexFile) != IDENTIFICATION_FLAG: raise Exception('pas IDENTIFICATION_FLAG à %08X'%(addrIdentification))
    maxIdentifiant = litNombre3(termindexFile)
    dateHeure = litNombre4(termindexFile)
    print "max=%d dateheure=%d"%(maxIdentifiant, dateHeure)
    
    termindexFile.seek(0, 2)
    offsetFin = termindexFile.tell()
    total = tailleIndirection + tailleDefinition + tailleVides + tailleExtension + IDENTIFICATION_SIZE
    
    print "6) récapitulatif :"
    print "INDIRECTION    % 10d (%0.2f %%)"%(tailleIndirection, float(100)*tailleIndirection/total)
    print "DEFINITION     % 10d (%0.2f %%)"%(tailleDefinition, float(100)*tailleDefinition/total)
    print "EXTENSION      % 10d (%0.2f %%)"%(tailleExtension, float(100)*tailleExtension/total)
    print "VIDE           % 10d (%0.2f %%)"%(tailleVides, float(100)*tailleVides/total)
    print "IDENTIFICATION % 10d (%0.2f %%)"%(IDENTIFICATION_SIZE, float(100)*IDENTIFICATION_SIZE/total)
    print "TOTAL          % 10d %08X"%(total, total)
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    print "(%0.2f octets / occurence de terme-doc"%(float(total)/occurencesDoc)
    print "(%0.2f octets / occurence de terme"%(float(total)/occurencesGenerale)
    termindexFile.close()
    termindexFile2.close()

def litNombre1(termindexFile):
    oc = termindexFile.read(1)
    return ord(oc)

def litNombre2(termindexFile):
    #big-endian
    oc = termindexFile.read(2)
    return ord(oc[0])*256 + ord(oc[1])

def litNombre3(termindexFile):
    #little-endian
    oc = termindexFile.read(3)
    return (ord(oc[2])*256 + ord(oc[1]))*256 + ord(oc[0])

def litNombre4(termindexFile):
    #little-endian
    oc = termindexFile.read(4)
    return ((ord(oc[3])*256 + ord(oc[2]))*256 + ord(oc[1]))*256 + ord(oc[0])

def litNombre5(termindexFile):
    #big-endian
    oc = termindexFile.read(5)
    return (((ord(oc[0])*256 + ord(oc[1]))*256 + ord(oc[2]))*256 + ord(oc[3]))*256 + ord(oc[4])

def litNombreLat(termindexFile):
    octet = ord(termindexFile.read(1))
    if not octet&0x80: return octet
    result = ord(termindexFile.read(1))
    if not octet&0x40: return (octet&0x3F) * 256 + result
    result = result * 256 + ord(termindexFile.read(1))
    if not octet&0x20: return (octet&0x1F) * 256 * 256 + result
    result = result * 256 + ord(termindexFile.read(1))
    if not octet&0x10: return (octet&0x0F) * 256 * 256 * 256 + result
    result = result * 256 + ord(termindexFile.read(1))
    if not octet&0x0F: return result
    raise Exception('entier latecon invalide à %08X'%(termindexFile.tell()))
    
if __name__ == '__main__':
        main()
    
        