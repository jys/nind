//
// C++ Interface: NindLocalAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme document du fichier des index locaux.
//
// Author: jys <jy.sage@orange.fr>, (C) LATECON 2015
//
// Copyright: See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#ifndef NindLocalAmose_H
#define NindLocalAmose_H
////////////////////////////////////////////////////////////
#include "NindIndex/NindLocalIndex.h"
#include "NindAmose/NindLexiconAmose.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <string>
#include <list>
#include <vector>
#include <set>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindLocalAmose : public NindLocalIndex {
public:

    /** \brief Creates NindLocalAmose with a specified name associated with.
    *\param fileName absolute path file name
    *\param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindLocalAmose(const std::string &fileName,
                   const bool isLocalIndexWriter,
                   const Identification &lexiconIdentification,
                   const unsigned int indirectionBlocSize = 0);

    virtual ~NindLocalAmose();

// /** 
// * @brief Fill the data structure positions with position of occurrences 
// *        of terms (from @ref termIds) in documents (from @ref documents) 
// * @param termIds: vector of identifier of terms 
// * @param documents: vector of documents where to search for position of terms. The vector must be sorted by content id. 
// * @param positions: position of occurrences of terms in documents. One element foreach content  id in @ref documents. 
// * Each element is a vector containing one element for each term in termIds.
// * And each of these elements is the list of positions and lengths of the occurrences of this term in this document. 
// */ 
// virtual void getTermPositionIndocs(
//     const std::vector<TermId>& termIds, 
//     const std::vector<Lima::CONTENT_ID>& documents, 
//     std::vector<std::vector<Lima::Common::Misc::PositionLengthList> >& positions ) const = 0; 
    /** \brief Fill the data structure positions with position of occurrences 
    * of terms (from @ref termIds) in documents (from @ref documents)
    * \param termIds vector of identifier of terms
    * \param documents vector of documents where to search for position of terms. 
    * \param positions position of occurrences of terms in documents. One element foreach content  id in @ref documents. 
    * Each element is a vector containing one element for each term in termIds.
    * And each of these elements is the list of positions and lengths of the occurrences of this term in this document.  */
    void getTermPositionIndocs(const std::vector<unsigned int>& termIds, 
                               const std::vector<unsigned int>& documents, 
                               std::vector<std::vector<std::list<Localisation> > >& positions ); 
    
// /** 
// * @brief get the set of unique term in a document 
// * @param cid: identifier of the document 
// * @param type: type of terms (simple term, multi-term, named entity) 
// * @return a pair of iterators pointing to 1) the first element of a set 
// * of terms and to 2) past the end of this set. 
// */ 
// virtual std::pair<DocTermsIterator, DocTermsIterator> getDocTerms(
//     Lima::CONTENT_ID cid, 
//     Lima::Common::BagOfWords::BoWType type) const = 0;*/ 
// 
// La bufferisation de la liste des termes par la classe NindLocalAmose serait possible 
// o si getDocTerms n'est pas réentrant (pas rappellej jusqu'ah ce que son rejsultat soit complehtement utilisej)
// o s'il existait une méthode pour libérer l'espace utilisé pour la liste de termes     

    /** \brief get the set of unique term in a document 
    * \param docId identifier of the document
    * \param termType type of terms (0: simple term, 1: multi-term, 2: named entity) 
    * \param termsSet set de termes uniques dans le document */
    bool getDocTerms(const unsigned int docId,
                     const unsigned int termType,
                     std::set<std::string> &termsSet);
 
// /** 
// * @brief get length of a document 
// * @return  an integer, the number of occurrences of terms 
// */ 
// virtual uint32_t getDocLength(
//     Lima::CONTENT_ID cid) const = 0; 
    /** \brief get length of a document
    *\return  an integer, the number of occurrences of terms    */
    unsigned int getDocLength(const unsigned int docId);
        
private:
    NindLexiconAmose m_nindLexicon;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
