//
// C++ Interface: NindLexiconA
//
// Description: La gestion du lexique en memoire : solution 1 symétrique avec maps classiques
// Étude de la représentation des mots composés en mémoire ANT2012.JYS.R356 revA
// §6.2 Solution : mêmes identifiants pour les chaînes et les mots composés
//
// Cette classe donne la correspondance entre un mot et son identifiant
// utilise dans le moteur
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindLexiconA_H
#define NindLexiconA_H
////////////////////////////////////////////////////////////
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
class NindLexiconA {
public:
    /**\brief Creates NindLexicon. */
    NindLexiconA();

    virtual ~NindLexiconA();

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
    
    /**\brief get word of the specified ident
     * if ident exists in lexicon, corresponding word is returned (compound word separator is '#')
     * else, return empty string (empty string is not a valid word !)
     * \param id ident as number 
     * \param componants list of componants of the word (list size means 0 : unknown id, 1 : simple word, more : compound word) */
    void getWord(const unsigned int id, std::list<std::string> &componants) const;

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
    unsigned int m_currentId;
    std::map<std::string, unsigned int> m_lexiconSW;  //lexique des mots simples
    std::map<unsigned int, std::string> m_retrolexiconSW;  //lexique inverse des mots simples
    std::map<std::pair<unsigned int, unsigned int>, unsigned int > m_lexiconCW;  //lexique des mots composes
    std::map<unsigned int, std::pair<unsigned int, unsigned int> > m_retrolexiconCW;  //lexique inverse des mots composes
};
    } // end namespace
} // end namespace
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////

    

