#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import re
import NindLateconFile

def usage():
    print """© l'ATÉCON.
Analyse un fichier contenant une liste de mots en UTF-8 (un mot par ligne).
Donne la répartition des mots en fonction de leur longueur (en octets et en
caractères Unicode).
Donne les répartitions des mots sur la clef A, sur la clef B et sur le 
doublet (clef A, clef B).

usage   : %s <fichier de mots>
exemple : %s dump/FRE-corpus.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dumpFileName = os.path.abspath(sys.argv[1])
    
    #prend tous les mots simples
    motsSimples = []
    dumpFile = codecs.open(dumpFileName, 'r', 'utf-8')
    for ligne in dumpFile:
        ligne = ligne.strip()
        if ligne == '': continue
        motsSimples.append(ligne)
    dumpFile.close()    
    print '%d mots simples trouvés dans %s'%(len(motsSimples), dumpFileName)
            
    #les tailles
    taillesBytes = {}
    taillesUtf8 = {}
    for mot in motsSimples:
        tUtf8 = len(mot)
        tBytes = len(mot.encode('utf-8'))
        if not tBytes in taillesBytes: taillesBytes[tBytes] = 0
        taillesBytes[tBytes] +=1
        if not tUtf8 in taillesUtf8: taillesUtf8[tUtf8] = 0
        taillesUtf8[tUtf8] +=1
        #if tUtf8 == 53: print mot
    affiche('octets', taillesBytes)
    affiche('UTF-8', taillesUtf8)
    
    #le hash-code en octets
    codeH = {}
    #1er hash
    for mot in motsSimples:
        keyA = NindLateconFile.clefA(mot)
        if keyA not in codeH: codeH[keyA] = []
        codeH[keyA].append(mot)
    afficheClefs('code A', codeH)
    #2eme hash
    codeH.clear()
    for mot in motsSimples:
        keyB = NindLateconFile.clefB(mot)
        if keyB not in codeH: codeH[keyB] = []
        codeH[keyB].append(mot)
    afficheClefs('code B', codeH)
    #3eme hash
    codeH.clear()
    for mot in motsSimples:
        keyA = NindLateconFile.clefA(mot)
        keyB = NindLateconFile.clefB(mot)
        if (keyA, keyB) not in codeH: codeH[(keyA, keyB)] = []
        codeH[(keyA, keyB)].append(mot)
    afficheClefs('code A+B', codeH)
    
def afficheClefs(titre, codeH):
    tailleKey = {}
    for (key, mots) in codeH.items():
        nbMots = len(mots)
        if nbMots not in tailleKey: tailleKey[nbMots] = 0
        tailleKey[nbMots] +=1
        if nbMots > 5: print '%X :: %s'%(key,', '.join(mots))
    resultat = []
    keys = tailleKey.keys()
    keys.sort()
    for key in keys: resultat.append('%d:%d'%(key, tailleKey[key]))
    print '%s => %d clefs (%s)'%(titre, len(codeH), ', '.join(resultat))
    
    
def affiche(titre, tailles):
    resultat = []
    total = 0
    keys = tailles.keys()
    keys.sort()
    for key in keys:
        resultat.append('%d:%d'%(key, tailles[key]))
        total += key * tailles[key]
    print '%s :: %d (%s)'%(titre, total, ', '.join(resultat))
                      
        
        
if __name__ == '__main__':
        main()
