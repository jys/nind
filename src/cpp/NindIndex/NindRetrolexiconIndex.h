//
// C++ Interface: NindRetrolexiconIndex
//
// Description: La gestion du lexique et du lexique inverse sous forme de fichiers index
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2016
//
// Copyright: 2014-2016 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindRetrolexiconIndex_H
#define NindRetrolexiconIndex_H
////////////////////////////////////////////////////////////
#include "NindIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between indentifiants and words
*/
class DLLExportLexicon NindRetrolexiconIndex : public NindIndex {
public:
    /**\brief Creates NindRetrolexiconIndex.
    *\param fileName absolute path file name. Lexicon is identified by its file name
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindRetrolexiconIndex(const std::string &fileName,
                          const bool isLexiconWriter,
                          const Identification &lexiconIdentification,
                          const unsigned int indirectionBlocSize = 0);

    virtual ~NindRetrolexiconIndex();

// <donneesMot>          ::= <motCompose> | <motSimple>
// <motCompose>          ::= <flagCompose> <identifiantA> <identifiantRelS>
// <motSimple>           ::= <flagSimple> <longueurMot> <motUtf8>
    struct RetroWord {
        unsigned int identifiant;
        std::string motSimple;
        unsigned int identifiantA;
        unsigned int identifiantS;
        RetroWord(): identifiant(0), motSimple(), identifiantA(0), identifiantS(0) {}
        RetroWord(const unsigned int id, const std::string terS): 
            identifiant(id), motSimple(terS), identifiantA(0), identifiantS(0) {}
        RetroWord(const unsigned int id, const unsigned int idA, const unsigned int idS): 
            identifiant(id), motSimple(), identifiantA(idA), identifiantS(idS) {}
        ~RetroWord() {}
    };

    /**\brief add a list of word idents in retro lexicon. If one of idents still exists, exception is raised
    * \param retroWords list of words definitions 
    * \param lexiconWordsNb number of words contained in lexicon 
    * \param lexiconIdentification unique identification of lexicon */
    void addRetroWords(const std::list<struct RetroWord> &retroWords,
                  const Identification &lexiconIdentification);
    
    /**\brief get word components from the specified ident
    * \param ident ident of word
    * \param components list of components of a word 
    * (1 component = simple word, more components = compound word) 
    * \return true if word was found, false otherwise */
    bool getComponents(const unsigned int ident,
                       std::list<std::string> &components);


private:
    //Recupere sur le fichier retro lexique la definition d'un mot specifie par son identifiant
    //ident identifiant du mot
    //retroWord structure ou est ecrite la definition
    //retourne true si le mot existe, sinon false
    bool getRetroWord(const unsigned int ident,
                    struct RetroWord &retroWord);
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////