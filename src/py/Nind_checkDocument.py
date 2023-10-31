#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2023 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
# Copyright: 2014-2023 LATEJCON. See LICENCE.md file that comes with this distribution
# This file is part of NIND (as "nouvelle indexation").
# NIND is free software: you can redistribute it and/or modify it under the terms of the 
# GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
# (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
# NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Less General Public License for more details.
import sys
from os import getenv, path
from NindLexiconindex import NindLexiconindex
from NindRetrolexicon import NindRetrolexicon
from NindTermindex import NindTermindex
from NindLocalindex import NindLocalindex

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print (f"""© l'ATEJCON.
Dans une base nind spécifiée par un de ses fichiers, vérifie l'indexation 
d'un document spécifié par son identifiant externe (ou "last" pour le 
dernier document indexé).
Chaque terme du document est recréé en clair par le rétrolexique puis cherché
via le lexique et le fichier de termes. Ce test valide la cohérence des 4 
fichiers nind sur le document spécifié.
Pour visualiser les termes indexés de ce document, utilisez le script
Nind_dumpDocument.py
Le système nind est expliqué dans le document LAT2017.JYS.470.

usage   : {script} <fichier nind> <n° doc>
exemple : {script} FRE.lexiconindex 9546
exemple : {script} FRE.lexiconindex last
""")

def main():
    try:
        if len(sys.argv) < 3 : raise Exception()
        nindFileName = path.abspath(sys.argv[1])
        noDoc = sys.argv[2]
        checkDocument(nindFileName, noDoc)
        
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()

################################
def checkDocument(nindFileName, noDoc):
    #calcul des noms de fichiers (remplace l'extension)
    nn = nindFileName.split('.')
    nindlexiconindexName = '.'.join(nn[:-1])+'.nindlexiconindex'
    nindretrolexiconName = '.'.join(nn[:-1]) + '.nindretrolexicon'
    nindtermindexName = '.'.join(nn[:-1]) +'.nindtermindex'
    nindlocalindexName = '.'.join(nn[:-1]) + '.nindlocalindex'
    #les classes
    nindLexiconindex = NindLexiconindex(nindlexiconindexName)
    nindRetrolexicon = NindRetrolexicon(nindretrolexiconName)
    nindTermindex = NindTermindex(nindtermindexName)
    nindLocalindex = NindLocalindex(nindlocalindexName)
    #trouve l'identifiant interne
    if noDoc.startswith('las'):
        (noInterne, noExterne) = nindLocalindex.donneMaxIdentifiants()
    else:
        noExterne = int(noDoc)
    if noExterne not in nindLocalindex.docIdTradExtInt: 
        print ("doc inconnu")
        sys.exit()
    identInterne = nindLocalindex.docIdTradExtInt[noExterne]
    print (f'noExterne -> identInterne : {noExterne} -> {identInterne}')
    #trouve les termes dans le fichier des index locaux
    termList = nindLocalindex.donneListeTermes(noExterne)
    cmptTermes = 0
    for (noTerme, catejgorie1, localisationsList) in termList:
        cmptTermes +=1
        # trouve le mot en clair par le rejtrolexique
        motsSimples = nindRetrolexicon.donneMot(noTerme)
        terme = '#'.join(motsSimples)
        # et retrouve l'identifiant par le lexique
        motId = nindLexiconindex.donneIdentifiant(motsSimples)
        if motId != noTerme:
            raise Exception(f'INCOHÉRENCE LEXIQUE-RÉTRO LEXIQUE #{terme}# {noTerme} <> {motId}')
        # trouve les utilisations du terme dans le fichier inverse
        termesCGList = nindTermindex.donneListeTermesCG(noTerme)
        trouvej = False
        for (catejgorie2, frequenceTerme, docs) in termesCGList:
            if catejgorie1 != catejgorie2: continue
            for (noDoc, frequenceDoc) in docs: 
                trouvej = noDoc == noExterne
                if trouvej: break
            if trouvej: break
        if not trouvej:
            raise Exception(f'INCOHÉRENCE TERMES-LOCAL #{terme}# {noTerme}')
    # terminej
    nindLexiconindex.close()
    nindRetrolexicon.close()
    nindTermindex.close()
    nindLocalindex.close()
    print(f'{cmptTermes} termes du document n°{noExterne} correctement retrouvés par le système nind')
    
if __name__ == '__main__':
        main()
    
