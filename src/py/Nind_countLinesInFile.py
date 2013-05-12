#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """
Compte le nombre de lignes et les différentes tailles de
des lignes d'un fichier texte spécifié.

usage   : %s <fichier texte>
exemple : %s  fre-boxon.fdb-DumpByDocuments.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    textFileName = os.path.abspath(sys.argv[1])

    lineCount = 0
    maxSize = 0
    textFile = codecs.open(textFileName, 'r', 'utf-8')
    for line in textFile:
        line = line.strip()
        lineCount +=1
        if maxSize < len(line):
            print '%d : %d'%(lineCount, len(line))
            maxSize = len(line)
            #print line
    textFile.close()
    
    
if __name__ == '__main__':
    main()
    
        

