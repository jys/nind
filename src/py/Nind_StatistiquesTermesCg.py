#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """© l'ATÉCON.
Analyse un fichier de dump par documents d'une base S2 et compte le 
nombre de CG pour chaque terme et en affiche la statistique.
Idem pour les termes simples uniquement.
affichage : 
nbre de CG, termes en nbre et %%, occurences/termes, occurrences en nbre et %%

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
    termeSimpleDico = {}
    nbLigne = 0
    nbOccurence = 0
    nbOccurenceSimple = 0
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
                if not terme in termeDico: termeDico[terme] = [set(), 0]
                termeDico[terme][0].add(cG)
                termeDico[terme][1] +=1
                nbOccurence +=1
                if not '#' in terme:
                    if not terme in termeSimpleDico: termeSimpleDico[terme] = [set(), 0]
                    termeSimpleDico[terme][0].add(cG)
                    termeSimpleDico[terme][1] +=1
                    nbOccurenceSimple +=1
                        
    except Exception as exc: print 'ERREUR ', exc.args[0]
    dumpFile.close()
    nbTerme = len(termeDico)
    print '% 8d lignes lues dans %s'%(nbLigne, dumpFileName)
    print '% 8d termes différents'%(nbTerme)
    print '% 8d occurences de termes'%(nbOccurence)    
    afficheStatistiques(termeDico, nbTerme, nbOccurence)
    nbTerme = len(termeSimpleDico)
    print '% 8d termes simples différents'%(nbTerme)
    print '% 8d occurences de termes simples'%(nbOccurenceSimple)    
    afficheStatistiques(termeSimpleDico, nbTerme, nbOccurenceSimple)
    
def afficheStatistiques(termeDico, nbTerme, nbOccurence):
    compteDico = {}
    for (terme, (cGSet, nbTermeOccur)) in termeDico.items():
        nbCg = len(cGSet)
        if not nbCg in compteDico: compteDico[nbCg] = [0, 0]
        compteDico[nbCg][0] +=1
        compteDico[nbCg][1] += nbTermeOccur
    compteList = compteDico.items()
    compteList.sort()
    print '     termes                             occurences'
    for (nbCg, (termes, occurences)) in compteList: 
        print '%d : % 7d  % 6.2f%%      x% 5d      % 9d  % 6.2f%%'%(nbCg, termes, 100.0*termes/nbTerme, occurences/termes, occurences, 100.0*occurences/nbOccurence)
    
if __name__ == '__main__':
        main()
    