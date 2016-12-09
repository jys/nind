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
Programme de test de la classe NindTermindex.
Cette classe gère le contenu du fichier inversé spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche la liste des documents contenant le
terme spécifié par son identifiant du lexique, ainsi que les
catégories grammaticales et les fréquences.

usage   : %s <fichier termindex> <identifiant terme>
exemple : %s FRE.termindex 203547
"""%(script, script)

OFF = "\033[m"
RED = "\033[1;31m"

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    termindexFileName = path.abspath(sys.argv[1])
    terme = int(sys.argv[2])
    
    #la classe
    nindTermindex = NindTermindex(termindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure, spejcifique) = nindTermindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    #trouve les utilisations du terme dans le fichier inverse et les affiche
    termesCGList = nindTermindex.getTermCGList(terme)
    for (categorie, frequenceTerme, docs) in termesCGList:
        docsListe = []
        for (noDoc, frequenceDoc) in docs: docsListe.append('%d(%d)'%(noDoc, frequenceDoc))
        print '%s[%s] %s%s %d fois dans %s'%(RED, terme, NindLateconFile.catNb2Str(categorie), OFF, frequenceTerme, ' '.join(docsListe))

#<definition>            ::= <flagDefinition=17> <identifiantTerme> <longueurDonnees> <donneesTerme>
#<flagDefinition=17>     ::= <Integer1>
#<identifiantTerme>      ::= <Integer3>
#<longueurDonnees>       ::= <Integer3>
#<donneesTerme>          ::= { <donneesCG> }
#<donneesCG>             ::= <flagCg=61> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
#<flagCg=61>             ::= <Integer1>
#<categorie>             ::= <Integer1>
#<frequenceTerme>        ::= <IntegerULat>
#<nbreDocs>              ::= <IntegerULat>
#<listeDocuments>        ::= { <identDocRelatif> <frequenceDoc> }
#<identDocRelatif>       ::= <IntegerULat>
#<frequenceDoc>          ::= <IntegerULat>

FLAG_DEFINITION = 17
FLAG_CG = 61

class NindTermindex(NindIndex):
    def __init__(self, termindexFileName):
        NindIndex.__init__(self, termindexFileName)

    def getTermCGList(self, ident):
        (offsetDefinition, longueurDefinition) = self.getDefinitionAddr(ident)
        if offsetDefinition == 0: return []          #terme inconnu
        #lit l'indirection
        self.seek(offsetDefinition, 0)
        #<flagDefinition=17> <identifiantTerme> <longueurDonnees>
        if self.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.latFileName, offsetDefinition))
        if self.litNombre3() != ident: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, ident, offsetDefinition+1))
        longueurDonnees = self.litNombre3()
        finDonnees = self.tell() + longueurDonnees
        #lit les donnes
        resultat = []
        while self.tell() < finDonnees:
            #<flagCg=61> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
            if self.litNombre1() != FLAG_CG: raise Exception('pas FLAG_CG à %d'%(self.tell() -1))
            categorie = self.litNombre1()
            frequenceTerme = self.litNombreULat()
            nbreDocs = self.litNombreULat()
            noDoc = 0
            docs = []
            for i in range(nbreDocs):
                #<identDocRelatif> <frequenceDoc>
                identDocRelatif = self.litNombreULat()
                frequenceDoc = self.litNombreULat()
                noDoc += identDocRelatif
                docs.append((noDoc, frequenceDoc))
            resultat.append((categorie, frequenceTerme, docs))
        return resultat
    
            
if __name__ == '__main__':
    main()
