#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import datetime
import time
import NindLateconFile
import NindLexiconindex
import NindTermindex
import NindLocalindex

def usage():
    print """© l'ATÉCON.
Programme d'interrogation d'une base nind préalablement indexée
L'utilisateur indique le terme cherché, simple ou composé.
Un terme composé a la forme dièsée (ex kyrielle#outil#informatique).
L'utilisateur indique ensuite le document examiné
et la localisation du terme cherché est affichée.
Le format des fichiers est défini dans le document LAT2014.JYS.440.

usage   : %s <fichier lexiconindex> <terme cherché>
exemple : %s box/dumps/boxon/FRE.lexiconindex syntagme#nominal
"""%(sys.argv[0], sys.argv[0])

OFF = "\033[m"
BLUE = "\033[0;34m"
RED = "\033[1;31m"


def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = os.path.abspath(sys.argv[1])
    terme = sys.argv[2].decode('utf-8').strip()
    
    #calcul des noms de fichiers (remplace l'extension)
    nn = lexiconindexFileName.split('.')
    termindexFileName = '.'.join(nn[:-1])+'.termindex'
    localindexFileName = '.'.join(nn[:-1])+'.localindex'
    
    #ouvre les classes
    nindLexiconindex = NindLexiconindex.NindLexiconindex(lexiconindexFileName)
    nindTermindex = NindTermindex.NindTermindex(termindexFileName)
    nindLocalindex = NindLocalindex.NindLocalindex(localindexFileName)
    
    #1) verifie l'identification des fichiers
    (maxIdentifiant, dateHeure) = nindLexiconindex.getIdentification()
    print "max=%d dateheure=%d (%s)"%(maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    (maxIdentifiant2, dateHeure2) = nindTermindex.getIdentification()
    if maxIdentifiant2 != maxIdentifiant or dateHeure2 != dateHeure:
        print "%s NON APAIRÉ : max=%d dateheure=%d (%s)"%(termindexFileName, maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))
    (maxIdentifiant2, dateHeure2) = nindLocalindex.getIdentification()
    if maxIdentifiant2 != maxIdentifiant or dateHeure2 != dateHeure:
        print "%s NON APAIRÉ : max=%d dateheure=%d (%s)"%(localindexFileName, maxIdentifiant, dateHeure, time.ctime(int(dateHeure)))   
    
    #2) trouve l'identifiant du terme
    #trouve la clef des termes simples
    termesSimples = terme.split('#')
    sousMotId = 0
    for mot in termesSimples:
        sousMotId = nindLexiconindex.getIdent(mot, sousMotId)
        if sousMotId == 0: break
    if sousMotId == 0:
        print 'INCONNU'
        sys.exit()
        
    #3) trouve les utilisations du terme dans le fichier inverse et les affiche
    termesCGList = nindTermindex.getTermCGList(sousMotId)
    for (categorie, frequenceTerme, docs) in termesCGList:
        docsListe = []
        for (noDoc, frequenceDoc) in docs: docsListe.append('%d(%d)'%(noDoc, frequenceDoc))
        print '%s[%s] %s%s %d fois dans %s'%(RED, sousMotId, NindLateconFile.catNb2Str(categorie), OFF, frequenceTerme, ' '.join(docsListe))

    #4) choisit un doc et affiche ce qui concerne le terme
    noDocStr = raw_input("%sno doc : %s"%(BLUE, OFF))
    if not noDocStr.isdigit(): sys.exit()
    noDoc = int(noDocStr)
    termList = nindLocalindex.getTermList(noDoc)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        if noTerme != sousMotId: continue
        locListe = []
        for (localisationAbsolue, longueur) in localisationsList: locListe.append('%d(%d)'%(localisationAbsolue, longueur))
        resultat.append('%s<%s>'%(NindLateconFile.catNb2Str(categorie), ' '.join(locListe)))
    print ' '.join(resultat)
                                                                                  

if __name__ == '__main__':
        main()
