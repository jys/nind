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
    print """© l'ATÉCON.
Convertit un dump amose de Lucene en un fichier texte.
Le nom du fichier texte est <fichier xml>.txt
Il y a une ligne par document: noDoc { terme localisation,longueur }
Le fichier texte est le corpus déjà analysé et prêt pour l'indexation.

usage   : %s <fichier xml> 
exemple : %s dump/lucene/amose-dump-lucene-index-fre-10.xml
"""%(script, script)

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        inFileName = path.realpath(sys.argv[1])
        convertitLuceneDump(inFileName)
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print "******************************"
            print exc.args[0]
            print "******************************"
            raise
        sys.exit()

def convertitLuceneDump(inFileName):
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

class DumpParser():
    def __init__(self, inFileName, outFile):
        self.outFile = outFile
        self.compteurs = [0, 0, 0]
        #cette classe analyse un fichier XML. Elle utilise une facon a la xpath.
        pathsArray = ['/dump/documents/document', '/dump/documents/document/field', '/dump/documents/document/term']
        #le parser
        xmlParser = LatXmlParser()
        xmlParser.setPathsAndCallback(pathsArray, self.nodeCallback, self.textCallback, self.endNodeCallback)
        xmlParser.startParse(inFileName)
        
    def nodeCallback(self, path, attr):
        if path == '/dump/documents/document': 
            #nouveau document
            self.termes = []
            self.compteurs[0] +=1
        elif path == '/dump/documents/document/field':
            #recherche n° de document
            #<field name="docid" value="10170346">
            if attr.getValue('name') == 'docid':
                #ecrit l'identifiant du document
                self.outFile.write('%s '%(attr.getValue('value')))
                self.compteurs[1] +=1
        elif path == '/dump/documents/document/term':
            #recupere un terme et sa localisation
            #<term text="démocratique" tag="1" freq="2" positions="308-320;308-320">
            terme = attr.getValue('text')
            localisations = attr.getValue('positions')
            for localisation in localisations.split(';'):
                (deb, fin) = localisation.split('-')
                self.termes.append((int(deb), int(fin), terme))
                self.compteurs[2] +=1
        
    def endNodeCallback(self, path):
        if path == '/dump/documents/document': 
            #fin document
            #met les termes par ordre d'apparition et les ecrit
            self.termes.sort()
            for (deb, fin, terme) in self.termes:
                self.outFile.write(' %s %d,%d'%(terme, deb, fin-deb))
            #ligne suivante
            self.outFile.write('\n')

    def textCallback(self, path, text):
        #Sax ne s'interdit pas d'envoyer les textes par morceaux ! 
        #il est donc IMPERATIF de programmer ainsi
        #self.text += text
        return
       
    
if __name__ == '__main__':
        main()
