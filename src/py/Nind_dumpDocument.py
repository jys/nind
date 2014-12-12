#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import datetime
import time
import NindLexiconindexInverse
import NindLocalindex
import NindLateconFile

def usage():
    print """© l'ATÉCON.
Dumpe en clair un document préalablement indexé dans une base nind.
Les termes apparaissent dans l'ordre, les localisations ne sont pas affichées.
Le système nind est expliqué dans le document LAT2014.JYS.440.

usage   : %s <fichier lexiconindex> <n° doc>
exemple : %s box/dumps/boxon/FRE.lexiconindex 9546
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = os.path.abspath(sys.argv[1])
    noDoc = int(sys.argv[2])
    
    #calcul des noms de fichiers (remplace l'extension)
    nn = lexiconindexFileName.split('.')
    localindexFileName = '.'.join(nn[:-1])+'.localindex'
    
    #ouvre les classes
    lexiconindexInverseFile = NindLexiconindexInverse.NindLexiconindexInverse(lexiconindexFileName)
    nindLocalindex = NindLocalindex.NindLocalindex(localindexFileName)

    #trouve les termes dans le fichier des index locaux et les affiche sans leurs localisations
    termList = nindLocalindex.getTermList(noDoc)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        terme = lexiconindexInverseFile.getTerme(noTerme)
        resultat.append('%s [%s]'%(terme, NindLateconFile.catNb2Str(categorie)))
    print ', '.join(resultat)
   

if __name__ == '__main__':
        main()
