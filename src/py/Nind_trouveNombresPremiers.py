#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os

def usage():
    print """© l'ATÉCON.
Trouve le nombre premier immédiatement supérier et le nombre premier
immédiatement inférieur à un nombre donné.

usage   : %s <nombre>
exemple : %s 100000
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    nombre = int(sys.argv[1])
    print premier(nombre, +1)
    print premier(nombre, -1)
    
def premier(nombre, increment):
    offset = 0
    while True:
        candidat = nombre + offset
        diviseur = 2
        while True:
            if candidat < diviseur * diviseur: return candidat
            if candidat % diviseur == 0: break 
            diviseur += 1
        offset += increment

if __name__ == '__main__':
        main()
        