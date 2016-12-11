//
// C++ Interface: NindLexiconAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les spejcificitejs du lexique d'Amose, particuliehrement les types de mots.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: See LICENCE.md file that comes with this distribution
////////////////////////////////////////////////////////////
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
/**\brief various types of Amose words */
enum AmoseTypes { SIMPLE_TERM = 0, MULTI_TERM, NAMED_ENTITY }; 

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
    
    /**\brief add specified word in lexicon if it doesn't still exist in,
    * In all cases, word ident is returned.
    * \param lemma word to be lexiced. Compound word is structured with "_"
    * \param type type of the words (0: simple word, 1: multi-word, 2: named entity) 
    * \param namedEntity type of named entity, eventually
    * \return ident of word */
    unsigned int addWord(const std::string &lemma,
                         const AmoseTypes type,
                         const std::string &namedEntity = "");

    /**\brief get ident of a specified word
    * if word exists in lexicon, its ident is returned
    * else, return 0 (0 is not a valid ident !)
    * \param lemma word to be searched. Compound word is structured with "_"
    * \param type type of the words (0: simple word, 1: multi-word, 2: named entity) 
    * \param namedEntity type of named entity, eventually
    * \return ident of word if word exists, 0 otherwise*/
    unsigned int getWordId(const std::string &lemma,
                           const AmoseTypes type,
                           const std::string &namedEntity = "");

    /**\brief get word components from a specified word id 
    * \param lemma word corresponding to word id. Compound word is structured with "_"
    * \param type type of the words (0: simple word, 1: multi-word, 2: named entity) 
    * \param namedEntity type of named entity, eventually
    * \return true if word exists, false otherwise */
    bool getWord(const unsigned int wordId,
                 std::string &lemma,
                 AmoseTypes &type,
                 std::string &namedEntity);
    
private:
    unsigned int m_semicolonIdent;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////
