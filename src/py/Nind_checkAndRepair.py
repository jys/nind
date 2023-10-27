#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2023 LATEJCON"
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
from os import getenv, path
from time import ctime, localtime, strftime
from builtins import input
from shutil import copy2
import NindFile
from NindLexiconindex import NindLexiconindex
from NindRetrolexicon import NindRetrolexicon
from NindTermindex import NindTermindex
from NindLocalindex import NindLocalindex
from NindPadFile import NindPadFile

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print (f"""© l'ATEJCON.
Programme de test et de réparation d'une base nind préalablement indexée et
spécifiée par un de ses fichiers.
En cas de réparation les fichiers modifiés sont sauvegardés en ajoutant la
date et l'heure derrière l'extension.

usage   : {script} <fichier index> 
usage   : {script} NindFre.nindlexiconindex
""")
    
def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        nindFileName = path.abspath(sys.argv[1])
        checkAndRepair(nindFileName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc)
            print ("******************************")
            raise
        sys.exit()

def checkAndRepair(nindFileName):
    #calcul des noms de fichiers (remplace l'extension)
    nn = nindFileName.split('.')
    nindlexiconindexFullname = '.'.join(nn[:-1])+'.nindlexiconindex'
    nindretrolexiconFullname = '.'.join(nn[:-1])+'.nindretrolexicon'
    nindtermindexFullname = '.'.join(nn[:-1])+'.nindtermindex'
    nindlocalindexFullname = '.'.join(nn[:-1])+'.nindlocalindex'
    nindlexiconindexName = path.basename(nindlexiconindexFullname)
    nindretrolexiconName = path.basename(nindretrolexiconFullname)
    nindtermindexName = path.basename(nindtermindexFullname)
    nindlocalindexName = path.basename(nindlocalindexFullname)
    
    ######################## Diagnostic
    print('DIAGNOSTIC\n')
    # ejtat des lieux
    lexiqueOk = rejtroOk = termOk = localOk = False
    rejtroAjour = termAjour = localAjour = False
    
    # 1) le lexique
    maxLexicon = dateHeureLexicon = 0
    print(f'Ouvre {nindlexiconindexName}')
    try:
        nindLexiconindex = NindLexiconindex(nindlexiconindexFullname)
        (maxLexicon, dateHeureLexicon) = nindLexiconindex.donneIdentificationFichier()
        nindLexiconindex.close()
        lexiqueOk = True
        print(f'OK    {nindlexiconindexName}')
        print (f'max={maxLexicon} dateheure={dateHeureLexicon} ({ctime(int(dateHeureLexicon))})')
    except Exception as exc:
        print(f'NOK!! {nindlexiconindexName}')
        print (exc)
    print()
    
    # 2) le rejtrolexique
    maxRejtro = dateHeureRejtro = 0
    print(f'Ouvre {nindretrolexiconName}')
    try:
        nindretrolexicon = NindRetrolexicon(nindretrolexiconFullname)
        (maxRejtro, dateHeureRejtro) = nindretrolexicon.donneIdentificationFichier()
        nindretrolexicon.close()
        rejtroOk = True
        print(f'OK    {nindretrolexiconName}')
        rejtroAjour = maxRejtro == maxLexicon and dateHeureRejtro == dateHeureLexicon
        if not rejtroAjour:
            print('!!!!  PROBLÈME DE COHÉRENCE AVEC LE LEXIQUE')
            print (f'!!!!  max={maxRejtro} dateheure={dateHeureRejtro} ({ctime(int(dateHeureRejtro))})')
    except Exception as exc:
        print(f'NOK!! {nindretrolexiconName}')
        print (exc)
    print()
    
    # 3) le fichier des termes
    maxTerm = dateHeureTerm = 0
    print(f'Ouvre {nindtermindexName}')
    try:
        nindtermindex = NindTermindex(nindtermindexFullname)
        (maxTerm, dateHeureTerm) = nindtermindex.donneIdentificationFichier()
        nindtermindex.close()
        termOk = True
        print(f'OK    {nindtermindexName}')
        termAjour = maxTerm == maxLexicon and dateHeureTerm == dateHeureLexicon
        if not termAjour:
            print('!!!!  PROBLÈME DE COHÉRENCE AVEC LE LEXIQUE')
            print (f'!!!!  max={maxTerm} dateheure={dateHeureTerm} ({ctime(int(dateHeureTerm))})')
    except Exception as exc:
        print(f'NOK!! {nindtermindexName}')
        print (exc)
    print()

    # 4) le fichier des index locaux
    maxLocal = dateHeureLocal = 0
    print(f'Ouvre {nindlocalindexName}')
    try:
        nindlocalindex = NindTermindex(nindlocalindexFullname)
        (maxLocal, dateHeureLocal) = nindlocalindex.donneIdentificationFichier()
        nindlocalindex.close()
        localOk = True
        print(f'OK    {nindlocalindexName}')
        localAjour = maxLocal == maxLexicon and dateHeureLocal == dateHeureLexicon
        if not localAjour:
            print('!!!!  PROBLÈME DE COHÉRENCE AVEC LE LEXIQUE')
            print (f'!!!!  max={maxLocal} dateheure={dateHeureLocal} ({ctime(int(dateHeureLocal))})')
    except Exception as exc:
        print(f'NOK!! {nindlocalindexName}')
        print (exc)
    print()

    # si tout est ok, terminej
    if lexiqueOk and rejtroOk and termOk and localOk and rejtroAjour and termAjour and localAjour: 
        print('TOUT EST OK')
        return

    ######################## Rejparation
    print('RÉPARATION\n')
    # pour le moment, on ne rejpare que si les fichiers sont cohejrents en interne
    if not (lexiqueOk and rejtroOk and termOk and localOk):
        print('AU MOINS UN FICHIER NOK!!, RÉPARATION IMPOSSIBLE POUR LE MOMENT')
        return
    
    # le lexique et le rejtrolexique doivent estre cohejrents
    if not rejtroAjour:
        print(f'{nindretrolexiconName} PAS ALIGNÉ, RÉPARATION IMPOSSIBLE POUR LE MOMENT')
        return
    
    # le fichier des termes 
    if not termAjour:
        # il doit estre antejrieur au lexique
        if maxTerm > maxLexicon or dateHeureTerm > dateHeureLexicon:
            print(f'{nindtermindexName} POSTÉRIEUR AU LEXIQUE, RÉPARATION IMPOSSIBLE POUR LE MOMENT')
        elif okToRepair(nindtermindexName):
            # sauvegarde du fichier ah modifier
            saveFile(nindtermindexFullname)
            # ejcrit l'identification du lexique
            writeIdentification(nindtermindexFullname, maxLexicon, dateHeureLexicon)

    # le fichier des index locaux 
    if not localAjour:
        # il doit estre antejrieur au lexique
        if maxLocal > maxLexicon or dateHeureLocal > dateHeureLexicon:
            print(f'{nindlocalindexName} POSTÉRIEUR AU LEXIQUE, RÉPARATION IMPOSSIBLE POUR LE MOMENT')
        elif okToRepair(nindlocalindexName):
            # sauvegarde du fichier ah modifier
            saveFile(nindlocalindexFullname)
            # ejcrit l'identification du lexique
            writeIdentification(nindlocalindexFullname, maxLexicon, dateHeureLexicon)
 
# demande accord pour modifier
def okToRepair(fileName):
    accord = input(f'Voulez-vous réparer {fileName} ? (o/O)')
    return accord in ['o', 'O']

# sauvegarde fichier ah modifier
def saveFile(fileName):
    # sauvegarde du fichier ah modifier
    saveFileName = fileName + strftime("%Y%m%d-%H%M%S", localtime(path.getmtime(fileName)))
    print(f'Sauve {fileName} sur {saveFileName}')
    copy2(fileName, saveFileName)

# ejcrit la nouvelle identification du fichier
def writeIdentification(fileName, maxLexicon, dateHeureLexicon):
    nindPadFile = NindPadFile(fileName, True, True)
    nindPadFile.changeIdentificationFichier(maxLexicon, dateHeureLexicon)
    nindPadFile.close()
 
if __name__ == '__main__':
        main()
  
