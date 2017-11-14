//
// C++ Interface: NindLocalAmose
//
// Description: L'adaptation de nind à amose
// voir "Adaptation de l'indexation nind au moteur de recherche Amose", LAT2015.JYS.448
// Cette classe gere les comptages necessaires a Amose ainsi que les caches pour les acces
// multiples au meme document du fichier des index locaux.
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
    *\param fileNameExtensionLess absolute path file name without extension
    *\param isLocalIndexWriter true if localIndex writer, false if localIndex reader  
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindLocalAmose(const std::string &fileNameExtensionLess,
                   const bool isLocalIndexWriter,
                   const Identification &lexiconIdentification,
                   const unsigned int indirectionBlocSize = 0);

    virtual ~NindLocalAmose();
    
    typedef std::vector<std::vector<std::list<Localisation> > > Positions;

    /** \brief Fill the data structure positions with position of occurrences 
    * of terms (from @ref termIds) in documents (from @ref documents)
    * \param termIds vector of identifier of terms
    * \param documents vector of documents where to search for position of terms. 
    * \param positions position of occurrences of terms in documents. One element foreach content  id in @ref documents. 
    * Each element is a vector containing one element for each term in termIds.
    * And each of these elements is the list of positions and lengths of the occurrences of this term in this document.  */
    void getTermPositionIndocs(const std::vector<unsigned int> &termIds, 
                               const std::vector<unsigned int> &documents, 
                               Positions &positions ); 
    
    /** \brief get the set of unique term in a document 
    * \param docId identifier of the document
    * \param termType type of terms (0: simple term, 1: multi-term, 2: named entity) 
    * \param termsSet set de termes uniques dans le document */
    bool getDocTerms(const unsigned int docId,
                     const AmoseTypes termType,
                     std::set<std::string> &termsSet);
 
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
