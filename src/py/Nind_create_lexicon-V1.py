#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import os
import codecs
import StringIO
import Lexicon

def usage():
    print """
À partir d'une base de données Firebird au format LIC2M, 
extrait des blobs de la table LEXICON et forme un nouveau
lexique écrit sur un fichier csv destinés à faire des
essais de performances.
Les identifiants des mots composés sont communs avec les 
identifiants des mots simples. 
Les blobs sont récupérés directement dans la base spécifiée
par son chemin complet.
Le fichier résultat est <nom de la base>-Nind-Lexique-V1.csv 
dans le répertoire courant.

usage   : %s <path base Firebird>
exemple : %s /mnt/extension/etudeIndex/index/fre-boxon.fdb 
"""%(sys.argv[0], sys.argv[0])

def main():
    if len(sys.argv) < 2 :
        usage()
        sys.exit()
    dbName = os.path.abspath(sys.argv[1])
    #nom du fichier de sortie
    outFileName = '%s-Nind-Lexique-V1.csv'%(os.path.basename(dbName))

    #etablit le lexique 
    lexicon = Lexicon.Lexicon(dbName, True)
    #sort la liste du lexique
    lexiconWords = lexicon.getLexicon()
    #on va les écrire en prenant les mots composes dans leur ordre d'identifiant
    #un mot compose a un identifiant > 100000000
    keys = lexiconWords.keys()
    keys.sort()
    print '%d entrées dans lexiconWords'%(len(keys))
    #stock de mots simples
    simpleWords = []
    #les 2 maps du lexique
    lexiconSW = {}
    lexiconCW = {}
    #les 2 maps inverses du lexique pour la verification
    reverseLexiconSW = {}
    reverseLexiconCW = {}
    #le compteur d'identifiants
    ident = 0
    #ecrit sur fichier
    outFile = codecs.open(outFileName, 'w', 'utf-8')
    for key in keys:
        #outFile.write('====="%s"\n'%(lexiconWords[key].decode('utf-8')))
        #on ne prend pas les mots simples tout seuls, on les stocke dans l'ordre
        if key < 100000000: 
            simpleWords.append(lexiconWords[key].decode('utf-8'))
        else:
            word = lexiconWords[key].decode('utf-8')
            componantsSW = word.split('#')
            for componantSW in componantsSW:
                #si le composant est deja enregistre en mot simple, raf
                if lexiconSW.has_key(componantSW): continue
                while True:
                    #normalement, la liste ne peut etre vide
                    newSW = simpleWords.pop(0)
                    ident +=1
                    lexiconSW[newSW] = ident
                    reverseLexiconSW[ident] = newSW
                    outFile.write('%d;"%s"\n'%(ident, newSW))
                    #met tous les mots simples stockes avant le composant desire
                    if newSW == componantSW: break
            #tous les composants sont dans lexiconSW
            #maintenant les mots composes
            head = lexiconSW[componantsSW[0]]
            for idx in range(1, len(componantsSW)):
                tail = lexiconSW[componantsSW[idx]]
                #si le composant est deja enregistre en mot compose, raf
                if lexiconCW.has_key((head, tail)):  
                    head = lexiconCW[(head, tail)]
                else:
                    #l'enregistre
                    ident +=1
                    lexiconCW[(head, tail)] = ident
                    reverseLexiconCW[ident] = (head, tail)
                    outFile.write('%d;(%d, %d)\n'%(ident, head, tail))
                    head = ident
    for simpleWord in simpleWords:
        ident +=1
        lexiconSW[simpleWord] = ident
        reverseLexiconSW[ident] = simpleWord
        outFile.write('%d;"%s"\n'%(ident, simpleWord))
    outFile.close()    
    
    #relit les maps et affiche les premiers et les derniers
    result = []
    for key in reverseLexiconSW.keys(): result.append((key, reverseLexiconSW[key]))
    for key in reverseLexiconCW.keys():
        componants = []
        (head, tail) = reverseLexiconCW[key]
        while True:
            componants.insert(0, reverseLexiconSW[tail])
            if reverseLexiconSW.has_key(head):
                componants.insert(0, reverseLexiconSW[head])
                break
            else:(head, tail) = reverseLexiconCW[head]
        result.append((key, '#'.join(componants)))
    #affiche le resultat
    print u'%d entrées dans le lexique'%(len(result))
    result.sort()
    if len(result) <= 20:
        for (key, word) in result: print '% 7d  %s'%(key, word)
    else:
        for (key, word) in result[:10]: print '% 7d  %s'%(key, word)
        print '...'
        for (key, word) in result[-10:]: print '% 7d  %s'%(key, word)
        
    
if __name__ == '__main__':
    main()
