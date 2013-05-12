//
// C++ Interface: NindLexiconC
//
// Description: La gestion du lexique en memoire, version hash_map
//
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindLexiconC_H
#define NindLexiconC_H
////////////////////////////////////////////////////////////
#include <boost/serialization/hash_map.hpp>
#include <ostream>
#include <string>
#include <map>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between words and their indentifiant
*/
class NindLexiconC {
public:
    /**\brief Creates NindLexicon. */
    NindLexiconC();

    virtual ~NindLexiconC();

    /**\brief add specified word in lexicon and return its ident
     * if word still exists in lexicon,
     * else, word is created in lexicon
     * in both cases, word ident is returned.
     * \param word simple or compound word in string (compound word separator is '#') */
    unsigned int addWord(const std::string &word);

    /**\brief get ident of the specified word
     * if word exists in lexicon, its ident is returned
     * else, return 0 (0 is not a valid ident !)
     * \param word simple or compound word in string (compound word separator is '#') */
    unsigned int getId(const std::string &word) const;

    /**\brief get word of the specified ident
     * if ident exists in lexicon, corresponding word is returned (compound word separator is '#')
     * else, return empty string (empty string is not a valid word !)
     * \param id ident as number */
    std::string getWord(const unsigned int id) const;

    /**\brief structure to hold lexicon characterictics
     * swNb number of simple words
     * rswNb number of simple words
     * cwNb number of compound words
     * rcwNb number of compound words */
    struct LexiconSizes{
        unsigned int swNb;
        unsigned int rswNb;
        unsigned int cwNb;
        unsigned int rcwNb;
        unsigned int successCount;
        unsigned int failCount;
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

    struct HashPair {
        size_t operator()(const std::pair<unsigned int, unsigned int> &p) const;
    };
    struct EqualPair {
        bool operator()(const std::pair<unsigned int, unsigned int> &p1, const std::pair<unsigned int, unsigned int> &p2) const;
    };
    typedef __gnu_cxx::hash_map<
        std::pair<unsigned int, unsigned int>,
        unsigned int,
        HashPair,
        EqualPair > PairHashMap;

    unsigned int m_currentId;
    //std::map<std::string, unsigned int> m_lexiconSW;  //lexique inverse des mots simples
    StringHashMap m_lexiconSW;  //lexique inverse des mots simples
    std::map<unsigned int, std::string> m_retrolexiconSW;  //lexique des mots simples
    //std::map<std::pair<unsigned int, unsigned int>, unsigned int > m_lexiconCW;  //lexique inverse des mots composes
    PairHashMap m_lexiconCW;  //lexique inverse des mots composes
    std::map<unsigned int, std::pair<unsigned int, unsigned int> > m_retrolexiconCW;  //lexique des mots composes
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



