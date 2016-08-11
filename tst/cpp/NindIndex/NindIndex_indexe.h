//
// C++ Interface: NindIndex_indexe
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
#ifndef NindIndex_indexe_H
#define NindIndex_indexe_H
////////////////////////////////////////////////////////////
#include "NindIndex/NindLexiconIndex.h"
#include "NindIndex/NindTermIndex.h"
#include "NindIndex/NindLocalIndex.h"
#include "NindExceptions.h"
#include "NindCommonExport.h"
#include <string>
#include <map>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindIndex_indexe {
public:

    /**\brief Creates NindIndex_indexe with specified names and parameters associated with.
    *\param lexiconFileName absolute path lexicon file name
    *\param termindexFileName absolute path term index file name
    *\param localindexFileName absolute path local index file name
    *\param lexiconEntryNb number of lexicon entries in a single indirection block
    *\param termindexEntryNb number of term index entries in a single indirection block
    *\param localindexEntryNb number of local index entries in a single indirection block
    *\param termBufferSize size of term buffer before indexation (0 means immediate indexation) */
    NindIndex_indexe(const std::string &lexiconFileName,
                     const std::string &termindexFileName,
                     const std::string &localindexFileName,
                     const unsigned int lexiconEntryNb,
                     const unsigned int termindexEntryNb,
                     const unsigned int localindexEntryNb,
                     const unsigned int termBufferSize = 0);

    virtual ~NindIndex_indexe();
    
    /**\brief Initiates a new document, writes current document on local index file
    *\param docIdent identification of new document (0 means no more documents) */
    void newDoc(const unsigned int docIdent);
    
    /**\brief Add a simple word or a composed word with its cg ans position into index files
    *\param componants word to index
    *\param cg categorie grammaticale
    *\param pos position into origine file
    *\param size size into origine file */
    void indexe(const std::list<std::string> &componants,
                const unsigned int cg,
                const unsigned int pos,
                const unsigned int size);
    
    /**\brief Flushes buffered terms on term index file */
    void flush();
    
    /**\brief Gets number of accesses on lexicon file
    *\return  number of accesses on lexicon file */
    unsigned int lexiconAccessNb() const;
    
    /**\brief Gets number of accesses on term index file
    *\return  number of accesses on term index file */
    unsigned int termindexAccessNb() const;
    
    /**\brief Gets number of accesses on local index file
    *\return  number of accesses on local index file */
    unsigned int localindexAccessNb() const;
    
 private:
    NindLexiconIndex m_nindLexicon;
    NindTermIndex m_nindTermindex;
    NindLocalIndex m_nindLocalindex;
    unsigned int m_termBufferSize;
    unsigned int m_docIdent;
    unsigned int m_lexiconAccessNb;
    unsigned int m_termindexAccessNb;
    unsigned int m_localindexAccessNb;
    std::list<NindLocalIndex::Term> m_localindex;
    std::map<unsigned int, std::list<std::pair<unsigned int, unsigned int> > > m_termBuffer;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
   


    