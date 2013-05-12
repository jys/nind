//
// C++ Interface: NindLexiconE
//
// Description: La gestion du lexique en memoire : solution asymétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.6 Réduction du lexique à une version utile pour l'indexation et la recherche
// §6.7.2 Avec une hash_map et une map classique
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindLexiconE_H
#define NindLexiconE_H
////////////////////////////////////////////////////////////
#include <boost/serialization/hash_map.hpp>
#include <ostream>
#include <string>
#include <map>
#include <list>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between words and their indentifiant
*/
class NindLexiconE {
public:
    /**\brief Creates NindLexicon. */
    NindLexiconE();

    virtual ~NindLexiconE();

    /**\brief add specified word in lexicon and return its ident
     * if word still exists in lexicon,
     * else, word is created in lexicon
     * in both cases, word ident is returned.
     * \param componants list of componants of a word (1 componant = simple word, more componants = compound word)
     * \return ident of word */
    unsigned int addWord(const std::list<std::string> &componants);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param componants list of componants of a word (1 componant = simple word, more componants = compound word)
     * \return ident of word */
    unsigned int getId(const std::list<std::string> &componants) const;

    /**\brief structure to hold lexicon characterictics
     * swNb number of simple words
     * cwNb number of compound words */
    struct LexiconSizes{
        unsigned int swNb;
        unsigned int cwNb;
    };

    /**\brief check lexicon integrity and return counts and statistics
     * \param swNb where number of simple words is returned
     * \param cwNb where number of compound words is returned
     * \return true if integrity is ok, false elsewhere */
    bool integrityAndCounts(struct LexiconSizes &lexiconSizes) const;

     /**\brief dump full lexicon on specified ostream
     * \param out ostream where to dump */
     void dump(std::ostream &out);

private:
    struct HashString {
        size_t operator()(const std::string &s) const;
    };
    struct EqualString {
        bool operator()(const std::string &s1, const std::string &s2) const;
    };
    typedef __gnu_cxx::hash_map<
        std::string,
        unsigned int,
        HashString,
        EqualString > StringHashMap;

    unsigned int m_currentId;
    StringHashMap m_lexiconSW;  //lexique inverse des mots simples
    std::map<std::pair<unsigned int, unsigned int>, unsigned int > m_lexiconCW;  //lexique des mots composes
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



