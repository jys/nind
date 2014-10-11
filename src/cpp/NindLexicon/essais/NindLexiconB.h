//
// C++ Interface: NindLexiconB
//
// Description: La gestion du lexique en memoire, solution alternative symétrique avec maps classiques
// VERSION ECARTEE
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.3 Solution alternative : identifiants différents pour les chaînes et les mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindLexiconB_H
#define NindLexiconB_H
////////////////////////////////////////////////////////////
#include <string>
#include <map>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
/**\brief This class maintains correspondance between words and their indentifiant
*/
class NindLexiconB {
public:
    /**\brief Creates NindLexiconB. */
    NindLexiconB();

    virtual ~NindLexiconB();

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

private:
    unsigned int m_SWcurrentId;
    unsigned int m_CWcurrentId;
    std::map<unsigned int, std::string> m_lexiconSW;  //lexique des mots simples
    std::map<std::string, unsigned int> m_retrolexiconSW;  //lexique inverse des mots simples
    std::map<unsigned int, std::pair<unsigned int, unsigned int> > m_lexiconCW;  //lexique des mots composes
    std::map<std::pair<unsigned int, unsigned int>, unsigned int > m_retrolexiconCW;  //lexique inverse des mots composes
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////



