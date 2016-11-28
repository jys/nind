//
// C++ Interface: NindLexicon
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revB
// §6.6 Réduction du lexique à une version utile pour l'indexation et la recherche
// §6.7.3 Avec deux hash_maps
// Étude et maquette d'un lexique complet ANT2012.JYS.R357 revA
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
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
#ifndef NindLexicon_H
#define NindLexicon_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include "NindLexiconFile.h"
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
class DLLExportLexicon NindLexicon {
public:
    /**\brief Creates NindLexicon.
    *\param fileName absolute path file name
    *\param isLexiconWriter true if lexicon writer, false if lexicon reader  */
    NindLexicon(const std::string &fileName,
                const bool isLexiconWriter)
        throw(NindLexiconException);

    virtual ~NindLexicon();

    /**\brief add specified word in lexicon and return its ident if word still exists in lexicon,
     * else, word is created in lexicon
     * in both cases, word ident is returned.
     * \param components list of components of a word (1 component = simple word, more components = compound word)
     * \return ident of word */
    unsigned int addWord(const std::list<std::string> &components)
        throw(NindLexiconException);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param components list of components of a word (1 component = simple word, more components = compound word)
     * \return ident of word */
    unsigned int getId(const std::list<std::string> &components)
        throw(NindLexiconException);

    /**\brief get identification of lexicon
     * \param wordsNb where number of words contained in lexicon is returned
     * \param identification where unique identification of lexicon is returned */
    void getIdentification(unsigned int &wordsNb, unsigned int &identification);
    
    /**\brief structure to hold lexicon characterictics
     * swNb number of simple words
     * cwNb number of compound words */
    struct LexiconChar{
        bool isOk;
        unsigned int swNb;
        unsigned int cwNb;
        unsigned int wordsNb;
        unsigned int identification;
    };

    /**\brief check lexicon integrity and return counts and statistics
     * \param lexiconChar where lexicon characteristics are returned
     * \return true if integrity is ok, false elsewhere */
    bool integrityAndCounts(struct LexiconChar &lexiconChar);

     /**\brief dump full lexicon on specified ostream
     * \param out ostream where to dump */
     void dump(std::ostream &out);

private:
    //met a jour le lexique lecteur avec le fichier lexique
    void updateFromFile()
        throw(EofException, ReadFileException, InvalidFileException, OutReadBufferException);
    
#ifdef _MSC_VER
    struct HashString : public stdext::hash_compare<std::string> {
        size_t operator()(const  std::string &s) const;
        bool operator()(const std::string &s1, const std::string &s2) const;
    };
    typedef stdext::hash_map<
        const std::string,
        unsigned int,
        HashString > StringHashMap;
#else
    struct HashString {
        size_t operator()(const std::string &s) const;
    };
    struct EqualString {
        bool operator()(const std::string &s1, const std::string &s2) const;
    };
    typedef std::unordered_map<
        const std::string,
        unsigned int,
        HashString,
        EqualString > StringHashMap;
#endif
#ifdef _MSC_VER
    struct HashPair : public stdext::hash_compare<std::pair<unsigned int, unsigned int> > {
        size_t operator()(const std::pair<unsigned int, unsigned int> &p) const;
        bool operator()(const std::pair<unsigned int, unsigned int> &p1, const std::pair<unsigned int, unsigned int> &p2) const;
    };
    typedef stdext::hash_map<
        std::pair<unsigned int, unsigned int>,
        unsigned int,
        HashPair > PairHashMap;
#else
    struct HashPair {
        size_t operator()(const std::pair<unsigned int, unsigned int> &p) const;
    };
    struct EqualPair {
        bool operator()(const std::pair<unsigned int, unsigned int> &p1, const std::pair<unsigned int, unsigned int> &p2) const;
    };
    typedef std::unordered_map<
        std::pair<unsigned int, unsigned int>,
        unsigned int,
        HashPair,
        EqualPair > PairHashMap;
#endif

        
    bool m_isLexiconWriter;         //true sir lexique ecrivain, false si lexique lecteur
    std::string m_fileName;
    unsigned int m_currentId;       //identifiant courant
    unsigned int m_identification;  //identification unique de ce lexique
    StringHashMap m_lexiconSW;      //lexique memoire des mots simples
    PairHashMap m_lexiconCW;        //lexique memoire des mots composes
    NindLexiconFile m_lexiconFile;  //fichier lexique
    unsigned int m_nextRefreshTime; //timestamp du prochain refresh
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



