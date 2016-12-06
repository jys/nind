#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import getenv

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Trouve le nombre premier immédiatement supérier et le nombre premier
immédiatement inférieur à un nombre donné.

usage   : %s <nombre>
exemple : %s 100000
"""%(script, script)

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
        