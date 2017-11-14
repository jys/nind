#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2017 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
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
from os import path
import re
import codecs

def usage():
    script = '$PY/' + path.basename(sys.argv[0])
    print ("""© l'ATEJCON.
Programme de test de la classe NindIdentifiantsClef.
Cette classe permet de convertir un identifiant Clef en identifiant nind
et vice-versa. 
Le programme de test convertit un fichier d'identifiants Clef en le fichier
correspondant d'identifiants Nind ou l'inverse.
Le fichier de sortie est nommé en fonction du fichier d'entrée.

usage   : %s <CLEF | NIND> <langue> <fichierEntrée>
exemple : %s NIND ger ger-identPrpty.txt
exemple : %s CLEF ger ger-identPrpty-NIND.txt
"""%(script, script, script))


def main():
    try:
        if len(sys.argv) < 4 : raise Exception()
        clefNind = sys.argv[1].lower()
        langue = sys.argv[2]
        nomFichierEntreje = path.abspath(sys.argv[3])
        if clefNind not in ('nind', 'clef') : raise Exception('%s : NIND ou CLEF'%(clefNind))
        if langue not in ('fre', 'eng', 'spa', 'ger'): raise Exception('%s : langue inconnue'%(langue))
        traduit(nomFichierEntreje, langue, clefNind)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise

def traduit(nomFichierEntreje, langue, clefNind):
    m = re.match('(.*)(\.[^\.]*)',nomFichierEntreje)
    if not m: raise Exception('%s : mauvais format de nom'%(nomFichierEntreje))
    nomFichierSortie = m.group(1) + '-' + clefNind + m.group(2)
    #ouvre les fichiers 
    fichierEntreje = codecs.open(nomFichierEntreje, 'r', 'utf-8')
    fichierSortie = codecs.open(nomFichierSortie, 'w', 'utf-8')
    #init classe
    nindIdentifiantsClef = NindIdentifiantsClef(langue)
    compt = 0
    for ligne in fichierEntreje:
        if clefNind == 'nind': 
            num = int(nindIdentifiantsClef.tradVersNind(ligne))
            fichierSortie.write('%10d\n'%(num))
        else: fichierSortie.write('%s\n'%(nindIdentifiantsClef.tradVersClef(ligne)))
        compt +=1
    fichierEntreje.close()
    fichierSortie.close()
    print ('%d identifiants générés dans %s'%(compt, path.basename(nomFichierSortie)))
    
class NindIdentifiantsClef:
    def __init__(self, langue):
        self.langue = langue
        
    def tradVersNind(self, entreje):
        if self.langue == 'fre':
            m = re.match('^ATS\.94([0-9]*)\.0([0-9]*)',entreje)         # ATS.94XXXX.0YYY -> XXXXYYY
            if m: return m.group(1) + m.group(2)
            m = re.match('^ATS\.95([0-9]*)\.0([0-9]*)',entreje)         # ATS.95XXXX.0YYY -> 1XXXXYYY
            if m: return '1' + m.group(1) + m.group(2)
            m = re.match('^LEMONDE94-00([0-9]*)-1994([0-9]*)',entreje)  # LEMONDE94-00XXXX-1994YYYY-> 1XXXXYYYY
            if m: return '1' + m.group(1) + m.group(2) 
        if self.langue == 'eng':
            m = re.match('^GH95([0-9]*)-000([0-9]*)',entreje)           # GH95XXXX-000YYY -> XXXXYYY
            if m: return m.group(1) + m.group(2)
            m = re.match('^.LA([0-9]*)94-0([0-9]*)',entreje)            #  LAXXXX94-0YYY -> 1XXXXYYY
            if m: return '1' + m.group(1) + m.group(2)
        if self.langue == 'spa':
            m = re.match('^EFE1994([0-9]*)-([0-9]*)',entreje)           # EFE1994XXXX-YYYYY -> XXXXYYYYY
            if m: return m.group(1) + m.group(2)
            m = re.match('^EFE1995([0-9]*)-([0-9]*)',entreje)           # EFE1995XXXX-YYYYY -> 1XXXXYYYYY
            if m: return '1' + m.group(1) + m.group(2)
        if self.langue == 'ger':
            m = re.match('^FR94([0-9]*)-00([0-9]*)',entreje)            # FR94XXXX-00YYYY -> XXXXYYYY
            if m: return m.group(1) + m.group(2)
            m = re.match('^SDA\.9([0-9]*)\.0([0-9]*)',entreje)          # SDA.9XXXXX.0YYY -> XXXXXYYY
            if m: return m.group(1) + m.group(2)
            m = re.match('^SPIEGEL9495-00([0-9]*)',entreje)             # SPIEGEL9495-00XXXX -> XXXX
            if m: return m.group(1)
        raise Exception('%s : mauvais format'%(entreje))
        
    def tradVersClef(self, entreje):
        num = int(entreje)
        if num > 2000000000 : raise Exception('%s : mauvais format'%(entreje))
        digits = '%010d'%(num)
        if self.langue == 'fre':
            if num < 10000000: return 'ATS.94' + digits[3:7] + '.0' + digits[7:10]   # XXXXYYY -> ATS.94XXXX.0YYY
            if num < 100000000: return 'ATS.95' + digits[3:7] + '.0' + digits[7:10]  # 1XXXXYYY -> ATS.95XXXX.0YYY
            return 'LEMONDE94-00' + digits[2:6] + '-1994' + digits[6:10]             # 1XXXXYYYY -> LEMONDE94-00XXXX-1994YYYY
        if self.langue == 'eng':
            if num < 10000000: return 'GH95' + digits[3:7] + '-000' + digits[7:10]   # XXXXYYY -> GH95XXXX-000YYY
            return ' LA' +  digits[3:7] + '94-0' + digits[7:10] + ' '                # 1XXXXYYY ->  LAXXXX94-0YYY
        if self.langue == 'spa':
            if num < 1000000000: return 'EFE1994' + digits[1:5] + '-' + digits[5:10] # XXXXYYYYY -> EFE1994XXXX-YYYYY
            return 'EFE1995' + digits[1:5] + '-' + digits[5:10]                      # 1XXXXYYYYY -> EFE1995XXXX-YYYYY
        if self.langue == 'ger':
            if num < 10000: return 'SPIEGEL9495-00' + digits[6:10]                   # XXXX -> SPIEGEL9495-00XXXX
            if num < 40000000: return 'FR94' + digits[2:6] + '-00' + digits[6:10]    # XXXXYYYY -> FR94XXXX-00YYYY
            return 'SDA.9' + digits[2:7] + '.0' + digits[7:10]                       # XXXXXYYY -> SDA.9XXXXX.0YYY

       
if __name__ == '__main__':
    main()

    
    