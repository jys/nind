#!/usr/bin/env python
# -*- coding: utf-8 -*-

#cette classe analyse un fichier XML. Elle utilise une facon a la xpath.

import sys
from os import path, getenv
from LatXmlParser import LatXmlParser

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATÉCON.
Vérifie la cohérence d'un dump amose de Lucene.
Donne le nombre de documents et de termes.

usage   : %s <fichier xml> 
exemple : %s dump/lucene/amose-dump-lucene-index-fre-10.xml
"""%(script, script)

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        inFileName = path.realpath(sys.argv[1])
        analyseLuceneDump(inFileName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print "******************************"
            print exc.args[0]
            print "******************************"
            raise
        sys.exit()
  
def analyseLuceneDump(inFileName):
    dumpParser = DumpParser(inFileName, ['/dump/lexicon/term/document'], 'docid')
    docidSet = dumpParser.resultat
    print '%d documents différents dans "/dump/lexicon/term/document"'%(len(docidSet))
    
    dumpParser = DumpParser(inFileName, ['/dump/documents/document/field'], 'value', 'name', ['docid'])
    valueSet = dumpParser.resultat
    print '%d documents différents dans "/dump/documents/document/field"'%(len(valueSet))
    
    if docidSet == valueSet: print 'DOCUMENTS OK'
    else: print docidSet ^ valueSet
    
    dumpParser = DumpParser(inFileName, ['/dump/lexicon/term'], 'text', 'field', ['1', '2', '3'])
    termTextASet = dumpParser.resultat
    print '%d termes différents dans "/dump/lexicon/term"'%(len(termTextASet))

    dumpParser = DumpParser(inFileName, ['/dump/documents/document/term'], 'text')
    termTextBSet = dumpParser.resultat
    print '%d termes différents dans "/dump/documents/document/term"'%(len(termTextBSet))
    
    if termTextASet == termTextBSet: print 'TERMES OK'
    else: print termTextASet ^ termTextBSet
    
    
class DumpParser():
    def __init__(self, inFileName, pathsArray, attribut, attCond = '', attValues = []):
        self.attribut = attribut
        self.attCond = attCond
        self.attValues = attValues
        self.resultat = set()
        #le parser
        xmlParser = LatXmlParser()
        xmlParser.setPathsAndCallback(pathsArray, self.nodeCallback, self.textCallback, self.endNodeCallback)
        xmlParser.startParse(inFileName)
        
    def nodeCallback(self, path, attr):
        if self.attCond == '' or attr.getValue(self.attCond) in self.attValues:
            for name in attr.getNames(): 
                if name == self.attribut: self.resultat.add(attr.getValue(name))
        #self.text = ''
        
    def endNodeCallback(self, path):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #on ne peut disposer du texte complet qu'a la balise fermante
        #il est donc IMPERATIF de programmer ainsi     
        return

    def textCallback(self, path, text):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #il est donc IMPERATIF de programmer ainsi
        #self.text += text
        return
           
if __name__ == '__main__':
        main()
