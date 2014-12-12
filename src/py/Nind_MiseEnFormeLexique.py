#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import re

def usage():
    print """© l'ATÉCON.
Met en forme le fichier dump du lexique en regroupant les mots composés
autour du mot simple d'origine.
(000005 : (000003, 000004)  suivi#recherche#financement est dérivé de
000004 : financement)
Écrit le résultat sur <fichier dump lexique>-org.txt

usage   : %s <fichier dump lexique>
exemple : %s box/dumps/boxon/FRE.lexicon-dump.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dumpFileName = os.path.abspath(sys.argv[1])
    
    #fichier de sortie
    outFilename = dumpFileName.replace('.txt', '-org.txt')
    
    motDico = {}
    dumpFile = codecs.open(dumpFileName, 'r', 'utf-8')
    for ligne in dumpFile:
        ligne = ligne.strip()
        if ligne == '': continue
        match = re.search('^([0-9]{6}) : ?([^#]*)$', ligne)
        if match:
            ident = int(match.group(1))
            mot = match.group(2)
            if ident in motDico: raise 'ERREUR %06d déjà trouvé (%s)'%(ident, ligne)
            motDico[ident] = (mot, [])
            continue
        match = re.search('^([0-9]{6}) : \(([0-9]{6}), ([0-9]{6})\).*$', ligne)
        if match:
            ident = int(match.group(1))
            identA = int(match.group(2))
            identB = int(match.group(3))
            if identB not in motDico: raise 'ERREUR %06d : pas trouvé (%s)'%(identB, ligne)
            motDico[identB][1].append((identA, ident))
            continue
        print ligne
        #raise 'ERREUR %s'%(ligne)
    dumpFile.close()    
    print '%d termes simples'%(len(motDico))
    
    repartComp = {}
    nbComp = 0
    outFile = codecs.open(outFilename, 'w', 'utf-8')
    keys = motDico.keys()
    keys.sort()
    for key in keys:
        nb = len(motDico[key][1])
        if nb not in repartComp: repartComp[nb] = 0
        repartComp[nb] +=1
        nbComp += nb
        if nb > 2000: print motDico[key][0]
        outFile.write('%06d (%s) '%(key, motDico[key][0]))
        composes = []
        for (identA, ident) in motDico[key][1]: composes.append('%06d+=%06d'%(identA, ident))
        outFile.write(', '.join(composes))
        outFile.write('\n')                                    
    outFile.close()
    print '%d termes composés'%(nbComp)
    
    nbs = repartComp.keys()
    nbs.sort()
    result = []
    for nb in nbs: result.append('%d:%d'%(nb, repartComp[nb]))
    print ', '.join(result)
        
        
        
if __name__ == '__main__':
        main()
    
