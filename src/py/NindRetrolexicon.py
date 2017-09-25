#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv, path
from time import ctime
from NindLateconFile import NindLateconFile
from NindIndex import NindIndex

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Programme de test de la classe NindRetrolexicon.
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
    nindRetrolexicon = NindRetrolexicon(lexiconindexFileName)
    #affiche l'identification du fichier
    (maxIdentifiant, dateHeure, spejcifique) = nindRetrolexicon.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, ctime(int(dateHeure)))
    rejsultat = []
    for ident in idents: rejsultat.append(nindRetrolexicon.getWord(int(ident)))
    print ',  '.join(rejsultat)

#// <fichier>               ::= { <blocDejfinition> <blocUtf8> } <blocIdentification> 
#//
#// <blocDejfinition>       ::= <flagDejfinition=47> <addrBlocSuivant> <nombreDejfinitions> { <dejfinitionMot> }
#// <flagDejfinition=43>    ::= <Integer1>
#// <addrBlocSuivant>       ::= <Integer5>
#// <nombreDejfinitions>    ::= <Integer3>
#//
#// <dejfinitionMot>        ::= <motComposej> | <motSimple>
#// <motComposej>           ::= <flagComposej=31> <identifiantA> <identifiantS>
#// <flagComposej=31>       ::= <Integer1>
#// <identifiantA>          ::= <Integer3>
#// <identifiantS>          ::= <Integer3>
#// <motSimple>             ::= <flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
#// <flagSimple=37>         ::= <Integer1>
#// <longueurMotUtf8>       ::= <Integer1>
#// <adresseMotUtf8>        ::= <Integer5>
#//
#// <blocUtf8>              ::= { <motUtf8> }
#// <motUtf8>               ::= { <Octet> }
#//
#// <blocIdentification>    ::= <flagIdentification=53> <maxIdentifiant> <identifieurUnique>
#// <flagIdentification=53> ::= <Integer1>
#// <maxIdentifiant>        ::= <Integer3>
#// <identifieurUnique>     ::= <dateHeure>
#// <dateHeure >            ::= <Integer4>


FLAG_COMPOSEJ = 31
FLAG_SIMPLE = 37
TAILLE_COMPOSE_MAXIMUM = 30

class NindRetrolexicon(NindLateconFile):
    def __init__(self, lexiconindexFileName):
        retrolexiconFileName = '.'.join(lexiconindexFileName.split('.')[:-1])+'.retrolexiconindex'
        #l'identification de reference
        lexiconindexFile = NindIndex(lexiconindexFileName)
        lexiconIdent = lexiconindexFile.getIdentification()
        lexiconindexFile.close()
        #si le lexique inverse n'esiste pas, on ne fait rien
        if not path.isfile(retrolexiconFileName): raise Exception("%s n'existe pas"%(retrolexiconFileName))
        #on initialise la classe mehre
        NindLateconFile.__init__(self, retrolexiconFileName, False)
        #on verifie l'appairage
        retrolexiconIdent = self.getIdentification()
        if lexiconIdent != retrolexiconIdent: raise Exception("%s pas à jour"%(retrolexiconFileName))

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
        offsetDejfinition = self.getDejfinitionAddr(ident)
        #si pas trouve, le mot est inconnu
        if offsetDejfinition == 0: return (False, "", 0, 0)          #mot inconnu
        #lit la dejfinition
        self.seek(offsetDejfinition, 0)
        flag = self.litNombre1()
        #<flagSimple=37> <longueurMotUtf8> <adresseMotUtf8>
        if flag == FLAG_SIMPLE:
            longueurMotUtf8 = self.litNombre1()
            adresseMotUtf8 = self.litNombre5()
            #lit la chaisne
            self.seek(adresseMotUtf8, 0)
            return (True, self.litChaine(longueurMotUtf8), 0, 0)
        #<flagComposej=31> <identifiantA> <identifiantS>
        elif flag == FLAG_COMPOSEJ:
            return (True, '', self.litNombre3(), ident + self.litNombreS3())
        else: 
            raise Exception("%d mauvaise définition %s"%(ident, self.latFileName))
        
    def getDejfinitionAddr(self, identifiant):
        FLAG_DEJFINITION = 47
        TETE_DEJFINITION = 9
        TAILLE_DEJFINITION = 7
        self.seek(0, 0)
        maxIdent = 0
        addrBlocDejfinition = 0
        while True:
            #<flagDejfinition=43> <addrBlocSuivant> <nombreDejfinitions>
            if self.litNombre1() != FLAG_DEJFINITION: raise Exception('%s : pas FLAG_DEJFINITION à %08X'%(self.latFileName, addrBlocDejfinition))
            addrBlocSuivant = self.litNombre5()
            nombreDejfinition = self.litNombre3()
            maxIdent += nombreDejfinition
            if maxIdent > identifiant: break
            if addrBlocSuivant == 0: break
            addrBlocDejfinition = addrBlocSuivant
            self.seek(addrBlocDejfinition, 0)
        #si identifiant hors limite, retourne 0
        if maxIdent < identifiant: return 0      
        #sinon retourne l'adresse de la dejfinition 
        index = identifiant + nombreDejfinition - maxIdent
        return addrBlocDejfinition + TETE_DEJFINITION + (index * TAILLE_DEJFINITION)

    def getIdentification(self):
        FLAG_IDENTIFICATION = 53
        TAILLE_IDENTIFICATION = 12
        #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
        self.seek(-TAILLE_IDENTIFICATION, 2)
        if self.litNombre1() != FLAG_IDENTIFICATION: raise Exception('pas FLAG_IDENTIFICATION sur %s'%(self.latFileName))
        maxIdentifiant = self.litNombre3()
        identifieurUnique = self.litNombre4()
        identifieurSpecifique = self.litNombre4()
        return (maxIdentifiant, identifieurUnique, identifieurSpecifique)
                       
if __name__ == '__main__':
    main()
