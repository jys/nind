//
// C++ Interface: NindLexiconIndex
//
// Description: La gestion du lexique sous forme de fichier index
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2017
//
// Copyright: 2014-2017 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindLexiconIndex_H
#define NindLexiconIndex_H
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
class NindRetrolexicon;
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between words and their indentifiant
*/
class DLLExportLexicon NindLexiconIndex : public NindIndex {
public:
    /**\brief Creates NindLexiconIndex.
    *\param fileNameExtensionLess absolute path file name without extension
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  
    *\param withRetrolexicon true if retro lexicon 
    *\param indirectionBlocSize number of entries in a lexicon single indirection block (for first writer only)
    *\param retroIndirectionBlocSize number of entries in a retro lexicon single indirection block (for first writer only)*/
    NindLexiconIndex(const std::string &fileNameExtensionLess,
                     const bool isLexiconWriter,
                     const bool withRetrolexicon = false,
                     const unsigned int indirectionBlocSize = 0,
                     const unsigned int retroIndirectionBlocSize = 0);

    virtual ~NindLexiconIndex();

    /**\brief add specified word in lexicon it doesn't still exist in,
     * In all cases, word ident is returned.
     * \param components list of components of a word 
     * (1 component = simple word, more components = compound word)
     * \return ident of word */
    unsigned int addWord(const std::list<std::string> &components);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param components list of components of a word 
     * (1 component = simple word, more components = compound word)
     * \return ident of word if word exists, 0 otherwise */
    unsigned int getWordId(const std::list<std::string> &components);

    /**\brief get identification of lexicon
     * \return unique identification of lexicon */
    Identification getIdentification();
    
    /**\brief get word components from the specified ident
    * if retro lexicon is not implanted, an exception is raised
    * \param ident ident of word
    * \param components list of components of a word 
    * (1 component = simple word, more components = compound word) 
    * \return true if word was found, false otherwise */
    bool getComponents(const unsigned int ident,
                       std::list<std::string> &components);
    
    /**\brief get retrolexicon file name
    * \return file name of retrolexicon */
    std::string getRetrolexiconFileName();
    
private:
    //<identifiantA> <identifiantRelC>
    struct Composej {
        unsigned int identA;
        unsigned int identComp;
        Composej(): identA(0), identComp(0) {}
        Composej(const unsigned int idA, const unsigned int idC): identA(idA), identComp(idC) {}
        ~Composej() {}
    };
    //<motSimple> <identifiantS> <nbreComposes> <composes>
    struct Mot {
        std::string motSimple;
        unsigned int identifiantS;
        std::list<Composej> composejs;
        Mot(): motSimple(), identifiantS(0), composejs() {}
        Mot(const std::string terS, const unsigned int idS): 
            motSimple(terS), identifiantS(idS), composejs() {}
        ~Mot() {}
    };
    
    //Recupere l'identifiant d'un mot sur le fichier lexique
    //retourne l'identifiant du mot s'il existe, 0 s'il n'existe pas
    unsigned int getIdentifiant(const std::string &mot,
                                const unsigned int sousMotId);

    //recupere les donnees de tous les mots qui ont la meme clef modulo 
    //les donnejes ont dejjah ejtej lue par getIdentifiant
    void getDefinitionWords(const std::string &motSimple,
                            std::list<Mot> &mots,
                            std::list<Mot>::iterator &motIt);
        
    //Ecrit les donnees de tous les mots qui ont la meme clef modulo 
    void setDefinitionWords(const std::list<Mot> &definition,
                             const Identification &lexiconIdentification);

    unsigned int m_modulo;              //pour trouver l'identifiant dans le fichier
    Identification m_identification;      //identification unique de ce lexique
    bool m_withRetrolexicon;            //avec ou sans retro lexique
    NindRetrolexicon *m_nindRetrolexicon;       //l'eventuel retro lexique
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



