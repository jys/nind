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
Convertit un corpus XML LVIC en un fichier texte.
Le nom du fichier texte est <fichier xml>.txt
Il y a une ligne par document: noDoc { terme localisation,longueur }
Le fichier texte est le corpus déjà analysé et prêt pour l'indexation.

usage   : %s <fichier xml> 
exemple : %s sample_fre.xml.mult.xml
"""%(script, script)

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        inFileName = path.realpath(sys.argv[1])
        convertitLvicDump(inFileName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print "******************************"
            print exc.args[0]
            print "******************************"
            raise
        
def convertitLvicDump(inFileName):
    #nom fichier de sortie
    outFileName = inFileName + '.txt'
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    dumpParser = DumpParser(inFileName, outFile)
    outFile.close()
    #affiche resultat
    (comptDocs, comptIds, comptTermes) = dumpParser.compteurs
    print '%d documents trouvés'%(comptDocs)
    print '%d identifiants de documents trouvés'%(comptIds)
    print '%d occurrences de termes trouvées'%(comptTermes)
    sys.exit()
        
class DumpParser():
    def __init__(self, inFileName, outFile):
        self.outFile = outFile
        self.compteurs = [0, 0, 0]
        #cette classe analyse un fichier XML. Elle utilise une facon a la xpath.
        pathsArray = ['/MultimediaDocuments/node/node', '/MultimediaDocuments/node/node/properties/property', 'bowToken', 'bowTerm', 'bowNamedEntity']
        #le parser
        self.xmlParser = LatXmlParser()
        self.xmlParser.setPathsAndCallback(pathsArray, self.nodeCallback, self.textCallback, self.endNodeCallback)
        self.xmlParser.startParse(inFileName)
        
    def nodeCallback(self, path, attr):
        if path == '/MultimediaDocuments/node/node':
            if attr.getValue('elementName') != 'DOC': 
                raise Exception('/MultimediaDocuments/node/node à la ligne %d'%(self.xmlParser.getLineNumber()))
            #nouveau document, on initialise le receptacle a termes
            self.termSet = set()
            self.compteurs[0] +=1
        elif path == '/MultimediaDocuments/node/node/properties/property':
            if attr.getValue('name') == 'StructureId':
                self.docId = int(attr.getValue('value'))
                self.compteurs[1] +=1
        elif path.endswith('bowToken') or path.endswith('bowTerm'):
            lemma = attr.getValue('lemma')
            position = int(attr.getValue('position'))
            length = int(attr.getValue('length'))
            if len(lemma) != 0: self.termSet.add((position, length, lemma))
        elif path.endswith('bowNamedEntity'):
            lemma = attr.getValue('lemma')
            position = int(attr.getValue('position'))
            length = int(attr.getValue('length'))
            typeNe = attr.getValue('type')
            if len(lemma) != 0: self.termSet.add((position, length, typeNe + ':' + lemma))
                  
    def endNodeCallback(self, path):
        if path == '/MultimediaDocuments/node/node':
            self.compteurs[2] += len(self.termSet)
            termList = list(self.termSet)
            termList.sort()
            self.outFile.write('%d'%(self.docId))
            for (position, length, lemma) in termList:
                self.outFile.write(' %s %d,%d'%(lemma, position, length))
            self.outFile.write('\n')
    
    def textCallback(self, path, text):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #il est donc IMPERATIF de programmer ainsi
        #self.text += text
        return
       
    
if __name__ == '__main__':
        main()
        