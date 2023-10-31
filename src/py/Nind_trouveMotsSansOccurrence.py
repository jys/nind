#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2019 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2019
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
from codecs import open
from NindLexiconindex import NindLexiconindex
from NindTermindex import NindTermindex
from NindRetrolexicon import NindRetrolexicon

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print (f"""© l'ATEJCON.
Dans une base nind spécifiée par un de ses fichiers, trouve les mots du 
lexique qui n'ont aucune occurrence dans le corpus indexé et ne sont donc
que des composants de mots composés ou une rémanence de documents tail effacés.
Le résultat est sauvegardé dans le fichier <base>-dump-motsSansOccurrence.txt.

usage   : {script} <fichier>
exemple : {script} FRE.nindlexiconindex
""")

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        nindFileName = path.abspath(sys.argv[1])
        
        #calcul des noms de fichiers (remplace l'extension)
        nn = nindFileName.split('.')
        nindlexiconindexName = '.'.join(nn[:-1])+'.nindlexiconindex'
        nindtermindexName = '.'.join(nn[:-1]) + '.nindtermindex'
        nindretrolexiconName = '.'.join(nn[:-1]) + '.nindretrolexicon'
        motsSansOccurrenceName = '.'.join(nn[:-1]) + '-motsSansOccurrence.txt'
        
        #les classes
        nindLexiconindex = NindLexiconindex(nindlexiconindexName)
        nindTermindex = NindTermindex(nindtermindexName)
        nindRetrolexicon = NindRetrolexicon(nindretrolexiconName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()
        
    mots = []
    #trouve le max des identifiants possibles
    (maxLexicon, dateHeureLexicon) = nindLexiconindex.donneIdentificationFichier()
    print(f'{maxLexicon} mots testés')
    for ident in range(1, maxLexicon+1):
        if (ident%10000) == 0:
            sys.stdout.write('%d\r'%(ident))
            sys.stdout.flush()        
        #vejrifie que c'est bien un identifiant de mot 
        motsSimples = nindRetrolexicon.donneMot(ident)
        if len(motsSimples) == 0: continue
        #lit la dejfinition du mot
        (offsetDejfinition, longueurDejfinition) = nindTermindex.donneAdresseDejfinition(ident)
        #si dejfinition existe, raf
        if offsetDejfinition != 0: continue
        #mejmorise le mot
        mots.append('{:06d} {:s}'.format(ident, '#'.join(motsSimples)))
    #affiche les résultats
    print(f'{len(mots)} mots du lexique sans occurrence dans le corpus')
    print('-> ', motsSansOccurrenceName)
    with open(motsSansOccurrenceName, 'w', 'utf8') as dump:
        dump.write('\n'.join(mots) + '\n')
            
if __name__ == '__main__':
        main()
        
