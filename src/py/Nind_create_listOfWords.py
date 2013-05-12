#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import StringIO
import Lexicon

def usage():
    print """
À partir d'une base de données Firebird au format LIC2M, 
extrait des blobs de la table LEXICON et forme la liste
de mots du lexique dans un fichier texte.
L'ordre des mots est aléatoire (celle du dico python)
Le fichier résultat est <nom de la base>-Nind-ListeDeMots.csv 
dans le répertoire courant.

usage   : %s <path base Firebird>
exemple : %s /mnt/extension/etudeIndex/index/fre-boxon.fdb 
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dbName = os.path.abspath(sys.argv[1])
    #nom du fichier de sortie
    outFileName = '%s-Nind-ListeDeMots.csv'%(os.path.basename(dbName))

    #etablit le lexique 
    lexicon = Lexicon.Lexicon(dbName, True)
    #sort la liste du lexique
    lexiconWords = lexicon.getLexicon()
    print lexiconWords.values()[:1000]
    
    
if __name__ == '__main__':
    main()
    