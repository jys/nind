//
// C++ Interface: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: 2014-2015 LATECON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindTermIndex_H
#define NindTermIndex_H
////////////////////////////////////////////////////////////
#include "NindIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermIndex : public NindIndex {
public:

    /**\brief Creates NindTermIndex with a specified name associated with.
    *\param fileName absolute path file name
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindTermIndex(const std::string &fileName,
                  const bool isTermIndexWriter,
                  const unsigned int lexiconWordsNb,
                  const unsigned int lexiconIdentification,
                  const unsigned int indirectionBlocSize = 0);

    virtual ~NindTermIndex();
    
    /**\brief Structures to hold datas of a term */
    struct Document {
        unsigned int ident;
        unsigned int frequency;
        Document(): ident(0), frequency(0) {}
        Document(const unsigned int id, const unsigned int freq): ident(id), frequency(freq) {}
        ~Document() {}
    };
    struct TermCG {
        unsigned char cg;
        unsigned int frequency;
        std::list<Document> documents;          //identifiants ordonnes par ordre ascendant
        TermCG(): cg(0), frequency(0), documents() {}
        TermCG(const unsigned char cat, const unsigned int freq): cg(cat), frequency(freq), documents() {}
        ~TermCG() {}
    };
    
    /**\brief Read a full termIndex as a list of structures
    *\param ident ident of term
    *\param termIndex structure to receive all datas of the specified term
    *\return true if term was found, false otherwise */
    bool getTermIndex(const unsigned int ident,
                      std::list<struct TermCG> &termIndex);

    /**\brief Write a full termIndex as a list of structures
    *\param ident ident of term
    *\param termIndex structure containing all datas of the specified term 
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon */
    void setTermIndex(const unsigned int ident,
                      const std::list<struct TermCG> &termIndex,
                      const unsigned int lexiconWordsNb,
                      const unsigned int lexiconIdentification);
        
private:
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
