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
Programme de test de la classe NindRetrolexiconindex.
Cette classe gère le lexique inverse du fichier lexique spécifié.
Le lexique inverse permet de trouver un terme à partir de son identifiant.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Si le fichier <nom>.retrolexiconindex existe et est appairé avec le fichier
<nom>.lexiconindex, il est ouvert en lecture.
Sinon, l'erreur est signalée et le fichier <nom>.B.retrolexiconindex est
créé en analysant <nom>.lexiconindex.
Puis le programme de test affiche le terme correspondant à l'identifiant 
spécifié.

usage   : %s <fichier lexiconindex> <ident terme>
exemple : %s box/dumps/boxon/FRE.lexiconindex 203547
"""%(script, script)

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    ident = int(sys.argv[2])
    
    #la classe
    nindRetrolexiconindex = NindRetrolexiconindex(lexiconindexFileName)
    print nindRetrolexiconindex.getTerme(ident)

#<definition>            ::= <flagDefinition=17> <identifiantTerme> <longueurDonnees> <donneesTerme>
#<flagDefinition=17>     ::= <Integer1>
#<identifiantTerme>      ::= <Integer3>
#<longueurDonnees>       ::= <Integer3>
#<donneesTerme>          ::= <termeCompose> | <termeSimple>
#<termeCompose>          ::= <flagCompose=31> <identifiantA> <identifiantRelS>
#<flagCompose=31>        ::= <Integer1>
#<identifiantA>          ::= <Integer3>
#<identifiantRelS>       ::= <IntegerSLat>
#<termeSimple>           ::= <flagSimple=37> <longueurTerme> <termeUtf8>
#<flagSimple=37>         ::= <Integer1>
#<longueurTerme>         ::= <Integer1>
#<termeUtf8>             ::= { <Octet> }

FLAG_DEFINITION = 17
FLAG_COMPOSE = 31
FLAG_SIMPLE = 37
TAILLE_COMPOSE_MAXIMUM = 30

class NindRetrolexiconindex(NindIndex):
    def __init__(self, lexiconindexFileName):
        retrolexiconindexFileName = '.'.join(lexiconindexFileName.split('.')[:-1])+'.retrolexiconindex'
        #l'identification de reference
        lexiconindexFile = NindIndex(lexiconindexFileName)
        lexiconIdent = lexiconindexFile.getIdentification()
        lexiconindexFile.close()
        #si le lexique inverse n'esiste pas, on ne fait rien
        if not path.isfile(retrolexiconindexFileName): raise Exception("%s n'existe pas"%(retrolexiconindexFileName))
        #on initialise la classe mehre
        NindIndex.__init__(self, retrolexiconindexFileName)
        #on verifie l'appairage
        retrolexiconIdent = self.getIdentification()
        if lexiconIdent != retrolexiconIdent: raise Exception("%s pas à jour"%(retrolexiconindexFileName))

    def createFile(self):
        return
               
    def getTerme (self, ident):
        terme = []
        (trouvej, termeSimple, identifiantA, identifiantS) = self.getTermDef(ident)
        #print 'A', ident, trouvej, termeSimple, identifiantA, identifiantS
        #si pas trouvej, retourne chaisne vide
        if not trouvej: return ''
        while True:
            #si c'est un terme simple, c'est la fin 
            if identifiantA == 0:
                terme.insert(0, termeSimple)
                break
            #si c'est un terme composej, recupehre le terme simple du couple
            (trouvej, termeSimple, identifiantA2, identifiantS2) = self.getTermDef(identifiantS)
            #print 'B', identifiantS, trouvej, termeSimple, identifiantA2, identifiantS2
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantS, self.latFileName))
            if identifiantA2 != 0: raise Exception("%d pas terminal %s"%(identifiantS, self.latFileName))
            terme.insert(0, termeSimple)
            #recupere l'autre terme du couple
            (trouvej, termeSimple, identifiantA, identifiantS) = self.getTermDef(identifiantA)
            #print 'C', identifiantA, trouvej, termeSimple, identifiantA, identifiantS
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantA, self.latFileName))
            #pour detecter les bouclages induits par un fichier bouclant
            if len(terme) == TAILLE_COMPOSE_MAXIMUM: 
                raise Exception("%d bouclage dans %s"%(ident, self.latFileName))
        #retourne la chaisne
        return '_'.join(terme)
    
    def getTermDef(self, ident):
        #trouve l'adresse des donnees dans le fichier
        (offsetDefinition, longueurDefinition) = self.getDefinitionAddr(ident)
        #si pas trouve, le terme est inconnu
        if offsetDefinition == 0: return (False, "", 0, 0)          #terme inconnu
        #lit l'indirection
        self.seek(offsetDefinition, 0)
        #<flagDefinition=17> <identifiantTerme> <longueurDonnees>
        if self.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.latFileName, offsetDefinition))
        if self.litNombre3() != ident: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, ident, offsetDefinition+1))
        longueurDonnees = self.litNombre3()
        finDonnees = self.tell() + longueurDonnees
        #lit les donnees 
        flag = self.litNombre1()
        if flag == FLAG_SIMPLE:
            #<flagSimple=37> <longueurTerme> <termeUtf8>
            return (True, self.litString(), 0, 0)
        elif flag == FLAG_COMPOSE:
            #<flagCompose=31> <identifiantA> <identifiantRelS>
            return (True, '', self.litNombre3(), ident + self.litNombreSLat())
        else: 
            raise Exception("%d mauvaise définition %s"%(ident, self.latFileName))
        
                       
if __name__ == '__main__':
    main()
