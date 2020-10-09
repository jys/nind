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
o Analyse un fichier nindtermindex du système nind et affiche les statistiques. 
o Peut dumper nindtermindex sur <fichier>-dump.txt
o Peut afficher les données correspondant à un terme spécifié par son identifiant
Le format du fichier est défini dans le document LAT2017.JYS.470.

usage   : %s <fichier> [ <analyse> | <dumpe> | <affiche> <ident> ]
exemple : %s FRE.nindtermindex 
exemple : %s FRE.nindtermindex dumpe
exemple : %s FRE.nindtermindex affi 186201
"""%(script, script, script, script))

OFF = "\033[m"
RED = "\033[1;31m"

def main():
    try:
        if len(sys.argv) < 2 : raise Exception()
        termindexFileName = path.abspath(sys.argv[1])
        action = 'analyse' 
        if len(sys.argv) > 2 : action = sys.argv[2]
        terme = 0
        if len(sys.argv) > 3 : terme = int(sys.argv[3])
        
        #la classe
        nindTermindex = NindTermindex(termindexFileName)
        if action.startswith('anal'): nindTermindex.analyseFichierTermindex(True)
        elif action.startswith('dump'):
            outFilename = termindexFileName + '-dump.txt'
            outFile = codecs.open(outFilename, 'w', 'utf-8')
            nbLignes = nindTermindex.dumpeFichier(outFile)
            outFile.close()
            print ('%d lignes écrites dans %s'%(nbLignes, outFilename))
        elif action.startswith('affi'):
            print (nindTermindex.afficheTerme(terme))
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
# <dejfinition>           ::= <flagDejfinition=17> <identifiantTerme>
#                             <longueurDonnejes> <donnejesTerme>
# <flagDejfinition=17>    ::= <Entier1>
# <identifiantTerme>      ::= <Entier3>
# <longueurDonnejes>      ::= <Entier3>
# <donnejesTerme>         ::= { <donnejesCG> }
# <donnejesCG>            ::= <flagCg=61> <catejgorie> <frejquenceTerme>
#                             <nbreDocs> <listeDocuments>
# <flagCg=61>             ::= <Entier1>
# <catejgorie>            ::= <Entier1>
# <frejquenceTerme>       ::= <EntierULat>
# <nbreDocs>              ::= <EntierULat>
# <listeDocuments>        ::= { <identDocRelatif> <frejquenceDoc> }
# <identDocRelatif>       ::= <EntierULat>
# <frejquenceDoc>         ::= <EntierULat>
##############################
# <spejcifique>           ::= { <valeur> }
# <valeur>                ::= <Entier4>
############################################################
FLAG_DEJFINITION = 17
FLAG_CG = 61
#<flagDejfinition=17>(1) <identifiantTerme>(4) <longueurDonnejes>(3) = 8
TAILLE_TESTE_DEJFINITION = 8
############################################################

class NindTermindex(NindIndex):
    def __init__(self, termindexFileName):
        NindIndex.__init__(self, termindexFileName)

    #trouve les donnejes 
    def __donneDonnejes(self, identifiant):
        #lit la définition du mot
        (offsetDejfinition, longueurDejfinition) = self.donneAdresseDejfinition(identifiant)
        if offsetDejfinition == 0: return False, 0, 0      #identifiant pas trouve
        self.seek(offsetDejfinition, 0)
        #<flagDejfinition=17> <identifiantTerme> <longueurDonnejes>
        if self.litNombre1() != FLAG_DEJFINITION: 
            raise Exception('NindTermindex.__donneDonnejes %s : pas FLAG_DEJFINITION à %08X'%(self.latFileName, offsetDejfinition))
        if self.litNombre4() != identifiant: 
            raise Exception('NindTermindex.__donneDonnejes %s : %d pas trouvé à %08X'%(self.latFileName, index, offsetDejfinition +1))
        longueurDonnejes = self.litNombre3()
        tailleExtension = longueurDejfinition - longueurDonnejes - TAILLE_TESTE_DEJFINITION
        if tailleExtension < 0:
            raise Exception('NindTermindex.__donneDonnejes %s : %d incohérent à %08X'%(self.latFileName, index, offsetDejfinition +5))
        return True, longueurDonnejes, tailleExtension

    #retourne la structure dejcrivant le fichier inversej pour ce terme
    def donneListeTermesCG(self, ident):
        #trouve les donnejes 
        trouvej, longueurDonnejes, tailleExtension = self.__donneDonnejes(ident)
        if not trouvej: return []          #terme inconnu
        finDonnejes = self.tell() + longueurDonnejes
        #lit les donnes
        resultat = []
        while self.tell() < finDonnejes:
            #<flagCg=61> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
            if self.litNombre1() != FLAG_CG: 
                raise Exception('NindTermindex.donneListeTermesCG %s : pas FLAG_CG'%(self.latFileName))
            catejgorie = self.litNombre1()
            frejquenceTerme = self.litNombreULat()
            nbreDocs = self.litNombreULat()
            noDoc = 0
            docs = []
            for i in range(nbreDocs):
                #<identDocRelatif> <frejquenceDoc>
                identDocRelatif = self.litNombreULat()
                frejquenceDoc = self.litNombreULat()
                noDoc += identDocRelatif
                docs.append((noDoc, frejquenceDoc))
            resultat.append((catejgorie, frejquenceTerme, docs))
        return resultat
 
    #######################################################################"
    #analyse du fichier
    def analyseFichierTermindex(self, trace):
        cestbon = self.analyseFichierIndex(trace)
        if not cestbon: return False
        if trace: print ("======TERMINDEX=======")
        try:
            #trouve le max des identifiants
            maxIdent = self.donneMaxIdentifiant()
            totalDonnejes = totalExtensions = 0
            occurrencesDoc = 0
            nbDonnejes = nbExtensions = 0
            nbHapax = 0
            noDocSet = set()
            frejquences = []
            for identifiant in range(maxIdent):
                try:
                    #print ("identifiant=", identifiant)
                    #trouve les donnejes 
                    trouvej, longueurDonnejes, tailleExtension = self.__donneDonnejes(identifiant)
                    if not trouvej: continue      #identifiant pas trouve
                    nbDonnejes +=1
                    totalDonnejes += longueurDonnejes + TAILLE_TESTE_DEJFINITION
                    if tailleExtension > 0: nbExtensions += 1
                    totalExtensions += tailleExtension
                    #examine les données
                    finDonnejes = self.tell() + longueurDonnejes
                    while self.tell() < finDonnejes:
                        #<flagCg=61> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
                        if self.litNombre1() != FLAG_CG: 
                            raise Exception('NindTermindex.analyseFichierTermindex %s : pas FLAG_CG'%(self.latFileName))
                        catejgorie = self.litNombre1()
                        frejquenceTerme = self.litNombreULat()
                        nbreDocs = self.litNombreULat()
                        totalFrejquences = 0
                        noDoc = 0
                        for i in range(nbreDocs):
                            #<identDocRelatif> <frejquenceDoc>
                            identDocRelatif = self.litNombreULat()
                            frejquenceDoc = self.litNombreULat()
                            totalFrejquences += frejquenceDoc
                            noDoc += identDocRelatif
                            noDocSet.add(noDoc)
                        if totalFrejquences != frejquenceTerme: 
                            raise Exception('%s : fréquences incompatibles sur terme %d'%(self.latFileName, identifiant))
                        frejquences.append(frejquenceTerme)
                        occurrencesDoc += nbreDocs 
                        if totalFrejquences == 1: nbHapax +=1
                except:
                    if trace: print ('*******ERREUR SUR IDENTIFIANT :', identifiant)
                    raise                    
            if trace:
                nbTermesCG, frejquenceMin, frejquenceMax, totalFrejquences, moyenne, ejcartType = calculeRejpartition(frejquences)
                total = totalDonnejes + totalExtensions
                print ("DONNÉES        % 10d (%6.2f %%) % 9d occurrences"%(totalDonnejes, float(100)*totalDonnejes/total, nbDonnejes))
                print ("EXTENSIONS     % 10d (%6.2f %%) % 9d occurrences"%(totalExtensions, float(100)*totalExtensions/total, nbExtensions))
                print ("TOTAL          % 10d %08X"%(total, total))
                print ("=============")
                print ("TERMES-CG      % 10d"%(nbTermesCG))
                print ("FRÉQUENCE MIN  % 10d"%(frejquenceMin))
                print ("FRÉQUENCE MAX  % 10d"%(frejquenceMax))
                print ("TOTAL FRÉQUENCE% 10d"%(totalFrejquences))
                print ("MOYENNE        % 10d"%(moyenne))
                print ("ÉCART TYPE     % 10d"%(ejcartType))
                print ("=============")
                print ("DOCUMENTS      % 10d "%(len(noDocSet)))
                print ("TERMES-DOCS    % 10d occurrences"%(occurrencesDoc))
                print ("TERMES         % 10d occurrences"%(totalFrejquences))
                print ("HAPAX          % 10d (%6.2f %%)"%(nbHapax, float(100)*nbHapax/nbDonnejes))
                print ("=============")
                print ("%0.2f octets / occurrence de terme-doc"%(float(self.donneTailleFichier())/occurrencesDoc))
                print ("%0.2f octets / occurrence de terme"%(float(self.donneTailleFichier())/totalFrejquences))
        except Exception as exc: 
            cestBon = False
            if trace: print ('ERREUR :', exc.args[0])
            
        try:
            #rejcupehre l'adresse et la longueur des spejcifiques 
            (offsetSpejcifiques, tailleSpejcifiques) = self.donneSpejcifiques()
            #doit estre un multiple de 4
            if tailleSpejcifiques//4*4 != tailleSpejcifiques:
                raise Exception('%s : taille incompatible des spécifiques (doit être multiple de 4)'%(self.latFileName))
            self.seek(offsetSpejcifiques, 0)
            spejcifiques = []
            for i in range(tailleSpejcifiques//4): spejcifiques.append('%d'%(self.litNombre4()))
            if trace:
                print ("=============")
                print ("%d mots de données spécifiques"%(tailleSpejcifiques//4))
                print (', '.join(spejcifiques))
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
            trouvej, longueurDonnejes, tailleExtension = self.__donneDonnejes(identifiant)
            if not trouvej: continue      #identifiant pas trouve
            outFile.write('%06d:\n'%(identifiant))
            frejquenceGlobale = 0
            nbLignes +=1
            #examine les données
            finDonnejes = self.tell() + longueurDonnejes
            while self.tell() < finDonnejes:
                #<flagCg=61> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
                if self.litNombre1() != FLAG_CG: 
                    raise Exception('NindTermindex.dumpeFichier %s : pas FLAG_CG'%(self.latFileName))
                catejgorie = self.litNombre1()
                frejquenceTerme = self.litNombreULat()
                nbreDocs = self.litNombreULat()
                outFile.write('[%d] (%d) <%d>'%(catejgorie, frejquenceTerme, nbreDocs))
                frejquenceGlobale += frejquenceTerme
                totalFrequences = 0
                noDoc = 0
                docList = []
                for i in range(nbreDocs):
                    #<identDocRelatif> <frejquenceDoc>
                    incrementIdentDoc = self.litNombreULat()
                    frejquenceDoc = self.litNombreULat()
                    totalFrequences += frejquenceDoc
                    noDoc += incrementIdentDoc
                    docList.append('%05d (%d)'%(noDoc, frejquenceDoc))
                outFile.write(' :: %s\n'%(', '.join(docList)))
                if totalFrequences != frejquenceTerme: raise Exception('fréquences incompatibles sur terme %d'%(identifiant))
            outFile.write('frequence totale de %06d : %d\n'%(identifiant, frejquenceGlobale))
            outFile.write('\n')
        return nbLignes

    #######################################################################
    #dejcode les donnejes associejes ah un terme
    def afficheTerme(self, identifiant):
        rejsultat = StringIO()
        rejsultat.write('%06d:  '%(identifiant))
        #trouve les donnejes 
        trouvej, longueurDonnejes, tailleExtension = self.__donneDonnejes(identifiant)
        if not trouvej: return '%d : inconnu'%(identifiant)
        frejquenceGlobale = 0
        #examine les données
        finDonnejes = self.tell() + longueurDonnejes
        while self.tell() < finDonnejes:
            #<flagCg=61> <catejgorie> <frejquenceTerme> <nbreDocs> <listeDocuments>
            if self.litNombre1() != FLAG_CG: 
                raise Exception('NindTermindex.afficheTerme %s : pas FLAG_CG'%(self.latFileName))
            catejgorie = self.litNombre1()
            frejquenceTerme = self.litNombreULat()
            nbreDocs = self.litNombreULat()
            rejsultat.write('[%d] (%d) <%d>'%(catejgorie, frejquenceTerme, nbreDocs))
            frejquenceGlobale += frejquenceTerme
            totalFrequences = 0
            noDoc = 0
            docList = []
            for i in range(nbreDocs):
                #<identDocRelatif> <frejquenceDoc>
                incrementIdentDoc = self.litNombreULat()
                frejquenceDoc = self.litNombreULat()
                totalFrequences += frejquenceDoc
                noDoc += incrementIdentDoc
                docList.append('%05d (%d)'%(noDoc, frejquenceDoc))
            rejsultat.write(' :: %s\n'%(', '.join(docList)))
            if totalFrequences != frejquenceTerme: raise Exception('fréquences incompatibles sur terme %d'%(identifiant))
        return rejsultat.getvalue()
    #######################################################################
            
if __name__ == '__main__':
    main()
