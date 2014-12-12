#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import re

def usage():
    print """© l'ATÉCON.
Analyse un fichier de dump par documents d'une base S2 et affiche
la liste des termes qui ont le nombre de composants spécifiés et
qui commence par la lettre spécifiée.

usage   : %s <fichier dump> <nbre composants> <lettre tête>
exemple : %s boxon/FRE.FDB-DumpByDocuments.txt 4 B
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 4 :
        usage()
        sys.exit()
    dumpFileName = os.path.abspath(sys.argv[1])
    nbComposants = int(sys.argv[2])
    premLettres = sys.argv[3]

    resultatOccur = []
    dumpFile = codecs.open(dumpFileName, 'r', 'utf-8')
    try:
        for ligne in dumpFile:
            ligne = ligne.strip()
            if ligne == '': continue
            (enTete, lemmes) = ligne.split('::')
            longueur = int(enTete.split('len=')[1].strip())
            lemmesList = lemmes.split(', ')
            if len(lemmesList) != longueur: raise Exception('A %s  /  %d'%(enTete, len(lemmesList)))
            for lemme in lemmesList:
                lemme = lemme.strip()
                (terme, cG) = lemme.split(' ')
                terme = terme.strip()
                if terme.count('#') == nbComposants-1 and terme.startswith(premLettres): resultatOccur.append(terme)
    except Exception as exc: print 'ERREUR ', exc.args[0]
    dumpFile.close()
    resultat = list(set(resultatOccur))
    resultat.sort()
    print '%d termes en %d occurences : %s'%(len(resultat), len(resultatOccur), ', '.join(resultat))
         
if __name__ == '__main__':
        main()
    