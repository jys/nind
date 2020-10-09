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
import sys
from os import getenv, path
from io import StringIO
import codecs
from NindPadFile import calculeRejpartition
from NindIndex import NindIndex

def usage():
    if getenv("PY") != None: script = sys.argv[0].replace(getenv("PY"), '$PY')
    else: script = sys.argv[0]
    print ("""© l'ATEJCON.
o Analyse un fichier nindlocalindex du système nind et affiche les statistiques 
o Peut dumper nindlocalindex sur <fichier>-dump.txt
o Peut donner la liste des identifiants externes de tous les documents indexés
o Peut afficher les données correspondant à un document spécifié par son 
  identifiant externe
Le format du fichier est défini dans le document LAT2017.JYS.470.

usage   : %s <fichier> [ <analyse> | <dumpe> | <ident> | <affiche> <ident> ]
exemple : %s FRE.nindlocalindex
exemple : %s FRE.nindlocalindex dump
exemple : %s FRE.nindlocalindex ident
exemple : %s FRE.nindtermindex affi 3456
"""%(script, script, script, script, script))


def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        localindexFileName = path.abspath(sys.argv[1])
        action = 'analyse' 
        if len(sys.argv) > 2 : action = sys.argv[2]
        identExterne = 0
        if len(sys.argv) > 3 : identExterne = int(sys.argv[3])
        
        #la classe
        nindLocalindex = NindLocalindex(localindexFileName)
        if action.startswith('anal'): nindLocalindex.analyseFichierLocalindex(True)
        elif action.startswith('dump'):
            outFilename = localindexFileName + '-dump.txt'
            outFile = codecs.open(outFilename, 'w', 'utf-8')
            nbLignes = nindLocalindex.dumpeFichier(outFile)
            outFile.close()
            print ('%d lignes écrites dans %s'%(nbLignes, outFilename))
        elif action.startswith('id'):
            listeIdentifiants = nindLocalindex.donneidentifiantsExternes()
            listeIdentifiants.sort()
            #les affiche, un par ligne
            for (externe, interne) in listeIdentifiants: print (externe, ' <-> ', interne)
        elif action.startswith('affi'):
            print (nindLocalindex.afficheDocument(identExterne))
        else: raise Exception()
    except Exception as exc:
        if len(exc.args) == 0: usage()
        else:
            print ("******************************")
            print (exc.args[0])
            print ("******************************")
            raise
        sys.exit()

############################################################
# <dejfinition>           ::= <flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> <donnejesDoc>
# <flagDejfinition=19>    ::= <Entier1>
# <identifiantDoc>        ::= <Entier3>
# <identifiantExterne>    ::= <Entier4>
# <longueurDonnejes>      ::= <Entier3>
# <donnejesDoc>           ::= { <donnejesTerme> }
# <donnejesTerme>         ::= <identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
# <identTermeRelatif>     ::= <EntierSLat>
# <catejgorie>            ::= <Entier1>
# <nbreLocalisations>     ::= <Entier1>
# <localisations>         ::= { <localisationRelatif> <longueur> }
# <localisationRelatif>   ::= <EntierSLat>
# <longueur>              ::= <Entier1>
##############################
# <spejcifique>           ::= <maxIdentifiantInterne> <nombreDocuments>
# <maxIdentifiantInterne> ::= <Entier4>
# <nombreDocuments>       ::= <Entier4>
############################################################

FLAG_DEJFINITION = 19
#<maxIdentifiantInterne>(4) <nombreDocuments>(4) = 8
TAILLE_SPEJCIFIQUES = 8
#<flagDejfinition=19>(1) <identifiantDoc>(3) <identifiantExterne>(4) <longueurDonnejes>(3) = 11
TAILLE_TESTE_DEJFINITION = 11

class NindLocalindex(NindIndex):
    def __init__(self, localindexFileName):
        NindIndex.__init__(self, localindexFileName)
        #rejcupehre l'adresse et la longueur des spejcifiques 
        (offsetSpejcifiques, tailleSpejcifiques) = self.donneSpejcifiques()
        if tailleSpejcifiques != TAILLE_SPEJCIFIQUES: 
            raise Exception('%s : taille incompatible des spécifiques'%(self.latFileName))
        self.seek(offsetSpejcifiques, 0)
        #<maxIdentifiantInterne> <nombreDocuments>
        maxIdentifiantInterne = self.litNombre4()
        nombreDocuments = self.litNombre4()
        #initialisation traduction identitiant externe -> identifiant interne
        self.docIdTradExtInt = {}
        for noDocInterne in range(1, maxIdentifiantInterne +1):
            (offsetDejfinition, longueurDejfinition) = self.donneAdresseDejfinition(noDocInterne)
            if offsetDejfinition == 0: continue
            self.seek(offsetDejfinition, 0)
            #<flagDejfinition=19> <identifiantDoc> <identifiantExterne> 
            if self.litNombre1() != FLAG_DEJFINITION: 
                raise Exception('%s : pas FLAG_DEJFINITION à %08X'%(self.latFileName, offsetDejfinition))
            if self.litNombre3() != noDocInterne: 
                raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, ident, offsetDejfinition+1))
            noDocExterne = self.litNombre4()
            self.docIdTradExtInt[noDocExterne] = noDocInterne       
  
    #######################################################################
    #trouve les donnejes 
    def __donneDonnejes(self, identifiant):
        #lit la définition du mot
        (offsetDejfinition, longueurDejfinition) = self.donneAdresseDejfinition(identifiant)
        if offsetDejfinition == 0: return False, 0, 0, 0      #identifiant pas trouve
        self.seek(offsetDejfinition, 0)
        #<flagDejfinition=19> <identifiantDoc> <identifiantExterne> <longueurDonnejes> 
        if self.litNombre1() != FLAG_DEJFINITION: 
            raise Exception('%s : pas FLAG_DEJFINITION à %08X'%(self.latFileName, offsetDejfinition))
        if self.litNombre3() != identifiant: 
            raise Exception('%s : %d pas trouvé à %08X'%(self.latFileName, index, offsetDejfinition+1))
        identifiantExterne = self.litNombre4()
        longueurDonnejes = self.litNombre3()
        tailleExtension = longueurDejfinition - longueurDonnejes - TAILLE_TESTE_DEJFINITION
        if tailleExtension < 0:
            raise Exception('%s : %d incohérent à %08X'%(self.latFileName, identifiant, offsetDejfinition+5))
        return True, longueurDonnejes, tailleExtension, identifiantExterne

    #######################################################################
    #retourne la structure dejcrivant les localisations de termes pour le document spejcifiej
    def donneListeTermes(self, noDocExterne):
        #trouve le numejro interne
        if noDocExterne not in self.docIdTradExtInt: return []          #doc inconnu
        noDocInterne = self.docIdTradExtInt[noDocExterne]
        #trouve les donnejes 
        trouvej, longueurDonnejes, tailleExtension, identifiantExterne = self.__donneDonnejes(noDocInterne)
        if not trouvej: return []          #doc inconnu
        if identifiantExterne != noDocExterne: 
            raise Exception('%s : %d incohérent à %08X'%(self.latFileName, identifiant, self.tell()))
        finDonnejes = self.tell() + longueurDonnejes
        #lit les donnejes
        rejsultat = []
        noTerme = 0
        localisationAbsolue = 0
        while self.tell() < finDonnejes:
            #<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
            noTerme += self.litNombreSLat()
            catejgorie = self.litNombre1()
            nbreLocalisations = self.litNombre1()
            localisationsList = []
            for i in range (nbreLocalisations):
                #<localisationRelatif> <longueur>
                localisationAbsolue += self.litNombreSLat()
                longueur = self.litNombre1()
                localisationsList.append((localisationAbsolue, longueur))
            rejsultat.append((noTerme, catejgorie, localisationsList))
        return rejsultat
              
    #######################################################################
    #analyse du fichier
    def analyseFichierLocalindex(self, trace):
        cestbon = self.analyseFichierIndex(trace)
        if not cestbon: return False
        if trace: print ("======LOCALINDEX=======")
        try:
            #trouve le max des identifiants
            maxIdent = self.donneMaxIdentifiant()
            totalExtensions = nbExtensions = 0
            totalDonnejes = 0
            totalLocalisations = 0
            totalTermDoc = 0
            occurrences = []
            for identifiant in range(maxIdent):
                try:
                    trouvej, longueurDonnejes, tailleExtension, identifiantExterne = self.__donneDonnejes(identifiant)
                    if not trouvej: continue
                    if tailleExtension > 0: nbExtensions += 1
                    totalExtensions += tailleExtension
                    totalDonnejes += longueurDonnejes + TAILLE_TESTE_DEJFINITION
                    #examine les données
                    noTerme = localisationAbsolue = 0
                    nbOccurrences = 0
                    noTermSet = set()
                    finDonnejes = self.tell() + longueurDonnejes
                    while self.tell() < finDonnejes:
                        #<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
                        nbOccurrences +=1
                        noTerme += self.litNombreSLat()
                        noTermSet.add(noTerme)
                        catejgorie = self.litNombre1()
                        nbreLocalisations = self.litNombre1()
                        totalLocalisations += nbreLocalisations
                        for i in range (nbreLocalisations):
                            #<localisationRelatif> <longueur>
                            localisationAbsolue += self.litNombreSLat()
                            longueur = self.litNombre1()
                    occurrences.append(nbOccurrences)
                    totalTermDoc += len(noTermSet)
                except:
                    if trace: print ('*******ERREUR SUR IDENTIFIANT :', identifiant)
                    raise                                   
            if trace:
                nbDonnejes, occurrencesMin, occurrencesMax, totalOccurrences, moyenne, ejcartType = calculeRejpartition(occurrences)
                total = totalDonnejes + totalExtensions
                print ("DONNÉES        % 10d (%6.2f %%) % 9d occurrences"%(totalDonnejes, float(100)*totalDonnejes/total, nbDonnejes))
                print ("EXTENSIONS     % 10d (%6.2f %%) % 9d occurrences"%(totalExtensions, float(100)*totalExtensions/total, nbExtensions))
                print ("TOTAL          % 10d %08X"%(total, total))
                print ("=============")
                print ("DOCUMENTS      % 10d "%(nbDonnejes))
                print ("TERMES-DOCS    % 10d occurrences"%(totalTermDoc))
                print ("TERMES         % 10d occurrences"%(totalOccurrences))
                print ("LOCALISATIONS  % 10d occurrences"%(totalLocalisations))
                print ("=============")
                print ("DOCUMENT MAX   % 10d occurrences de termes"%(occurrencesMax))
                print ("DOCUMENT MIN   % 10d occurrences de termes"%(occurrencesMin))
                print ("MOYENNE        % 10d occurrences de termes"%(moyenne))
                print ("ÉCART-TYPE     % 10d occurrences de termes"%(ejcartType))
                print ("=============")
                print ("%0.2f octets / occurrence de terme"%(float(self.donneTailleFichier())/totalOccurrences))
                
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
            
        if trace: print ("=============")
        try:
            #rejcupehre l'adresse et la longueur des spejcifiques 
            (offsetSpejcifiques, tailleSpejcifiques) = self.donneSpejcifiques()
            if tailleSpejcifiques != TAILLE_SPEJCIFIQUES: 
                raise Exception('%s : taille incompatible des spécifiques'%(self.latFileName))
            self.seek(offsetSpejcifiques, 0)
            #<maxIdentifiantInterne> <nombreDocuments>
            maxIdentifiantInterne = self.litNombre4()
            nombreDocuments = self.litNombre4()
            if trace:
                print ("Max identifiant interne utilisé: %d"%(maxIdentifiantInterne))
                print ("Nombre de documents indexés    : %d"%(nombreDocuments))
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])

    #######################################################################
    #dumpe le fichier lexique sur un fichier texte
    def dumpeFichier(self, outFile):
        nbLignes = 0
        #trouve le max des identifiants
        maxIdent = self.donneMaxIdentifiant()
        for identifiant in range(maxIdent):
            #trouve les donnejes 
            trouvej, longueurDonnejes, tailleExtension, identifiantExterne = self.__donneDonnejes(identifiant)
            if not trouvej: continue      #identifiant pas trouve
            nbLignes +=1
            outFile.write('%07d/%07d:: '%(identifiant, identifiantExterne))
            #examine les données
            noTerme = localisationAbsolue = 0
            finDonnejes = self.tell() + longueurDonnejes
            while self.tell() < finDonnejes:
                #<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
                noTerme += self.litNombreSLat()
                catejgorie = self.litNombre1()
                nbreLocalisations = self.litNombre1()
                outFile.write('[%d](%d)'%(noTerme, catejgorie))
                localisationsList = []
                for i in range (nbreLocalisations):
                    #<localisationRelatif> <longueur>
                    localisationAbsolue += self.litNombreSLat()
                    longueur = self.litNombre1()
                    localisationsList.append('%d(%d)'%(localisationAbsolue, longueur))
                outFile.write('<%s> '%(','.join(localisationsList)))
            outFile.write('\n')
        return nbLignes

    #######################################################################
    def donneidentifiantsExternes(self):
        return list(self.docIdTradExtInt.items())

    #######################################################################
    #dejcode les donnejes associejes ah un terme
    def afficheDocument(self, noDocExterne):
        rejsultat = StringIO()
        #trouve l'identifiant interne
        if noDocExterne not in self.docIdTradExtInt: return '%d : inconnu'%(noDocExterne)
        noDocInterne = self.docIdTradExtInt[noDocExterne]
        rejsultat.write('%07d/%07d:: '%(noDocInterne, noDocExterne))
        #trouve les donnejes 
        trouvej, longueurDonnejes, tailleExtension, identifiantExterne = self.__donneDonnejes(noDocInterne)
        if not trouvej: longueurDonnejes = 0                #identifiant pas trouvej, document effacej
        #examine les données
        noTerme = localisationAbsolue = 0
        finDonnejes = self.tell() + longueurDonnejes
        while self.tell() < finDonnejes:
            #<identTermeRelatif> <catejgorie> <nbreLocalisations> <localisations>
            noTerme += self.litNombreSLat()
            catejgorie = self.litNombre1()
            nbreLocalisations = self.litNombre1()
            rejsultat.write('[%d](%d)'%(noTerme, catejgorie))
            localisationsList = []
            for i in range (nbreLocalisations):
                #<localisationRelatif> <longueur>
                localisationAbsolue += self.litNombreSLat()
                longueur = self.litNombre1()
                localisationsList.append('%d(%d)'%(localisationAbsolue, longueur))
            rejsultat.write('<%s> '%(','.join(localisationsList)))
        return rejsultat.getvalue()
    #######################################################################
        
if __name__ == '__main__':
    main()
