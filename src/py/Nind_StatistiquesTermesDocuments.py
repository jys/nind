#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """© l'ATÉCON.
Analyse un fichier de dump par documents d'une base S2 et compte le 
nombre de termes en fonction du nombre de documents analysés, dans 
l'ordre de l'analyse de la base.
Écrit le résultat dans le fichier <fichier dump>-Stats.csv avec
<n° doc>;<termes S>;<termes M>;<occur S>;<occur M>;
<cumul termes S>;<cumul termes M>;<cumul occur S>;<cumul occur M>

usage   : %s <fichier dump par documents>
exemple : %s boxon/FRE.FDB-DumpByDocuments.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dumpFileName = os.path.abspath(sys.argv[1])
    outFileName = '%s-Stats.csv'%(dumpFileName)
    
    #30077 <=> 8108 len=5  ::  programmer (V), salon (NC), bien (NC), vieillir (V), pdf (NC)
    cumulTermesS = set()
    cumulTermesM = set()
    cumulOccurS = cumulOccurM = 0
    nbLigne = 0
    dumpFile = codecs.open(dumpFileName, 'r', 'utf-8')
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        for ligne in dumpFile:
            ligne = ligne.strip()
            if ligne == '': continue
            nbLigne +=1
            (enTete, lemmes) = ligne.split('::')
            longueur = int(enTete.split('len=')[1].strip())
            lemmesList = lemmes.split(', ')
            if len(lemmesList) != longueur: raise Exception('A %s  /  %d'%(enTete, len(lemmesList)))
            termesS = set()
            termesM = set()
            occurS = occurM = 0
            for lemme in lemmesList:
                lemme = lemme.strip()
                (terme, cG) = lemme.split(' ')
                terme = terme.strip()
                #cG = cG.strip()
                if '#' in terme: 
                    termesM.add(terme)
                    cumulTermesM.add(terme)
                    occurM +=1
                    cumulOccurM +=1
                else:
                    termesS.add(terme)
                    cumulTermesS.add(terme)
                    occurS +=1
                    cumulOccurS +=1
            outFile.write('%d;%d;%d;%d;%d;%d;%d;%d;%d\n'%(nbLigne, len(termesS), len(termesM), occurS, occurM, len(cumulTermesS), len(cumulTermesM), cumulOccurS, cumulOccurM))
    except Exception as exc: print 'ERREUR ', exc.args[0]
    dumpFile.close()
    outFile.close()
    
    print '% 8d lignes lues dans %s'%(nbLigne, dumpFileName)
    print '% 8d lignes écrites dans %s'%(nbLigne, outFileName)
    print '% 8d termes simples différents'%(len(cumulTermesS))
    print '% 8d termes composés différents'%(len(cumulTermesM))
    print '% 8d occurrences termes simples'%(cumulOccurS)
    print '% 8d occurrences termes composés'%(cumulOccurM)
    
if __name__ == '__main__':
        main()
    