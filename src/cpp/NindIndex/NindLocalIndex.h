//
// C++ Interface: NindLocalIndex
//
// Description: La gestion du fichier des index locaux
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier des index locaux qui doit rester coherent pour ses lecteurs
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
#ifndef NindLocalIndex_H
#define NindLocalIndex_H
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
class DLLExportLexicon NindLocalIndex : public NindIndex {
public:

    /**\brief Creates NindTermIndex with a specified name associated with.
    *\param fileName absolute path file name
    *\param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindLocalIndex(const std::string &fileName,
                   const bool isLocalIndexWriter,
                   const unsigned int lexiconWordsNb,
                   const unsigned int lexiconIdentification,
                   const unsigned int indirectionBlocSize = 0)
        throw(NindIndexException);

    virtual ~NindLocalIndex();
    
    /**\brief Structures to hold datas of a doc */
    struct Localisation {
        unsigned int position;
        unsigned int length;
        Localisation(): position(0), length(0) {}
        Localisation(const unsigned int loc, const unsigned int len): position(loc), length(len) {}
        ~Localisation() {}
    };
    struct Term {
        unsigned int term;
        unsigned char cg;
        std::list<Localisation> localisation; 
        Term(): term(0), cg(0), localisation() {}
        Term(const unsigned int ter, const unsigned char cat): term(ter), cg(cat), localisation() {}
        ~Term() {}
    };
    
    /**\brief Read a full document as a list of terms
    *\param ident ident of doc
    *\param localIndex structure to receive all datas of the specified doc
    *\return true if doc was found, false otherwise */
    bool getLocalIndex(const unsigned int ident,
                      std::list<struct Term> &localIndex)
        throw(NindLocalIndexException);

    /**\brief Write a full termIndex as a list of structures
    *\param ident ident of doc
    *\param localIndex structure containing all datas of the specified doc 
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon */
    void setLocalIndex(const unsigned int ident,
                      const std::list<struct Term> &localIndex,
                      const unsigned int lexiconWordsNb,
                      const unsigned int lexiconIdentification)
        throw(NindLocalIndexException);

private:
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
