#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
from os import path, getenv
import codecs

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print """© l'ATEJCON.
Analyse un fichier lexique, sauvegarde d'un lexique en mémoire et l'écrit 
en clair sur un fichier texte. 
Le format du fichier est défini dans le document LAT2014.JYS.440.
Le fichier de sortie s'appelle <fichier lexicon>-dump.txt

usage   : %s <fichier lexicon>
exemple : %s box/dumps/boxon/FRE.lexicon
"""%(script, script)

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    lexiconFileName = path.abspath(sys.argv[1])
    outFileName = '%s-dump.txt'%(lexiconFileName)

    #// <fichierLexique>        ::= <lexique> <identification>
    #// <lexique>               ::= { <definitionMot> }
    #// <definitionMot>         ::= [ <definitionMotSimple> | <definitionMotCompose> ]
    #// <definitionMotSimple>   ::= <flagMotSimple> <identifiant> <motSimple>
    #// <definitionMotCompose>  ::= <flagMotCompose> <identifiant> <motCompose>
    #// <flagMotSimple>         ::= <Integer1>
    #// <flagMotCompose>        ::= <Integer1>
    #// <motSimple>             ::= <longueurMot> <motUtf8>
    #// <motCompose>            ::= <identifiant> <identifiant>
    #// <longueurMot>           ::= <Integer1>
    #// <motUtf8>               ::= { <Byte> }
    #// <identifiant>           ::= <Integer3>
    #// <identification>        ::= <flagIdentification> <identificationLexique>
    #// <flagIdentification>    ::= <Integer1>
    #// <identificationLexique> ::= <maxIdentifiant> <identifieurUnique>
    #// <maxIdentifiant>        ::= <Integer3>
    #// <identifieurUnique>     ::= <dateHeure>
    #// <dateHeure >            ::= <Integer4>
    SIMPLE_WORD_FLAG = 13
    COMPOUND_WORD_FLAG = 29
    IDENTIFICATION_FLAG = 53

    simplesDico = {}
    composesDico = {}
    nbTermes = 0
    lexiconFile = open(lexiconFileName, 'rb')
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    try:
        #lit flag 
        octet = lexiconFile.read(1)
        while octet != '':
            flag = ord(octet)
            if flag == SIMPLE_WORD_FLAG:
                identifiant = litNombre3(lexiconFile)
                longueur = ord(lexiconFile.read(1))
                mot = lexiconFile.read(longueur).decode('utf-8')
                outFile.write('%06d : %s\n'%(identifiant, mot))   
                simplesDico[identifiant] = mot
                nbTermes +=1
            elif flag == COMPOUND_WORD_FLAG:
                identifiant = litNombre3(lexiconFile)
                identifiant1 = litNombre3(lexiconFile)
                identifiant2 = litNombre3(lexiconFile)
                mot = calculeMotCompose(simplesDico[identifiant2], identifiant1, simplesDico, composesDico)
                outFile.write('%06d : (%06d, %06d)  %s\n'%(identifiant, identifiant1, identifiant2, mot))
                composesDico[identifiant] = (identifiant1, identifiant2)
                nbTermes +=1
            elif flag == IDENTIFICATION_FLAG:
                maxIdentifiant = litNombre3(lexiconFile)
                identifieurUnique = litNombre4(lexiconFile)
                outFile.write('############## max=%d dateheure=%d ##############\n'%(maxIdentifiant, identifieurUnique))                      
            else: raise Exception('flag inconnu : %d'%(flag))
            octet = lexiconFile.read(1)
    except Exception as exc: 
        print 'ERREUR :', exc.args[0]
        #raise
    lexiconFile.close()
    outFile.close()
    print '%d identifiants de termes écrits dans %s'%(nbTermes, outFileName)
    
def litNombre3(lexiconFile):
    #le fichier est écrit en little-endian
    octets = lexiconFile.read(3)
    return (ord(octets[2])*256 + ord(octets[1]))*256 + ord(octets[0])

def litNombre4(lexiconFile):
    #le fichier est écrit en little-endian
    octets = lexiconFile.read(4)
    return ((ord(octets[3])*256 + ord(octets[2]))*256 + ord(octets[1]))*256 + ord(octets[0])

def calculeMotCompose(motPartiel, identifiant, simplesDico, composesDico):
    if identifiant in simplesDico: return simplesDico[identifiant] + '#' + motPartiel
    (identifiant1, identifiant2) = composesDico[identifiant]
    return calculeMotCompose(simplesDico[identifiant2] + '#' + motPartiel, identifiant1, simplesDico, composesDico)
    
    
if __name__ == '__main__':
        main()
    
        