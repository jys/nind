//
// C++ Interface: NindIndexTest
//
// Description: Utilitaires pour les tests du fichier inverse
// Étude de la représentation du fichier inversé et des index locaux LAT2014.JYS.440
//
// Cette classe gere les utilitaires necessaires aux programmes de tests
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindIndexTest_H
#define NindIndexTest_H
////////////////////////////////////////////////////////////
#include "NindExceptions.h"
#include "NindCommonExport.h"
#include <string>
#include <map>
#include <vector>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindIndexTest {
public:

    NindIndexTest();

    virtual ~NindIndexTest();
        
    /**\brief Get ident number of supplied cg string
    *\param cg grammatical category as string
    *\return ident number */
    unsigned char getCgIdent(const std::string &cg)
        throw(EncodeErrorException);

    /**\brief Get cg string of supplied ident number
    *\param ident ident number
    *\return grammatical category as string */
    std::string getCgStr(const unsigned char ident)
        throw(DecodeErrorException);
        
    /**\brief structure of a word */
    struct WordDesc {
        std::string word;
        std::string cg;
        unsigned int pos;
        WordDesc() : word(), cg(), pos(0) {}
        WordDesc(const std::string w, const std::string c, const unsigned int p) : word(w), cg(c), pos(p) {}
        ~WordDesc() {}
    };
        
    /**\brief Extract informations from line of dump file
     * \param dumpLine line from dump file
     * \param noDoc return document ident
     * \param wordsList return words, cg and pos*/
    static void getWords(const std::string &dumpLine, 
                         unsigned int &noDoc, 
                         std::list<struct WordDesc> &wordsList);
//                         std::list<std::pair<std::string, std::string> > &wordsList);
    
    /**\brief split words into single words
     * \param word composed word with "#"
     * \param simpleWords return list of single words */
    static void split(const std::string &word, 
                      std::list<std::string> &simpleWords);
private:
    std::vector<std::string> m_cgId2Str;
    std::map<std::string, unsigned char> m_cgStr2Id;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
