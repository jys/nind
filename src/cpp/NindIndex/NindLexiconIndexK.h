//
// C++ Interface: NindLexiconIndexK
//
// Description: La gestion du lexique sous forme de fichier index avec les chaines representees par 2 clefs 32 bits
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
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
#ifndef NindLexiconIndexK_H
#define NindLexiconIndexK_H
////////////////////////////////////////////////////////////
#include "NindIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
//#include <boost/serialization/hash_map.hpp>
#include <hash_map>
#include <ostream>
#include <string>
#include <map>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between words and their indentifiant
*/
class DLLExportLexicon NindLexiconIndexK : public NindIndex {
public:
    /**\brief Creates NindLexiconIndexK.
    *\param fileName absolute path file name
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindLexiconIndexK(const std::string &fileName,
                     const bool isLexiconWriter,
                     const unsigned int indirectionBlocSize = 0)
        throw(NindIndexException);

    virtual ~NindLexiconIndexK();

    /**\brief add specified word in lexicon and return its ident if word still exists in lexicon,
     * else, word is created in lexicon
     * in both cases, word ident is returned.
     * \param components list of components of a word (1 component = simple word, more components = compound word)
     * \return ident of word */
    unsigned int addWord(const std::list<std::string> &components)
        throw(NindLexiconIndexException);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param components list of components of a word (1 component = simple word, more components = compound word)
     * \return ident of word */
    unsigned int getId(const std::list<std::string> &components)
        throw(NindLexiconIndexException);

    /**\brief get identification of lexicon
     * \param wordsNb where number of words contained in lexicon is returned
     * \param identification where unique identification of lexicon is returned */
    void getIdentification(unsigned int &wordsNb, unsigned int &identification);
    
private:
    //<identifiantA> <identifiantRelC>
    struct Compose {
        unsigned int identA;
        unsigned int identComp;
        Compose(): identA(0), identComp(0) {}
        Compose(const unsigned int idA, const unsigned int idC): identA(idA), identComp(idC) {}
        ~Compose() {}
    };
    //<clefA> <clefB> <identifiantS> <nbreComposes> <composes>
    struct Terme {
        unsigned int clefA;
        unsigned int clefB;
        unsigned int identifiantS;
        std::list<Compose> composes;
        Terme(): clefA(0), clefB(0), identifiantS(0), composes() {}
        Terme(const unsigned int clA, const unsigned int clB, const unsigned int idS): 
            clefA(clA), clefB(clB), identifiantS(idS), composes() {}
        ~Terme() {}
    };
    
    //Recupere l'identifiant d'un terme sur le fichier lexique
    //retourne l'identifiant du terme s'il existe, 0 s'il n'existe pas
    unsigned int getIdentifiant(const std::string &terme,
                                const unsigned int sousMotId)
        throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException);

    //recupere les donnees de tous les termes qui ont la meme clef modulo 
    //retourne l'identifiant du terme s'il existe, sinon retourne 0
    //si le terme n'existe pas, la structure retournée est valide, sinon elle ne l'est pas
    unsigned int getDefinitionTermes(const std::string &termeSimple,
                                     const unsigned int sousMotId,
                                     std::list<Terme> &termes,
                                     std::list<Terme>::iterator &termeIt)
        throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException);
        
    //Ecrit les donnees de tous les termes qui ont la meme clef modulo 
    void setDefinitionTermes(const std::list<Terme> &definition,
                             const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification);

    unsigned int m_modulo;              //pour trouver l'identifiant dans le fichier
    unsigned int m_currentId;           //identifiant courant
    unsigned int m_identification;      //identification unique de ce lexique
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



