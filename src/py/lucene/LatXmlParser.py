#!/usr/bin/env python
# -*- coding: utf-8 -*-

#cette classe analyse un fichier XML. Elle utilise une facon a la xpath.

import sys
import os
import StringIO
from xml.sax.handler import ContentHandler
import xml.sax

def usage():
    print """© l'ATÉCON.
Programme de test de la classe LatXmlParser.
Attention, il faut respecter la façon de récupérer les textes 
telle que programmée dans le présent programme de test,
sinon il y a risque de longues déconvenues...

usage   : %s <paths séparés par ";"> <fichier xml> 
exemple : %s "/bowText/tokens/bowTerm;bowTerm;" toto.xml 
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 3:
        usage()
        sys.exit()
    pathsString = sys.argv[1]
    inFileName = os.path.realpath(sys.argv[2])
    #prepare l'array de paths
    pathsArray = pathsString.split(';')
    #c'est une classe qui va faire le boulot
    TestParser(inFileName, pathsArray)
    print "Terminé"
    
class TestParser():
    def __init__(self, inFileName, pathsArray):
        #le parser
        self.xmlParser = LatXmlParser()
        self.xmlParser.setPathsAndCallback(pathsArray, self.nodeCallback, self.textCallback, self.endNodeCallback)
        self.xmlParser.startParse(inFileName)
        
    def nodeCallback(self, path, attr):
        outText = StringIO.StringIO()
        outText.write('%d: %s => '%(self.xmlParser.getLineNumber(), path))
        for name in attr.getNames(): outText.write('%s=%s '%(name, attr.getValue(name)))
        print outText.getvalue()
        self.text = ''
        
    def endNodeCallback(self, path):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #on ne peut disposer du texte complet qu'a la balise fermante
        #il est donc IMPERATIF de programmer ainsi        
        print ('#%s#'%(self.text)).encode('utf-8')
        print '%d: end %s'%(self.xmlParser.getLineNumber(), path)

    def textCallback(self, path, text):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #il est donc IMPERATIF de programmer ainsi
        self.text += text
            
########################################

class LatXmlParser:
    def __init__(self):
        #init le parser SAX
        self.parser = xml.sax.make_parser()
        self.handler = XmlParserHandler()
        self.parser.setContentHandler(self.handler)
        
    def setPathsAndCallback(self, paths, nodeCallback, textCallback, endNodeCallback):
        self.handler.setPathsAndCallback(paths, nodeCallback, textCallback, endNodeCallback)
        
    def startParse(self, xmlFileName):
        self.parser.parse(xmlFileName)
        
    def getLineNumber(self):
        return self.parser.getLineNumber()

class XmlParserHandler(ContentHandler):
    def __init__(self):
        self.searchPaths = []
        self.currentPath = [""]
        
    def setDocumentLocator(self, locator):
        self.locator = locator
        
    def setPathsAndCallback(self, paths, nodeCallback, textCallback, endNodeCallback):
        #transforme les paths recherches en array d'array
        self.searchPaths = []
        for onePath in paths : self.searchPaths.append(onePath.split('/'))
        #memorise la callback
        self.nodeCallback = nodeCallback
        self.textCallback = textCallback
        self.endNodeCallback = endNodeCallback
        
    def startElement(self, name, attr):
        #agrandit le path courant
        self.currentPath.append(name)
        #si le bon path, appelle la nodeCallback
        if self.pathIsOk(): self.nodeCallback('/'.join(self.currentPath), attr)
        
    def endElement (self, name):
        #si le bon path, appelle la endNodeCallback
        if self.pathIsOk(): self.endNodeCallback('/'.join(self.currentPath))
        #diminue le path courant
        last = self.currentPath.pop()
        #verifie la syntaxe
        if last != name: raise Exception('</%s> au lieu de </%s>'%(name, last))
            
    def characters(self,ch):
        #si le bon path, et pas vide appelle la textCallback
        if ch == "": return
        if self.pathIsOk(): self.textCallback('/'.join(self.currentPath), ch)
            
    def pathIsOk(self):
        #verifie les paths absolus d'abord
        if self.currentPath in self.searchPaths: return True
        #ensuite, les paths relatifs
        for i in range(len(self.currentPath)): 
            if self.currentPath[i+1:] in self.searchPaths: return True
        return False
    
    def getLineNumber(self):
        return self.locator.getLineNumber()

    
if __name__ == '__main__':
        main()
