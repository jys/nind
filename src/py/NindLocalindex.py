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
Programme de test de la classe NindLocalindex.
Cette classe gère le contenu du fichier des index locaux spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche la liste des occurrences de termes 
dans un document spécifié. (attention aux documents longs)

usage   : %s <fichier localindex> <identifiant doc>
exemple : %s box/dumps/boxon/FRE.localindex 9546
"""%(script, script)


def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    localindexFileName = path.abspath(sys.argv[1])
    noDoc = int(sys.argv[2])
    
    #la classe
    nindLocalindex = NindLocalindex(localindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure) = nindLocalindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    #trouve les termes dans le fichier des index locaux et les affiche avec leurs localisations
    (noDocExterne, termList) = nindLocalindex.getTermList(noDoc)
    print '%d -> %d'%(noDoc, noDocExterne)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        locListe = []
        for (localisationAbsolue, longueur) in localisationsList: locListe.append('%d(%d)'%(localisationAbsolue, longueur))
        resultat.append('%d %s<%s>'%(noTerme, NindLateconFile.catNb2Str(categorie), ' '.join(locListe)))
    print ' '.join(resultat)

#<definition>            ::= <flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees> <donneesDoc>
#<flagDefinition=19>     ::= <Integer1>
#<identifiantDoc>        ::= <Integer3>
#<identifiantExterne>    ::= <Integer4>
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

FLAG_DEFINITION = 19
    
class NindLocalindex(NindIndex):
    def __init__(self, localindexFileName):
        NindIndex.__init__(self, localindexFileName)
  
    def getTermList(self, noDoc):
        (offsetDefinition, longueurDefinition) = self.getDefinitionAddr(noDoc)
        if offsetDefinition == 0: return (0, [])          #doc inconnu
        #lit l'indirection
        self.seek(offsetDefinition, 0)
        #<flagDefinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnees>
        if self.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.latFileName, offsetDefinition))
        if self.litNombre3() != noDoc: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, ident, offsetDefinition+1))
        noDocExterne = self.litNombre4()
        longueurDonnees = self.litNombre3()
        finDonnees = self.tell() + longueurDonnees
        #lit les donnes
        resultat = []
        noTerme = 0
        localisationAbsolue = 0
        while self.tell() < finDonnees:
            #<identTermeRelatif> <categorie> <nbreLocalisations> <localisations>
            noTerme += self.litNombreSLat()
            categorie = self.litNombre1()
            nbreLocalisations = self.litNombre1()
            localisationsList = []
            for i in range (nbreLocalisations):
                #<localisationRelatif> <longueur>
                localisationAbsolue += self.litNombreSLat()
                longueur = self.litNombre1()
                localisationsList.append((localisationAbsolue, longueur))
            resultat.append((noTerme, categorie, localisationsList))
        return (noDocExterne, resultat)
              
        
if __name__ == '__main__':
    main()
