#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import NindLateconFile

def usage():
    print """© l'ATÉCON.
Analyse un fichier d'index locaux et l'écrit en clair sur un fichier texte. 
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier localindex>-dump.txt
Donne des informations sur la composition du fichier 
et donne quelques statistiques

usage   : %s <fichier termindex>
exemple : %s box/dumps/boxon/FRE.localindex
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
    #DEF_CG_HEAD = 8
    #DOCUMENT_SIZE = 5
  
    termindexFile = NindLateconFile.NindLateconFile(termindexFileName)
    termindexFile2 = NindLateconFile.NindLateconFile(termindexFileName)
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        nbreDefinition = nbreExtension = 0
        tailleDefinition = tailleExtension = 0 
        occurencesGenerale = nbreDoc = 0
        localisations = [0, 0, 0, 0]
        noDoc = 0
        termindexFile.seek(0, 0)
        while True:
            addrIndirection = termindexFile.tell()
            #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
            if termindexFile.litNombre1() != INDIRECTION_FLAG: raise Exception('pas INDIRECTION_FLAG à %08X'%(addrIndirection))
            indirectionSuivante = termindexFile.litNombre5()
            nombreIndirection = termindexFile.litNombre3()
            for i in range(nombreIndirection):
                outFile.write('%07d:: '%(noDoc))
                #<offsetDefinition> <longueurDefinition> 
                offsetEntree = termindexFile.litNombre5()
                longueurEntree = termindexFile.litNombre3()
                if offsetEntree != 0: 
                    nbreDoc +=1
                    termindexFile2.seek(offsetEntree, 0)
                    #<flagDefinition> <identifiantDoc> <longueurDonnees>
                    if termindexFile2.litNombre1() != DEFINITION_FLAG: raise Exception('pas DEFINITION_FLAG à %08X'%(offsetEntree))
                    if termindexFile2.litNombre3() != noDoc: raise Exception('%d pas trouvé à %08X'%(noDoc, offsetEntree+1))
                    longueurDonnees = termindexFile2.litNombre3()
                    nbreDefinition +=1
                    tailleDefinition += longueurDonnees + DEFINITION_HEAD
                    extension = longueurEntree - longueurDonnees - DEFINITION_HEAD
                    #print "offsetEntree=%d longueurEntree=%d longueurDonnees=%d"%(offsetEntree, longueurEntree, longueurDonnees)
                    if extension > 0: 
                        nbreExtension +=1
                        tailleExtension += extension
                    elif extension < 0: raise Exception('%d incohérent à %08X'%(noDoc, offsetEntree+5))
                    #examine les données
                    finDonnees = offsetEntree + longueurDonnees + DEFINITION_HEAD
                    noTerme = 0;
                    localisationAbsolue = 0
                    while termindexFile2.tell() < finDonnees:
                        #<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
                        occurencesGenerale +=1
                        noTerme += termindexFile2.litNombreSLat()
                        categorie = termindexFile2.litNombre1()
                        nbreLocalisations = termindexFile2.litNombre1()
                        outFile.write('[%d](%d)'%(noTerme, categorie))
                        localisations[nbreLocalisations - 1] +=1
                        localisationsList = []
                        for i in range (nbreLocalisations):
                            #<localisationRelatif> <longueur>
                            localisationAbsolue += termindexFile2.litNombreSLat()
                            longueur = termindexFile2.litNombre1()
                            localisationsList.append('%d(%d)'%(localisationAbsolue, longueur))
                        outFile.write('<%s> '%(','.join(localisationsList)))
                outFile.write('\n')
                noDoc +=1
            if indirectionSuivante == 0: break
            termindexFile.seek(indirectionSuivante, 0)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
    outFile.close()
    print "Dump écrit sur %s"%(outFileName)
    print "%d définitions de taille totale %d octets"%(nbreDefinition, tailleDefinition)
    print "%d extensions de taille totale %d octets"%(nbreExtension, tailleExtension)
    print "%d documents"%(nbreDoc)
    print "%d occurrences de termes"%(occurencesGenerale)
    for i in range(4):
        print '%d localisations à %d élément(s)'%(localisations[i], i+1)    
    termindexFile.seek(0, 2)
    offsetFin = termindexFile.tell()
    print "taille fichier % 10d %08X"%(offsetFin, offsetFin)
    print
    print "%0.2f octets / occurrence de terme"%(float(offsetFin)/occurencesGenerale)
    termindexFile.close()
    termindexFile2.close()

    
if __name__ == '__main__':
        main()
    
        