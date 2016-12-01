#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from os import path, getenv
import codecs
import StringIO
from LatXmlParser import LatXmlParser

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un corpus XML LVIC en un fichier texte.
Liste les différentes valeurs des entités nommées.
Le nom du fichier texte est <fichier xml>.txt

usage   : %s <fichier xml> 
exemple : %s sample_fre.xml.mult.xml
"""%(script, script)

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        inFileName = path.realpath(sys.argv[1])
        analyseLvicDump(inFileName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print "******************************"
            print exc.args[0]
            print "******************************"
            raise
        
def analyseLvicDump(inFileName):
    dumpParser = DumpParser(inFileName)
    #affiche resultat
    namedEntityList = dumpParser.namedEntityDico.keys()
    namedEntityList.sort()
    print "%d types d'entités nommées trouvés"%(len(namedEntityList))
    for namedEntity in namedEntityList:
        print '% 5d %s'%(dumpParser.namedEntityDico[namedEntity], namedEntity)
    sys.exit()
    
class DumpParser():
    def __init__(self, inFileName):
        self.namedEntityDico = {}
        #cette classe analyse un fichier XML. Elle utilise une facon a la xpath.
        pathsArray = ['bowNamedEntity']
        #le parser
        self.xmlParser = LatXmlParser()
        self.xmlParser.setPathsAndCallback(pathsArray, self.nodeCallback, self.textCallback, self.endNodeCallback)
        self.xmlParser.startParse(inFileName)
        
    def nodeCallback(self, path, attr):
        if path.endswith('bowNamedEntity'):
            typeNe = attr.getValue('type')
            if typeNe not in self.namedEntityDico: self.namedEntityDico[typeNe] = 0
            self.namedEntityDico[typeNe] +=1
                  
    def endNodeCallback(self, path):
        return
     
    def textCallback(self, path, text):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #il est donc IMPERATIF de programmer ainsi
        #self.text += text
        return
       
    
if __name__ == '__main__':
        main()
