#!/usr/bin/env python3
# -*- coding: utf-8 -*-
__author__ = "jys"
__copyright__ = "Copyright (C) 2017 LATEJCON"
__license__ = "GNU LGPL"
__version__ = "2.0.1"
# Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
# Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
# This file is part of NIND (as "nouvelle indexation").
# NIND is free software: you can redistribute it and/or modify it under the terms of the 
# GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
# (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
# NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Less General Public License for more details.
################################################################
# IMPORTANT : des essais ont ejtej faits avec une bufferisation des lectures comme pour NindLateconFile.cpp
# Rejsultats trehs dejcevants en temps de traitement.
# Nous gardons la lecture directe depuis le fichier et remercions la gestion du cache !
################################################################
import sys
import os

def usage():
    #print(sys.version)
    print ("""© l'ATEJCON.
Programme de test de la classe NindFile.
Cette classe permet l'accès aux fichiers binaires avec codages Latejcon.
Nind_testLateconNumber a écrit dans /tmp/Nind_testLateconNumber.lat la
taille des données sur un entier 3 bits puis un nombre N sous 4 formes : 
1) N en non signé, 2) N en signé, 3) -N en signé, 4) -N en non signé.
Le programme relit ce fichier (ou un autre) et relit les 4 nombres
latecon sous 2 interprétations : 1) non signée, 2) signée"

usage   : %s <fichier codage latecon>
exemple : %s /tmp/Nind_testLateconNumber.lat
"""%(sys.argv[0], sys.argv[0]))

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    latFileName = os.path.abspath(sys.argv[1])
    
    latFile = NindFile(latFileName)
    #taille du fichier
    latFile.seek(0, 2)
    taille = latFile.tell()
    print ("taille du fichier : %d"%(taille))
    #lit la taille
    latFile.seek(0, 0)
    taille = latFile.litNombre3()
    print ("taille=%d"%(taille))
    nonSignes = []
    signes = []
    #lit les nombres en non signé 
    for i in range(4): nonSignes.append(latFile.litNombreULat())
    #lit les nombres en non signé 
    latFile.seek(3, 0)
    for i in range(4): signes.append(latFile.litNombreSLat())
    #affiche
    for i in range(4): print ("U: %d S: %d"%(nonSignes[i], signes[i]))
    
#calcule la clef A
def clefA(mot):
    mBytes = mot.encode('utf-8')
    clef = 0x55555555
    shifts = 0
    for octet in mBytes: 
        clef ^= (ord(octet) << shifts%24)
        shifts += 7
    return clef

#calcule la clef B
def clefB(mot):
    mBytes = mot.encode('utf-8')
    clef = 0x55555555
    shifts = 0
    for octet in mBytes: 
        clef ^= (octet << shifts%23)
        shifts += 5
    return clef

#donne la catégorie grammaticale en clair
def catNb2Str(cat):
    catList = ["", "ADJ", "ADV", "CONJ", "DET", "DETERMINEUR", "DIVERS", "DIVERS_DATE", "EXCLAMATION", "INTERJ", "NC", "NOMBRE", "NP", "PART", "PONCTU", "PREP", "PRON", "V", "DIVERS_PARTICULE", "CLASS", "AFFIX"]
    if cat >= len(catList): return ''
    return catList[cat]
######################################################################################
# <fichier>               ::= { <Entier1> | <Entier2> | <Entier3> | <Entier4> | <Entier5> | <EntierULat> | <EntierULat> |
#                              <MotUtf8> | <Utf8> | <Octet> }
# <MotUtf8>               ::= <longueur> <Utf8>
# <longueur>              ::= <Entier1>
# <Utf8>                  ::= { <Octet> }
# <Entier1>               ::= <Octet>
# <Entier2>               ::= <Octet> <Octet>
# <Entier3>               ::= <Octet> <Octet> <Octet>
# <Entier4>               ::= <Octet> <Octet> <Octet> <Octet>
# <Entier5>               ::= <Octet> <Octet> <Octet> <Octet> <Octet>
# <EntierULat>            ::= { <Octet> }
# <EntierSLat>            ::= { <Octet> }
######################################################################################
class NindFile:
    def __init__(self, latFileName, enEjcriture = False):
        self.latFileName = latFileName
        if not enEjcriture:
            #ouvre le fichier en lecture
            self.latFile = open(self.latFileName, 'rb')
            self.latFile.seek(0, 0)
        else:
            #ejcrit le fichier completement
            self.latFile = open(self.latFileName, 'wb')
        
    def seek(self, offset, from_what):
        self.latFile.seek(offset, from_what)
        
    def tell(self):
        return self.latFile.tell()
    
    def close(self):
        self.latFile.close()
        
    def litNombre1(self):
        return ord(self.latFile.read(1))

    def litNombre2(self):
        #gros-boutiste
        ba = bytes(self.latFile.read(2))
        return int( ba[0]*0x100 + ba[1] )
        #return (ba[0] <<8) + ba[1]

    def litNombre3(self):
        #petit-boutiste
        ba = bytes(self.latFile.read(3))
        return int( (ba[2]*0x100 + ba[1])*0x100 + ba[0] )
        #return (((ba[2] <<8) + ba[1]) <<8) + ba[0]
        #return (ba[2] <<16) + (ba[1] <<8) + ba[0]

    def litNombreS3(self):
        #petit-boutiste
        ba = bytes(self.latFile.read(3))
        res = (ba[2]*0x100 + ba[1])*0x100 + ba[0]
        #res = (((ba[2] <<8) + ba[1]) <<8) + ba[0]
        #res = (ba[2] <<16) + (ba[1] <<8) + ba[0]
        if res < 0x800000: return int(res) 
        return int( res - 0x1000000 )

    def litNombre4(self):
        #petit-boutiste
        ba = bytes(self.latFile.read(4))
        return int( ((ba[3]*0x100 + ba[2])*0x100 + ba[1])*0x100 + ba[0] )
        #return (((((ba[3] <<8) + ba[2]) <<8) + ba[1]) <<8) + ba[0]
        #return (ba[3] <<24) + (ba[2] <<16) + (ba[1] <<8) + ba[0]

    def litNombreS4(self):
        #petit-boutiste
        ba = bytes(self.latFile.read(4))
        res = ((ba[3]*0x100 + ba[2])*0x100 + ba[1])*0x100 + ba[0]
        if res < 0x80000000: return int(res) 
        return int(res - 0x100000000)

    def litNombre5(self):
        #gros-boutiste
        ba = bytes(self.latFile.read(5))
        return int( (((ba[0]*0x100 + ba[1])*0x100 + ba[2])*0x100 + ba[3])*0x100 + ba[4] )
        #return (((((((ba[0] <<8) + ba[1]) <<8) + ba[2]) <<8) + ba[3]) <<8) + ba[4]
        #return (ba[0] <<32) + (ba[1] <<24) + (ba[2] <<16) + (ba[3] <<8) + ba[4]

    def litNombreULat(self):
        octet = ord(self.latFile.read(1))
        if not octet&0x80: return octet
        result = ord(self.latFile.read(1))
        if not octet&0x40: return (octet&0x3F) * 0x100 + result
        result = result * 0x100 + ord(self.latFile.read(1))
        if not octet&0x20: return (octet&0x1F) * 0x10000 + result
        result = result * 0x100 + ord(self.latFile.read(1))
        if not octet&0x10: return (octet&0x0F) * 0x1000000 + result
        result = result * 0x100 + ord(self.latFile.read(1))
        if not octet&0x08: return result
        raise Exception('entier Ulatecon invalide à %08X'%(self.latFile.tell()))

    def litNombreSLat(self):
        octet = ord(self.latFile.read(1))
        if octet&0xC0 == 0x00: return octet
        if octet&0xC0 == 0x40: return octet - 0x80
        result = ord(self.latFile.read(1))
        if octet&0x60 == 0x00: return (octet&0x3F) * 0x100 + result
        if octet&0x60 == 0x20: return (octet&0x3F) * 0x100 + result - 0x4000
        result = result * 0x100 + ord(self.latFile.read(1))
        if octet&0x30 == 0x00: return (octet&0x1F) * 0x10000 + result
        if octet&0x30 == 0x10: return (octet&0x1F) * 0x10000 + result - 0x200000
        result = result * 0x100 + ord(self.latFile.read(1))
        if octet&0x18 == 0x00: return (octet&0x0F) * 0x1000000 + result
        if octet&0x18 == 0x08: return (octet&0x0F) * 0x1000000 + result - 0x10000000
        result = result * 0x100 + ord(self.latFile.read(1))
        if (octet&0x08 == 0x00) and (result&0x80000000 == 0x00000000): return result
        if (octet&0x08 == 0x00) and (result&0x80000000 == 0x80000000): return result - 0x0100000000
        raise Exception('entier Slatecon invalide à %08X'%(self.latFile.tell()))
    
    def litString(self):
        longueur = ord(self.latFile.read(1))
        #return self.latFile.read(longueur).decode('utf-8')
        return bytes(self.latFile.read(longueur)).decode()
    
    def litChaine(self, longueur):
        #return self.latFile.read(longueur).decode('utf-8')
        return bytes(self.latFile.read(longueur)).decode()
    
    def litOctets(self, longueur):
        return bytes(self.latFile.read(longueur))
    
    def ejcritNombre1(self, entier):
        ba = bytearray(1)
        ba[0] = entier&0xFF
        self.latFile.write(ba)

    def ejcritNombre3(self, entier):
        ba = bytearray(3)
        #petit-boutiste
        ba[0] = entier&0xFF
        ba[1] = (entier//0x100)&0xFF
        ba[2] = (entier//0x10000)&0xFF
        self.latFile.write(ba)
        
    def ejcritNombre4(self, entier):
        ba = bytearray(4)
        #petit-boutiste
        ba[0] = entier&0xFF
        ba[1] = (entier//0x100)&0xFF
        ba[2] = (entier//0x10000)&0xFF
        ba[3] = (entier//0x1000000)&0xFF
        self.latFile.write(ba)
        
    def ejcritNombre5(self, entier):
        ba = bytearray(5)
        #gros-boutiste
        ba[0] = (entier//0x100000000)&0xFF
        ba[1] = (entier//0x1000000)&0xFF
        ba[2] = (entier//0x10000)&0xFF
        ba[3] = (entier//0x100)&0xFF
        ba[4] = entier&0xFF
        self.latFile.write(ba)

    def ejcritChaine(self, chaine):
        self.latFile.write(chaine.encode('utf-8'))
        
    def ejcritZejros(self, taille):
        self.latFile.write(bytearray(taille))

       
if __name__ == '__main__':
    main()
       
