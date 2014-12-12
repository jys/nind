#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import re

def usage():
    print """© l'ATÉCON.
Analyse un fichier de dump par documents d'une base S2 et compte le 
nombre de termes en fonction du nombre de composants (1 pour les
termes simples, 2, 3, 4, pour les termes composés)
En affiche la statistique suivant le lexique et suivant les occurences.
Affichage : 
nbre de composants, termes en nbre et %%

usage   : %s <fichier dump par documents>
exemple : %s boxon/FRE.FDB-DumpByDocuments.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dumpFileName = os.path.abspath(sys.argv[1])
    
    #30077 <=> 8108 len=5  ::  programmer (V), salon (NC), bien (NC), vieillir (V), pdf (NC)
    termeDico = {}
    nbLigne = 0
    nbOccurence = 0
    dumpFile = codecs.open(dumpFileName, 'r', 'utf-8')
    try:
        for ligne in dumpFile:
            ligne = ligne.strip()
            if ligne == '': continue
            nbLigne +=1
            (enTete, lemmes) = ligne.split('::')
            longueur = int(enTete.split('len=')[1].strip())
            lemmesList = lemmes.split(', ')
            if len(lemmesList) != longueur: raise Exception('A %s  /  %d'%(enTete, len(lemmesList)))
            for lemme in lemmesList:
                lemme = lemme.strip()
                (terme, cG) = lemme.split(' ')
                terme = terme.strip()
                cG = cG.strip()
                if not terme in termeDico: termeDico[terme] = 0
                termeDico[terme] +=1
                nbOccurence +=1
    except Exception as exc: print 'ERREUR ', exc.args[0]
    dumpFile.close()
    #construit les stats
    try:
        repartLexique = [0, 0, 0, 0, 0]
        repartOccurences = [0, 0, 0, 0, 0]
        termes = termeDico.keys()
        for terme in termes:
            if re.match("^[^#]*$", terme) != None: 
                repartLexique[0] +=1
                repartOccurences[0] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*$", terme) != None: 
                repartLexique[1] +=1
                repartOccurences[1] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*#[^#]*$", terme) != None: 
                repartLexique[2] +=1
                repartOccurences[2] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*#[^#]*#[^#]*$", terme) != None: 
                repartLexique[3] +=1
                repartOccurences[3] += termeDico[terme]
            else: raise Exception('terme : %s'%(terme))
    except Exception as exc: print 'ERREUR ', exc.args[0]
    #affiche les statistiques
    nbTerme = len(termeDico)
    print '% 8d lignes lues dans %s'%(nbLigne, dumpFileName)
    print '% 8d termes différents'%(nbTerme)
    print '% 8d occurences de termes'%(nbOccurence)    
    print 'statistiques du lexique :'
    for i in range(4): print '%d : % 9d % 6.0f%%'%(i+1, repartLexique[i], 100.0*repartLexique[i]/nbTerme)
    print 'statistiques des occurrences :'
    for i in range(4): print '%d : % 9d % 6.0f%%'%(i+1, repartOccurences[i], 100.0*repartOccurences[i]/nbOccurence)
        
if __name__ == '__main__':
        main()
    