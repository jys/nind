#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs

def usage():
    print """
Compare la liste de mots extraites des index locaux et la liste
de mots extraite du lexique de la base fdb.

usage   : %s <Lexique S2> <Liste de mots des index locaux>
exemple : %s fre-theJysBox.fdb-DumpByLexicon.txt  fre-theJysBox.fdb-WordsList.txt
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3 :
        usage()
        sys.exit()
    s2LexiconName = os.path.abspath(sys.argv[1])
    wordsListName = os.path.abspath(sys.argv[2])
    
    #traite le lexique S2
    s2Set = set()
    s2Lexicon = codecs.open(s2LexiconName, 'r', 'utf-8')
    #   "œil#étincelant  100033367"
    for line in s2Lexicon:
        line = line.strip()
        if line == '': continue
        lineArray = line.split()
        word = lineArray[0].strip()
        s2Set.add(word)
    s2Lexicon.close()
    
    #traite la liste de mots des index locaux
    wordListSet = set()
    wordsList = codecs.open(wordsListName, 'r', 'utf-8')
    #   "fut#obligé"
    for line in wordsList:
        line = line.strip()
        if line == '': continue
        wordListSet.add(line)
    wordsList.close()
    
    print '%d mots trouvés dans le lexique S2'%(len(s2Set))
    print '%d mots trouvés dans les index locaux'%(len(wordListSet))
    diff1 = s2Set - wordListSet
    print '%d mots trouvés dans le lexique S2 et pas dans les index locaux'%(len(diff1))
    diff2 = wordListSet - s2Set
    print '%d mots trouvés dans les index locaux  et pas dans le lexique S2'%(len(diff2))
    
    
   
    
if __name__ == '__main__':
    main()

 

