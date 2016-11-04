#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """© l'ATÉCON.
Analyse un fichier inversé et l'écrit en clair sur un fichier texte. 
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier termindex>-dump.txt
Donne des informations sur la composition du fichier 
et donne quelques statistiques

usage   : %s <fichier termindex>
exemple : %s box/dumps/boxon/FRE.termindex
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    termindexFileName = os.path.abspath(sys.argv[1])
    outFileName = '%s-dump.txt'%(termindexFileName)
