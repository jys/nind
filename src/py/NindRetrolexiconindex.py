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
Le lexique inverse permet de trouver un mot à partir de son identifiant.
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le programme de test affiche les mots correspondants aux identifiants 
spécifiés.

usage   : %s <fichier lexiconindex> <ident mot>
exemple : %s FRE.lexiconindex 203547,203548,203549
"""%(script, script)

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    identsStr = sys.argv[2]
    
    idents = identsStr.split(',')
    #la classe
    nindRetrolexiconindex = NindRetrolexiconindex(lexiconindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure, spejcifique) = nindRetrolexiconindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    rejsultat = []
    for ident in idents: rejsultat.append(nindRetrolexiconindex.getWord(int(ident)))
    print ',  '.join(rejsultat)

#// <definition>            ::= <flagDefinition=17> <identifiantMot> <longueurDonnees> <donneesMot>
#// <flagDefinition=17>     ::= <Integer1>
#// <identifiantMot>        ::= <Integer3>
#// <longueurDonnees>       ::= <Integer3>
#// <donneesMot>            ::= <motCompose> | <motSimple>
#// <motCompose>            ::= <flagCompose=31> <identifiantA> <identifiantRelS>
#// <flagCompose=31>        ::= <Integer1>
#// <identifiantA>          ::= <Integer3>
#// <identifiantRelS>       ::= <IntegerSLat>
#// <motSimple>             ::= <flagSimple=37> <longueurMot> <motUtf8>
#// <flagSimple=37>         ::= <Integer1>
#// <longueurMot>           ::= <Integer1>
#// <motUtf8>               ::= { <Octet> }

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
               
    def getWord (self, ident):
        mot = []
        (trouvej, motSimple, identifiantA, identifiantS) = self.getWordDef(ident)
        #print 'A', ident, trouvej, motSimple, identifiantA, identifiantS
        #si pas trouvej, retourne chaisne vide
        if not trouvej: return ''
        while True:
            #si c'est un mot simple, c'est la fin 
            if identifiantA == 0:
                mot.insert(0, motSimple)
                break
            #si c'est un mot composej, recupehre le mot simple du couple
            (trouvej, motSimple, identifiantA2, identifiantS2) = self.getWordDef(identifiantS)
            #print 'B', identifiantS, trouvej, motSimple, identifiantA2, identifiantS2
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantS, self.latFileName))
            if identifiantA2 != 0: raise Exception("%d pas terminal %s"%(identifiantS, self.latFileName))
            mot.insert(0, motSimple)
            #recupere l'autre mot du couple
            (trouvej, motSimple, identifiantA, identifiantS) = self.getWordDef(identifiantA)
            #print 'C', identifiantA, trouvej, motSimple, identifiantA, identifiantS
            if not trouvej: raise Exception("%d pas trouvé dans %s"%(identifiantA, self.latFileName))
            #pour detecter les bouclages induits par un fichier bouclant
            if len(mot) == TAILLE_COMPOSE_MAXIMUM: 
                raise Exception("%d bouclage dans %s"%(ident, self.latFileName))
        #retourne la chaisne
        return '_'.join(mot)
    
    def getWordDef(self, ident):
        #trouve l'adresse des donnees dans le fichier
        (offsetDefinition, longueurDefinition) = self.getDefinitionAddr(ident)
        #si pas trouve, le mot est inconnu
        if offsetDefinition == 0: return (False, "", 0, 0)          #mot inconnu
        #lit l'indirection
        self.seek(offsetDefinition, 0)
        #<flagDefinition=17> <identifiantMot> <longueurDonnees>
        if self.litNombre1() != FLAG_DEFINITION: 
            raise Exception('%s : pas FLAG_DEFINITION à %08X'%(self.latFileName, offsetDefinition))
        if self.litNombre3() != ident: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, ident, offsetDefinition+1))
        longueurDonnees = self.litNombre3()
        finDonnees = self.tell() + longueurDonnees
        #lit les donnees 
        flag = self.litNombre1()
        if flag == FLAG_SIMPLE:
            #<flagSimple=37> <longueurMot> <motUtf8>
            return (True, self.litString(), 0, 0)
        elif flag == FLAG_COMPOSE:
            #<flagCompose=31> <identifiantA> <identifiantRelS>
            return (True, '', self.litNombre3(), ident + self.litNombreSLat())
        else: 
            raise Exception("%d mauvaise définition %s"%(ident, self.latFileName))
        
                       
if __name__ == '__main__':
    main()
