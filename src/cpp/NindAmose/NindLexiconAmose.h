//
// C++ Interface: NindLexiconAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les spejcificitejs du lexique d'Amose, particuliehrement les types de termes.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindLexiconAmose_H
#define NindLexiconAmose_H
////////////////////////////////////////////////////////////
#include "NindIndex/NindLexiconIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindLexiconAmose : public NindLexiconIndex {
public:

    /**\brief Creates NindLexiconIndex.
    *\param fileName absolute path file name. Lexicon is identified by its file name
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  
    *\param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
    *\param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
    NindLexiconAmose(const std::string &fileName,
                     const bool isLexiconWriter = false,
                     const unsigned int indirectionBlocSize = 0,
                     const unsigned int retroIndirectionBlocSize = 0);

    virtual ~NindLexiconAmose();
    
    /**\brief add specified term in lexicon if it doesn't still exist in,
    * In all cases, term ident is returned.
    * \param lemma word to be lexiced. Compound word is structured with "_"
    * \param type type of the terms (0: simple term, 1: multi-term, 2: named entity) 
    * \param namedEntity type of named entity, eventually
    * \return ident of word */
    unsigned int addTerm(const std::string &lemma,
                         const unsigned int type,
                         const std::string &namedEntity = "");

    /**\brief get term components from a specified term id 
    * \param lemma word corresponding to term id. Compound word is structured with "_"
    * \param type type of the terms (0: simple term, 1: multi-term, 2: named entity) 
    * \param namedEntity type of named entity, eventually
    * \return true if term exists, false otherwise */
    bool getTerm(const unsigned int termId,
                 std::string &lemma,
                 unsigned int type,
                 std::string &namedEntity);
    
};
////////////////////////////////////////////////////////////
/**\brief various types of Amose terms */
enum AmoseTypes { SIMPLE_TERM, MULTI_TERM, NAMED_ENTITY };
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////
