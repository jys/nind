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
Programme de test de la classe NindLocalindex.
Cette classe gère le contenu du fichier des index locaux spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche la liste des occurrences de termes 
dans un document spécifié. (attention aux documents longs)

usage   : %s <fichier localindex> <identifiant doc>
exemple : %s box/dumps/boxon/FRE.localindex 9546
"""%(sys.argv[0], sys.argv[0])


def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    localindexFileName = os.path.abspath(sys.argv[1])
    noDoc = int(sys.argv[2])
    
    #la classe
    nindLocalindex = NindLocalindex(localindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure) = nindLocalindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    #trouve les termes dans le fichier des index locaux et les affiche avec leurs localisations
    termList = nindLocalindex.getTermList(noDoc)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        locListe = []
        for (localisationAbsolue, longueur) in localisationsList: locListe.append('%d(%d)'%(localisationAbsolue, longueur))
        resultat.append('%d %s<%s>'%(noTerme, NindLateconFile.catNb2Str(categorie), ' '.join(locListe)))
    print ' '.join(resultat)

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

FLAG_DEFINITION = 17
    
class NindLocalindex:
    def __init__(self, localindexFileName):
        self.localindexFileName = localindexFileName
        #ouvre le fichier en lecture
        self.localindexFile = NindLateconFile.NindLateconFile(self.localindexFileName)

    def getIdentification(self):
        return NindIndex.getIdentification(self.localindexFile, self.localindexFileName)
    
    def getTermList(self, noDoc):
        (offsetDefinition, longueurDefinition) = NindIndex.getDefinitionAddr(self.localindexFile, self.localindexFileName, noDoc)
        if offsetDefinition == 0: return []          #doc inconnu
        #lit l'indirection
        self.localindexFile.seek(offsetDefinition, 0)
        #<flagDefinition> <identifiantDoc> <longueurDonnees>
        if self.localindexFile.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.localindexFileName, offsetDefinition))
        if self.localindexFile.litNombre3() != noDoc: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.localindexFileName, ident, offsetDefinition+1))
        longueurDonnees = self.localindexFile.litNombre3()
        finDonnees = self.localindexFile.tell() + longueurDonnees
        #lit les donnes
        resultat = []
        noTerme = 0
        localisationAbsolue = 0
        while self.localindexFile.tell() < finDonnees:
            #<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
            noTerme += self.localindexFile.litNombreSLat()
            categorie = self.localindexFile.litNombre1()
            nbreLocalisations = self.localindexFile.litNombre1()
            localisationsList = []
            for i in range (nbreLocalisations):
                #<localisationRelatif> <longueur>
                localisationAbsolue += self.localindexFile.litNombreSLat()
                longueur = self.localindexFile.litNombre1()
                localisationsList.append((localisationAbsolue, longueur))
            resultat.append((noTerme, categorie, localisationsList))
        return resultat
              
        
if __name__ == '__main__':
    main()
