#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import Lexicon

def usage():
    print """
Trouve les mots simples qui sont dans le lexique S2 et pas
dans le nouveau lexique.

usage   : %s <path base Firebird> <fichier index locaux>
exemple : %s index/fre-boxon.fdb fre-boxon.fdb-DumpByDocuments.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    dbName = os.path.abspath(sys.argv[1])
    textFileName = os.path.abspath(sys.argv[2])

    print '1) établit la liste des mots simples du lexique de la base'
    #etablit le lexique 
    lexicon = Lexicon.Lexicon(dbName, True)
    #sort la liste du lexique
    lexiconWords = lexicon.getLexicon()
    keys = lexiconWords.keys()
    keys.sort()
    print '%d entrées dans lexiconWords'%(len(keys))
    #stock de mots simples
    simpleWordsSet = set()
    for key in keys:
        #on ne prend que les mots simples
        if key >= 100000000: break
        simpleWordsSet.add(lexiconWords[key].decode('utf-8'))
    print '%d mots simples trouvés dans %s'%(len(simpleWordsSet), dbName)
    
    print '2) établit la liste des mots simples des index locaux'
    indexWordsSet = set()
    textFile = codecs.open(textFileName, 'r', 'utf-8')
    docCount = 0
    for line in textFile:
        line = line.strip()
        if len(line) == 0: continue
        docCount +=1
        #2283 <=> 926 len=4  ::  ant (NP), inno (NC), ged (NP), collaborative (NC)
        #on decode comme le C++ car il y a des nombres a virgule
        posb = line.find('::  ')+4
        while True:
            pose = line.find(' ', posb)
            word = line[posb:pose]
            simpleWords = word.split('#')
            for simpleWord in simpleWords:
                indexWordsSet.add(simpleWord)            
            posb = pose +1
            pose = line.find('), ', posb)
            if pose == -1: break
            posb = pose +3
    textFile.close()
    print '%d documents trouvés'%docCount
    print '%d mots simples trouvés dans %s'%(len(indexWordsSet), textFileName)

    diff1 = simpleWordsSet - indexWordsSet
    print '%d mots simples dans %s et pas dans %s'%(len(diff1), dbName, textFileName)
    print diff1
    
    diff2 = indexWordsSet - simpleWordsSet
    print '%d mots simples dans %s et pas dans %s'%(len(diff2), textFileName, dbName)
    print diff2
    
    
if __name__ == '__main__':
    main()
    
        

