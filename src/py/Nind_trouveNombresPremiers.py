#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
# Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
# This file is part of NIND (as "nouvelle indexation").
# NIND is free software: you can redistribute it and/or modify it under the terms of the 
# GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
# (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
# NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Less General Public License for more details.
import sys
from os import getenv

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
Trouve le nombre premier immédiatement supérier et le nombre premier
immédiatement inférieur à un nombre donné.

usage   : %s <nombre>
exemple : %s 100000
"""%(script, script))

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    nombre = int(sys.argv[1])
    print (premier(nombre, +1))
    print (premier(nombre, -1))
    
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
        