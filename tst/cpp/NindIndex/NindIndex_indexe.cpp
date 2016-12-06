//
// C++ Implementation: NindIndex_indexe
//
// Description: Indexation en mode immediat ou en mode bufferise
// Étude de la représentation du fichier inversé et des index locaux LAT2014.JYS.440
//
// Cette classe gere les acces aux fichiers d'indexation en bufferisant l'acces au
// fichier inverse... ou pas.
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2016
//
// Copyright: 2014-2016 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindIndex_indexe.h"
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int cg,
                        const unsigned int noDoc,
                        list<NindTermIndex::TermCG> &termIndex); 
//Construit la definition d'un fichier pour l'index local
static void majLocal(const unsigned int id,
                     const unsigned int cg,
                     const unsigned int pos,
                     const unsigned int taille,
                     const list<string> &componants,
                     list<NindLocalIndex::Term> &localIndex);
////////////////////////////////////////////////////////////
//brief Creates NindIndex_indexe with specified names and parameters associated with.
//param lexiconFileName absolute path lexicon file name
//param termindexFileName absolute path term index file name
//param localindexFileName absolute path local index file name
//param lexiconEntryNb number of lexicon entries in a single indirection block
//param termindexEntryNb number of term index entries in a single indirection block
//param localindexEntryNb number of local index entries in a single indirection block
//param termBufferSize size of term buffer before indexation (0 means immediate indexation) 
//param timeControl 3=structure, 2=+lexiconindex, 1=+termindex, 0=+localindex = normal*/
NindIndex_indexe::NindIndex_indexe(const string &lexiconFileName,
                                   const string &termindexFileName,
                                   const string &localindexFileName,
                                   const unsigned int lexiconEntryNb,
                                   const unsigned int termindexEntryNb,
                                   const unsigned int localindexEntryNb,
                                   const unsigned int termBufferSize,
                                   const unsigned int timeControl ) :
    m_nindLexicon(lexiconFileName, true, lexiconEntryNb),
    m_nindTermindex(termindexFileName, true, NindIndex::Identification(0, 0, 0), termindexEntryNb),
    m_nindLocalindex(localindexFileName, true, NindIndex::Identification(0, 0, 0), localindexEntryNb),
    m_termBufferSize(termBufferSize),
    m_timeControl(timeControl),
    m_docIdent(0),
    m_lexiconAccessNb(0),
    m_termindexAccessNb(0),
    m_localindexAccessNb(0),
    m_localindex(),
    m_termBuffer()
{
}
////////////////////////////////////////////////////////////
NindIndex_indexe::~NindIndex_indexe()
{
}
////////////////////////////////////////////////////////////
//brief Initiates a new document, writes current document on local index file
//param docIdent identification of new document (0 means no more documents) */
void NindIndex_indexe::newDoc(const unsigned int docIdent)
{
    //ecrit le document courant sur le  fichier des index locaux, sauf la premiere fois
    if (m_docIdent != 0) {
        //recupere l'identification du lexique
        NindIndex::Identification identification;
        if (m_timeControl < 3) m_nindLexicon.getIdentification(identification);
        //ecrit la definition sur le fichier des index locaux
        if (m_timeControl < 1) m_nindLocalindex.setLocalIndex(m_docIdent, m_localindex, identification);
        //vide le lexique local
        m_localindex.clear();
        //incremente le compteur
        m_localindexAccessNb++;
    }
    //memorise l'identifiant du document
    m_docIdent = docIdent;
}
////////////////////////////////////////////////////////////
//brief Add a simple word or a composed word with its cg ans position into index files
//param componants word to index
//param cg categorie grammaticale
//param pos position into origine file 
//param size size into origine file */
void NindIndex_indexe::indexe(const list<string> &componants,
                              const unsigned int cg,
                              const unsigned int pos,
                              const unsigned int size)
{
    //recupere l'id du terme dans le lexique, l'ajoute eventuellement
    unsigned int id = 0;
    if (m_timeControl < 3) id = m_nindLexicon.addWord(componants);
    //incremente le compteur
    m_lexiconAccessNb++;
    //bufferise quelle que soit la config
    //on cree l'entree si elle n'existe pas, ca ne fait rien si elle existe deja
    m_termBuffer[id];
    //on cherche l'id dans la map, elle existe a coup sur
    const map<unsigned int, list<pair<unsigned int, unsigned int> > >::iterator it = m_termBuffer.find(id);
    //et on y met la cg et le no de doc 
    (*it).second.push_back(make_pair(cg, m_docIdent));
    //si la taille de la map excede la taille maxi, on indexe tout le buffer
    if (m_termBuffer.size() > m_termBufferSize) {
        flush();
    }
    //augmente l'index local 
    majLocal(id, cg, pos, size, componants, m_localindex);
}
////////////////////////////////////////////////////////////
//brief Flushes buffered terms on term index file */
void NindIndex_indexe::flush()
{
    //recupere l'identification du lexique
    NindIndex::Identification identification;
    if (m_timeControl < 3) m_nindLexicon.getIdentification(identification);
    for (map<unsigned int, list<pair<unsigned int, unsigned int> > >::const_iterator it2 = m_termBuffer.begin();
         it2 != m_termBuffer.end(); it2++) {
        const unsigned int id2 = (*it2).first;
        const list<pair<unsigned int, unsigned int> > &cgdocsList = (*it2).second;
        //recupere l'index inverse pour ce terme
        list<NindTermIndex::TermCG> termIndex;
        if (m_timeControl < 2) m_nindTermindex.getTermIndex(id2, termIndex);
        //si le terme n'existe pas encore, la liste reste vide
        for (list<pair<unsigned int, unsigned int> >::const_iterator it3 = cgdocsList.begin(); it3 != cgdocsList.end(); it3++) {
            const unsigned int cg3 = (*it3).first;
            const unsigned int docId = (*it3).second;
            //met a jour la definition du terme
            majInverse(cg3, docId, termIndex);
        }
        //ecrit sur le fichier inverse
        if (m_timeControl < 2) m_nindTermindex.setTermIndex(id2, termIndex, identification);
        //incremente le compteur
        m_termindexAccessNb++;
    }
    //raz la bufferisation
    m_termBuffer.clear();
}   
////////////////////////////////////////////////////////////
//brief Gets number of accesses on lexicon file
//return  number of accesses on lexicon file */
unsigned int NindIndex_indexe::lexiconAccessNb() const
{
    return m_lexiconAccessNb;
}
////////////////////////////////////////////////////////////
//brief Gets number of accesses on term index file
//return  number of accesses on term index file */
unsigned int NindIndex_indexe::termindexAccessNb() const
{
    return m_termindexAccessNb;
}
////////////////////////////////////////////////////////////
//brief Gets number of accesses on local index file
//return  number of accesses on local index file */
unsigned int NindIndex_indexe::localindexAccessNb() const
{
     return m_localindexAccessNb;
}
////////////////////////////////////////////////////////////
//met a jour une definition de fichier inverse
static void majInverse (const unsigned int cg,
                        const unsigned int noDoc,
                        list<NindTermIndex::TermCG> &termIndex) 
{
    list<NindTermIndex::TermCG>::iterator it1 = termIndex.begin(); 
    while (it1 != termIndex.end()) {
        if ((*it1).cg == cg) {
            //c'est la meme cg, on ajoute le doc 
            list<NindTermIndex::Document> &documents = (*it1).documents;
            //trouve la place dans la liste ordonnee
            list<NindTermIndex::Document>::iterator it2 = documents.begin(); 
            while (it2 != documents.end()) {
                //deja dans la liste, incremente la frequence
                if ((*it2).ident == noDoc) {
                    (*it2).frequency +=1;  
                    break;
                }
                //insere a l'interieur de la liste
                if ((*it2).ident > noDoc) {
                    documents.insert(it2, NindTermIndex::Document(noDoc, 1));
                    break;
                }
                it2++;
            }
            //si fin de liste, insere en fin
            if (it2 == documents.end()) documents.push_back(NindTermIndex::Document(noDoc, 1));
            //met a jour la frequence globale de la cg
            (*it1).frequency +=1;
            break;
        }
        it1++;
    }
    //si c'est une nouvelle cg, insere en fin de liste
    if (it1 == termIndex.end()) {
        termIndex.push_back(NindTermIndex::TermCG(cg, 1));
        NindTermIndex::TermCG &termCG = termIndex.back();
        list<NindTermIndex::Document> &documents = termCG.documents;
        documents.push_back(NindTermIndex::Document(noDoc, 1));
    }
}
////////////////////////////////////////////////////////////
//Construit la definition d'un fichier pour l'index local
static void majLocal(const unsigned int id,
                     const unsigned int cg,
                     const unsigned int pos,
                     const unsigned int taille,
                     const list<string> &componants,
                     list<NindLocalIndex::Term> &localIndex)
{
    //simulation de cas reel de termes en plusieurs parties (c'est tout a fait arbitraire)
    //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
    //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
    //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
    //autres : (pos, taille)
    localIndex.push_back(NindLocalIndex::Term(id, cg));
    NindLocalIndex::Term &term = localIndex.back();
    const unsigned int nbComposants =  componants.size();
    list<string>::const_iterator compIt = componants.begin();
    const string &firstComp = (*compIt++);
    const char firstChar = firstComp.front();
    if (nbComposants == 2 && firstChar == 'T') {
        //TAA#BB : (pos, len(TAA)), (pos+len(TAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA, lenBB));
    }
    else if (nbComposants == 3 && firstChar == 'k') {
        //kAA#BB#CC : (pos, len(kAA), (pos+len(kAA#BB), len(CC)), (pos+len(kAA), len(BB))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + 1, lenCC));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA, lenBB));        
    }
    else if (nbComposants == 4 && firstChar == 'B') {
        //BAA#BB#CC#DD : (pos-10, 10), (pos, len(BAA)), (pos+len(BAA#BB#CC), len(DD)), (pos+len(BAA#BB), len(CC))
        const unsigned int lenAA = firstComp.size();
        const unsigned int lenBB = (*compIt++).size();
        const unsigned int lenCC = (*compIt++).size();
        const unsigned int lenDD = (*compIt++).size();
        term.localisation.push_back(NindLocalIndex::Localisation(pos - 10, 10));
        term.localisation.push_back(NindLocalIndex::Localisation(pos, lenAA));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + lenCC + 2, lenDD));
        term.localisation.push_back(NindLocalIndex::Localisation(pos + lenAA + lenBB + 1, lenCC));
    }
    else 
        //autres : (pos, taille)
        term.localisation.push_back(NindLocalIndex::Localisation(pos, taille));
}
////////////////////////////////////////////////////////////



    