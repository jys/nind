//
// C++ Interface: NindLexiconIndex
//
// Description: La gestion du lexique sous forme de fichier index
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
#ifndef NindLexiconIndex_H
#define NindLexiconIndex_H
////////////////////////////////////////////////////////////
#include "NindIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
//#include <boost/serialization/hash_map.hpp>
#include <unordered_map>
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
class DLLExportLexicon NindLexiconIndex : public NindIndex {
public:
    /**\brief Creates NindLexiconIndex.
    *\param fileName absolute path file name. Lexicon is identified by its file name
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindLexiconIndex(const std::string &fileName,
                     const bool isLexiconWriter,
                     const unsigned int indirectionBlocSize = 0)
        throw(NindIndexException);

    virtual ~NindLexiconIndex();

    /**\brief add specified term in lexicon it doesn't still exist in,
     * In all cases, word ident is returned.
     * \param componants list of componants of a word 
     * (1 componant = simple word, more componants = compound word)
     * \return ident of word */
    unsigned int addWord(const std::list<std::string> &componants)
        throw(NindLexiconIndexException);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param componants list of componants of a word 
     * (1 componant = simple word, more componants = compound word)
     * \return ident of word */
    unsigned int getId(const std::list<std::string> &componants)
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
    //<termeSimple> <identifiantS> <nbreComposes> <composes>
    struct Terme {
        std::string termeSimple;
        unsigned int identifiantS;
        std::list<Compose> composes;
        Terme(): termeSimple(), identifiantS(0), composes() {}
        Terme(const std::string terS, const unsigned int idS): 
            termeSimple(terS), identifiantS(idS), composes() {}
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



