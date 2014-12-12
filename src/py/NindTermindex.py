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
Programme de test de la classe NindTermindex.
Cette classe gère le contenu du fichier inversé spécifié.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche la liste des documents contenant le
terme spécifié par son identifiant du lexique, ainsi que les
catégories grammaticales et les fréquences.

usage   : %s <fichier termindex> <identifiant terme>
exemple : %s box/dumps/boxon/FRE.termindex 203547
"""%(sys.argv[0], sys.argv[0])

OFF = "\033[m"
RED = "\033[1;31m"

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    termindexFileName = os.path.abspath(sys.argv[1])
    terme = int(sys.argv[2])
    
    #la classe
    nindTermindex = NindTermindex(termindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure) = nindTermindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    #trouve les utilisations du terme dans le fichier inverse et les affiche
    termesCGList = nindTermindex.getTermCGList(terme)
    for (categorie, frequenceTerme, docs) in termesCGList:
        docsListe = []
        for (noDoc, frequenceDoc) in docs: docsListe.append('%d(%d)'%(noDoc, frequenceDoc))
        print '%s[%s] %s%s %d fois dans %s'%(RED, terme, NindLateconFile.catNb2Str(categorie), OFF, frequenceTerme, ' '.join(docsListe))

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
FLAG_CG = 61

class NindTermindex:
    def __init__(self, termindexFileName):
        self.termindexFileName = termindexFileName
        #ouvre le fichier en lecture
        self.termindexFile = NindLateconFile.NindLateconFile(self.termindexFileName)

    def getIdentification(self):
        return NindIndex.getIdentification(self.termindexFile, self.termindexFileName)
    
    def getTermCGList(self, ident):
        (offsetDefinition, longueurDefinition) = NindIndex.getDefinitionAddr(self.termindexFile, self.termindexFileName, ident)
        if offsetDefinition == 0: return []          #terme inconnu
        #lit l'indirection
        self.termindexFile.seek(offsetDefinition, 0)
        #<flagDefinition> <identifiantTerme> <longueurDonnees>
        if self.termindexFile.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.termindexFileName, offsetDefinition))
        if self.termindexFile.litNombre3() != ident: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.termindexFileName, ident, offsetDefinition+1))
        longueurDonnees = self.termindexFile.litNombre3()
        finDonnees = self.termindexFile.tell() + longueurDonnees
        #lit les donnes
        resultat = []
        while self.termindexFile.tell() < finDonnees:
            #<flagCg> <categorie> <frequenceTerme> <nbreDocs> <listeDocuments>
            if self.termindexFile.litNombre1() != FLAG_CG: raise Exception('pas FLAG_CG à %d'%(self.termindexFile.tell() -1))
            categorie = self.termindexFile.litNombre1()
            frequenceTerme = self.termindexFile.litNombreULat()
            nbreDocs = self.termindexFile.litNombreULat()
            noDoc = 0
            docs = []
            for i in range(nbreDocs):
                #<identDocRelatif> <frequenceDoc>
                identDocRelatif = self.termindexFile.litNombreULat()
                frequenceDoc = self.termindexFile.litNombreULat()
                noDoc += identDocRelatif
                docs.append((noDoc, frequenceDoc))
            resultat.append((categorie, frequenceTerme, docs))
        return resultat
    
            
if __name__ == '__main__':
    main()
