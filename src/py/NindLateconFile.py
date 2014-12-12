#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os

def usage():
    print """© l'ATÉCON.
Programme de test de la classe NindLateconFile.
Cette classe permet l'accès aux fichiers binaires avec codages Latecon.
Nind_testLateconNumber a écrit dans /tmp/Nind_testLateconNumber.lat la
taille des données sur un entier 3 bits puis un nombre N sous 4 formes : 
1) N en non signé, 2) N en signé, 3) -N en signé, 4) -N en non signé.
Le programme relit ce fichier (ou un autre) et relit les 4 nombres
latecon sous 2 interprétations : 1) non signée, 2) signée"

usage   : %s <fichier codage latecon>
exemple : %s /tmp/Nind_testLateconNumber.lat
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    latFileName = os.path.abspath(sys.argv[1])
    
    latFile = NindLateconFile(latFileName)
    #taille du fichier
    latFile.seek(0, 2)
    taille = latFile.tell()
    print "taille du fichier : %d"%(taille)
    #lit la taille
    latFile.seek(0, 0)
    taille = latFile.litNombre3()
    print "taille=%d"%(taille)
    nonSignes = []
    signes = []
    #lit les nombres en non signé 
    for i in range(4): nonSignes.append(latFile.litNombreULat())
    #lit les nombres en non signé 
    latFile.seek(3, 0)
    for i in range(4): signes.append(latFile.litNombreSLat())
    #affiche
    for i in range(4): print "U: %d S: %d"%(nonSignes[i], signes[i])
    
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
        clef ^= (ord(octet) << shifts%23)
        shifts += 5
    return clef

#donne la catégorie grammaticale en clair
def catNb2Str(cat):
    catList = ["", "ADJ", "ADV", "CONJ", "DET", "DETERMINEUR", "DIVERS", "DIVERS_DATE", "EXCLAMATION", "INTERJ", "NC", "NOMBRE", "NP", "PART", "PONCTU", "PREP", "PRON", "V", "DIVERS_PARTICULE", "CLASS", "AFFIX"]
    if cat >= len(catList): return ''
    return catList[cat]

######################################################################################
class NindLateconFile:
    def __init__(self, latFileName, enEcriture = False):
        self.latFileName = latFileName
        if not enEcriture:
            #ouvre le fichier en lecture
            self.latFile = open(self.latFileName, 'rb')
            self.latFile.seek(0, 0)
        else:
            #ecrit le fichier completement
            self.latFile = open(self.latFileName, 'wb')
        
    def seek(self, offset, from_what):
        self.latFile.seek(offset, from_what)
        
    def tell(self):
        return self.latFile.tell()
    
    def close(self):
        self.latFile.close()
        
    def litNombre1(self):
        oc = self.latFile.read(1)
        return ord(oc)

    def litNombre2(self):
        #gros-boutiste
        oc = self.latFile.read(2)
        return ord(oc[0])*256 + ord(oc[1])

    def litNombre3(self):
        #petit-boutiste
        oc = self.latFile.read(3)
        return (ord(oc[2])*256 + ord(oc[1]))*256 + ord(oc[0])

    def litNombre4(self):
        #petit-boutiste
        oc = self.latFile.read(4)
        return ((ord(oc[3])*256 + ord(oc[2]))*256 + ord(oc[1]))*256 + ord(oc[0])

    def litNombre5(self):
        #gros-boutiste
        oc = self.latFile.read(5)
        return (((ord(oc[0])*256 + ord(oc[1]))*256 + ord(oc[2]))*256 + ord(oc[3]))*256 + ord(oc[4])

    def litNombreULat(self):
        octet = ord(self.latFile.read(1))
        if not octet&0x80: return octet
        result = ord(self.latFile.read(1))
        if not octet&0x40: return (octet&0x3F) * 256 + result
        result = result * 256 + ord(self.latFile.read(1))
        if not octet&0x20: return (octet&0x1F) * 256 * 256 + result
        result = result * 256 + ord(self.latFile.read(1))
        if not octet&0x10: return (octet&0x0F) * 256 * 256 * 256 + result
        result = result * 256 + ord(self.latFile.read(1))
        if not octet&0x08: return result
        raise Exception('entier Ulatecon invalide à %08X'%(self.latFile.tell()))

    def litNombreSLat(self):
        octet = ord(self.latFile.read(1))
        if octet&0xC0 == 0x00: return octet
        if octet&0xC0 == 0x40: return octet - 0x80
        result = ord(self.latFile.read(1))
        if octet&0x60 == 0x00: return (octet&0x3F) * 256 + result
        if octet&0x60 == 0x20: return (octet&0x3F) * 256 + result - 0x4000
        result = result * 256 + ord(self.latFile.read(1))
        if octet&0x30 == 0x00: return (octet&0x1F) * 256 * 256 + result
        if octet&0x30 == 0x10: return (octet&0x1F) * 256 * 256 + result - 0x200000
        result = result * 256 + ord(self.latFile.read(1))
        if octet&0x18 == 0x00: return (octet&0x0F) * 256 * 256 * 256 + result
        if octet&0x18 == 0x08: return (octet&0x0F) * 256 * 256 * 256 + result - 0x10000000
        result = result * 256 + ord(self.latFile.read(1))
        if (octet&0x08 == 0x00) and (result&0x80000000 == 0x00000000): return result
        if (octet&0x08 == 0x00) and (result&0x80000000 == 0x80000000): return result - 0x0100000000
        raise Exception('entier Slatecon invalide à %08X'%(self.latFile.tell()))
    
    def litString(self):
        longueur = ord(self.latFile.read(1))
        return self.latFile.read(longueur).decode('utf-8')
    
    def litChaine(self, longueur):
        return self.latFile.read(longueur).decode('utf-8')
    
    def ecritNombre1(self, entier):
        self.latFile.write(chr(entier&0xFF))

    def ecritNombre3(self, entier):
        #petit-boutiste
        self.latFile.write(chr(entier&0xFF))
        self.latFile.write(chr((entier/0x100)&0xFF))
        self.latFile.write(chr((entier/0x10000)&0xFF))
        
    def ecritNombre4(self, entier):
        #petit-boutiste
        self.latFile.write(chr(entier&0xFF))
        self.latFile.write(chr((entier/0x100)&0xFF))
        self.latFile.write(chr((entier/0x10000)&0xFF))
        self.latFile.write(chr((entier/0x1000000)&0xFF))
        
    def ecritNombre5(self, entier):
        #gros-boutiste
        self.latFile.write(chr((entier/0x100000000)&0xFF))
        self.latFile.write(chr((entier/0x1000000)&0xFF))
        self.latFile.write(chr((entier/0x10000)&0xFF))
        self.latFile.write(chr((entier/0x100)&0xFF))
        self.latFile.write(chr(entier&0xFF))

    def ecritChaine(self, chaine):
        self.latFile.write(chaine.encode('utf-8'))
        
if __name__ == '__main__':
    main()
       