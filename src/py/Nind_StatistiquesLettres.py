#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import re

def usage():
    print """© l'ATÉCON.
Analyse un fichier de dump par documents d'une base S2 et compte le 
nombre de termes en fonction du nombre de composants et de la lettre
de tête.
En affiche la statistique suivant les occurences.
Affichage : pour chaque type de termes (1, 2, 3, 4 composants):
première lettre, nbre de termes dans le lexique, dans les occurrences

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
        repart = [{}, {}, {}, {}]
        termes = termeDico.keys()
        for terme in termes:
            premLettre = terme[0]
            if re.match("^[^#]*$", terme) != None: 
                if not premLettre in repart[0]: repart[0][premLettre] = [0, 0]
                repart[0][premLettre][1] +=1
                repart[0][premLettre][0] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*$", terme) != None: 
                if not premLettre in repart[1]: repart[1][premLettre] = [0, 0]
                repart[1][premLettre][1] +=1
                repart[1][premLettre][0] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*#[^#]*$", terme) != None: 
                if not premLettre in repart[2]: repart[2][premLettre] = [0, 0]
                repart[2][premLettre][1] +=1
                repart[2][premLettre][0] += termeDico[terme]
            elif re.match("^[^#]*#[^#]*#[^#]*#[^#]*$", terme) != None: 
                if not premLettre in repart[3]: repart[3][premLettre] = [0, 0]
                repart[3][premLettre][1] +=1
                repart[3][premLettre][0] += termeDico[terme]
            else: raise Exception('terme : %s'%(terme))
    except Exception as exc: print 'ERREUR ', exc.args[0]
    #affiche les statistiques
    nbTerme = len(termeDico)
    print '% 8d lignes lues dans %s'%(nbLigne, dumpFileName)
    print '% 8d termes différents'%(nbTerme)
    print '% 8d occurences de termes'%(nbOccurence)  
    for i in range(4):
        print '%d composant(s):'%(i+1)
        lettres = []
        for (lettre, compt) in repart[i].items(): lettres.append((compt, lettre))
        lettres.sort()
        lettres.reverse()
        #for (compt, lettre) in lettres: print '%s : %d %d'%(lettre, compt[1], compt[0])
        resultat = []
        for (compt, lettre) in lettres: resultat.append('%s : %d|%d'%(lettre, compt[1], compt[0]))
        print ', '.join(resultat)
        
if __name__ == '__main__':
        main()
    