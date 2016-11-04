#!/usr/bin/env python
# -*- coding: utf-8 -*-
#import sys
#import os
#import codecs
#import datetime
#import time
#import NindLateconFile

#utilitaires communs aux classes NindLexiconindex, NindLexiconindexInverse, NindLocalindex, NindTermindex
# <fichier>               ::= <blocIndirection> { <blocIndirection> <blocDefinition> } <blocIdentification> 
#
# <blocIndirection>       ::= <flagIndirection> <addrBlocSuivant> <nombreIndirection> { indirection }
# <flagIndirection>       ::= <Integer1>
# <addrBlocSuivant>       ::= <Integer5>
# <nombreIndirection>     ::= <Integer3>
# <indirection>           ::= <offsetDefinition> <longueurDefinition> 
# <offsetDefinition>      ::= <Integer5>
# <longueurDefinition>    ::= <Integer3>
#
# <blocDefinition>        ::= { <definition> | <vide> }
# <definition>            ::= { <Octet> }
# <vide>                  ::= { <Octet> }
#
# <blocIdentification>    ::= <flagIdentification> <maxIdentifiant> <identifieurUnique>
# <flagIdentification>    ::= <Integer1>
# <maxIdentifiant>        ::= <Integer3>
# <identifieurUnique>     ::= <dateHeure>
# <dateHeure >            ::= <Integer4>

def getIdentification(latFile, latFileName):
    FLAG_IDENTIFICATION = 53
    TAILLE_IDENTIFICATION = 8
    #<flagIdentification_1> <maxIdentifiant_3> <identifieurUnique_4>
    latFile.seek(-TAILLE_IDENTIFICATION, 2)
    if latFile.litNombre1() != FLAG_IDENTIFICATION: raise Exception('pas FLAG_IDENTIFICATION sur %s'%(latFileName))
    maxIdentifiant = latFile.litNombre3()
    dateHeure = latFile.litNombre4()
    return (maxIdentifiant, dateHeure)
    
def getDefinitionAddr(latFile, latFileName, identifiant):
    FLAG_INDIRECTION = 47  
    TETE_INDIRECTION = 9
    TAILLE_INDIRECTION = 8
    latFile.seek(0, 0)
    maxIdent = 0
    startIndirection = 0
    while True:
        addrIndirection = latFile.tell()
        #<flagIndirection> <addrBlocSuivant> <nombreIndirection>
        if latFile.litNombre1() != FLAG_INDIRECTION: raise Exception('%s : pas FLAG_INDIRECTION à %08X'%(latFileName, addrIndirection))
        indirectionSuivante = latFile.litNombre5()
        nombreIndirection = latFile.litNombre3()
        maxIdent += nombreIndirection
        if maxIdent > identifiant: break
        if indirectionSuivante == 0: break
        startIndirection = indirectionSuivante
        latFile.seek(startIndirection, 0)
    if maxIdent < identifiant: return (0, 0)          #identifiant hors limite
    #lit l'indirection
    index = identifiant + nombreIndirection - maxIdent
    #lit la définition du terme
    addrIndir = startIndirection + TETE_INDIRECTION + (index * TAILLE_INDIRECTION)
    latFile.seek(addrIndir, 0)
    #<offsetDefinition> <longueurDefinition>
    offsetDefinition = latFile.litNombre5()
    longueurDefinition = latFile.litNombre3()
    return (offsetDefinition, longueurDefinition)
