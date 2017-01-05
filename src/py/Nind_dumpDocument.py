#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv, path
import NindLateconFile
from NindRetrolexicon import NindRetrolexicon
from NindLocalindex import NindLocalindex
import NindLateconFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Dumpe en clair un document préalablement indexé dans une base nind.
Les termes apparaissent dans l'ordre, les localisations ne sont pas affichées.
Le système nind est expliqué dans le document LAT2014.JYS.440.

usage   : %s <fichier lexiconindex> <n° doc>
exemple : %s FRE.lexiconindex 9546
"""%(script, script)

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    lexiconindexFileName = path.abspath(sys.argv[1])
    noDoc = int(sys.argv[2])
    
    #calcul des noms de fichiers (remplace l'extension)
    nn = lexiconindexFileName.split('.')
    localindexFileName = '.'.join(nn[:-1])+'.localindex'
    
    #ouvre les classes
    nindRetrolexiconindex = NindRetrolexicon(lexiconindexFileName)
    nindLocalindex = NindLocalindex(localindexFileName)

    #trouve l'identifiant interne
    identInterne = nindLocalindex.docIdTradExtInt[noDoc]
    print '%d -> %d'%(noDoc, identInterne)
    #trouve les termes dans le fichier des index locaux et les affiche sans leurs localisations
    termList = nindLocalindex.getTermList(noDoc)
    resultat = []
    for (noTerme, categorie, localisationsList) in termList:
        terme = nindRetrolexiconindex.getWord(noTerme)
        resultat.append('%s [%s]'%(terme, NindLateconFile.catNb2Str(categorie)))
    print ', '.join(resultat)
   

if __name__ == '__main__':
        main()
