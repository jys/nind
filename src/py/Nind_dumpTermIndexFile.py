#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import NindLateconFile

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

    #<fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 

    #<blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
    #<flagIndirection>       ::= <Integer1>
    #<addrBlocSuivant>       ::= <Integer5>
    #<nombreIndirection>     ::= <Integer3>
    #<indirection>           ::= <offsetDefinition> <longueurDefinition> 
    #<offsetDefinition>      ::= <Integer5>
    #<longueurDefinition>    ::= <Integer3>

    #<blocDefinition>        ::= { <definition> | <vide> }
    #<definition>            ::= <flagDefinition> <identifiantTerme> <longueurDonnees> <donneesTerme>
    #<flagDefinition>        ::= <Integer1>
    #<identifiantTerme>      ::= <Integer3>
    #<longueurDonnees>       ::= <Integer3>
    #<donneesTerme>          ::= { <donneesCG> }
    #<donneesCG>             ::= <flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
    #<flagCg>                ::= <Integer1>
    #<categorie>             ::= <Integer1>
    #<frequenceTerme>        ::= <IntegerULat>
    #<nbreDocs>              ::= <IntegerULat>
    #<listeDocuments>        ::= { <identDocRelatif> <frequenceDoc> }
    #<identDocRelatif>       ::= <IntegerULat>
    #<frequenceDoc>          ::= <IntegerULat>
    #<vide>                  ::= { <octet> }

    #<blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
    #<flagIdentification>    ::= <Integer1>
    #<maxIdentifiant>        ::= <Integer3>
    #<identifieurUnique>     ::= <dateHeure>
    #<dateHeure >            ::= <Integer4>    
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
  
    termindexFile = NindLateconFile.NindLateconFile(termindexFileName)
    termindexFile2 = NindLateconFile.NindLateconFile(termindexFileName)
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        occurencesGenerale = occurencesDoc = 0
        nbreHapax = 0
        noTerm = 0
        maxFreq = 0
        termindexFile.seek(0, 0)
        while True:
            addrIndirection = termindexFile.tell()
            #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
            if termindexFile.litNombre1() != INDIRECTION_FLAG: raise Exception('pas INDIRECTION_FLAG à %08X'%(addrIndirection))
            indirectionSuivante = termindexFile.litNombre5()
            nombreIndirection = termindexFile.litNombre3()
            for i in range(nombreIndirection):
                outFile.write('%06d:\n'%(noTerm))
                frequenceGlobale = 0
                #<offsetDefinition> <longueurDefinition> 
                offsetEntree = termindexFile.litNombre5()
                longueurEntree = termindexFile.litNombre3()
                if offsetEntree != 0: 
                    termindexFile2.seek(offsetEntree, 0)
                    #<flagDefinition> <identifiantTerme> <longueurDonnees>
                    if termindexFile2.litNombre1() != DEFINITION_FLAG: raise Exception('pas DEFINITION_FLAG à %08X'%(offsetEntree))
                    if termindexFile2.litNombre3() != noTerm: raise Exception('%d pas trouvé à %08X'%(noTerm, offsetEntree+1))
                    longueurDonnees = termindexFile2.litNombre3()
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
                        #<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
                        if termindexFile2.litNombre1() != CG_FLAG: raise Exception('pas CG_FLAG à %d'%(termindexFile2.tell() -1))
                        categorie = termindexFile2.litNombre1()
                        frequenceTerme = termindexFile2.litNombreULat()
                        nbreDocs = termindexFile2.litNombreULat()
                        outFile.write('[%d] (%d) <%d>'%(categorie, frequenceTerme, nbreDocs))
                        frequenceGlobale += frequenceTerme
                        totalFrequences = 0
                        noDoc = 0
                        docList = []
                        for i in range(nbreDocs):
                            #<identDocRelatif> <frequenceDoc>
                            incrementIdentDoc = termindexFile2.litNombreULat()
                            frequenceDoc = termindexFile2.litNombreULat()
                            totalFrequences += frequenceDoc
                            noDoc += incrementIdentDoc
                            docList.append('%05d (%d)'%(noDoc, frequenceDoc))
                        outFile.write(' :: %s\n'%(', '.join(docList)))
                        if totalFrequences != frequenceTerme: raise Exception('fréquences incomptibles sur terme %d'%(noTerm))
                        occurencesGenerale += totalFrequences
                        occurencesDoc += nbreDocs 
                        if totalFrequences == 1: nbreHapax +=1
                outFile.write('frequence totale de %06d : %d\n'%(noTerm, frequenceGlobale))
                outFile.write('\n')
                maxFreq = max(maxFreq, frequenceGlobale)
                noTerm +=1
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    outFile.close()
    print "Dump écrit sur %s"%(outFileName)
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print "%d occurrences de couples termes-doc"%(occurencesDoc)
    print "%d occurrences de termes"%(occurencesGenerale)
    print "%d hapax (%0.2f %%)"%(nbreHapax, float(100)*nbreHapax/nbreDefinition)
    print "%d comme fréquence maximum pour un terme"%(maxFreq)
    
    termindexFile.seek(0, 2)
    offsetFin = termindexFile.tell()
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    print "%0.2f octets / occurrence de terme-doc"%(float(offsetFin)/occurencesDoc)
    print "%0.2f octets / occurrence de terme"%(float(offsetFin)/occurencesGenerale)
    termindexFile.close()
    termindexFile2.close()

    
if __name__ == '__main__':
        main()
    
        