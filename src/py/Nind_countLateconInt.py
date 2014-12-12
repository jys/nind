#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """© l'ATÉCON.
Analyse un fichier binaire contenant une suite d'entiers latecon.
Les compte en global et en fonction de leur nombre d'octets.

usage   : %s <fichier>
exemple : %s Nind_testNumbersInLocalIndexes.latU
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    inFileName = os.path.abspath(sys.argv[1])
    
    tailleDico = {}
    nbInt = 0
    inFile = open(inFileName, 'rb')
    octet = inFile.read(1)
    while octet != '':
        premier = ord(octet)
        nbOctets = 1
        if premier&0x80: 
            nbOctets +=1
            if premier&0x40: 
                nbOctets +=1
                if premier&0x20: 
                    nbOctets +=1
                    if premier&0x10: 
                        nbOctets +=1
        if not nbOctets in tailleDico: tailleDico[nbOctets] = 0
        tailleDico[nbOctets] +=1
        nbInt +=1
        inFile.seek(nbOctets-1, 1)
        octet = inFile.read(1)
    inFile.close()
    print '% 8d entiers lus'%(nbInt)
    for i in range(1,6):
        if i in tailleDico:
            print '% 8d entiers à %d octet(s) (%0.2f %%)'%(tailleDico[i], i, float(100)*tailleDico[i]/nbInt)
    
if __name__ == '__main__':
        main()
