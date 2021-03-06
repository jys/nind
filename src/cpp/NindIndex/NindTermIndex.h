//
// C++ Interface: NindTermIndex
//
// Description: La gestion du fichier inverse en fichier
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier inverse qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
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
#ifndef NindTermIndex_H
#define NindTermIndex_H
////////////////////////////////////////////////////////////
#include "NindIndex.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTermIndex : public NindIndex {
public:

    /**\brief Creates NindTermIndex with a specified name associated with.
    *\param fileNameExtensionLess absolute path file name without extension
    *\param isTermIndexWriter true if termIndex writer, false if termIndex reader  
    *\param specificsNumber number of specific unsigned int
    *\param lexiconIdentification unique identification of lexicon 
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindTermIndex(const std::string &fileNameExtensionLess,
                  const bool isTermIndexWriter,
                  const Identification &lexiconIdentification,
                  const unsigned int specificsNumber,
                  const unsigned int indirectionBlocSize = 0);

    virtual ~NindTermIndex();
    
    /**\brief Structures to hold datas of a term */
    struct Document {
        unsigned int ident;
        unsigned int frequency;
        Document(): ident(0), frequency(0) {}
        Document(const unsigned int id, const unsigned int freq): ident(id), frequency(freq) {}
        ~Document() {}
        bool operator< (const Document &doc2) { return (this->ident < doc2.ident); }
    };
    struct TermCG {
        unsigned char cg;
        unsigned int frequency;
        std::list<Document> documents;          //identifiants ordonnes par ordre ascendant
        TermCG(): cg(0), frequency(0), documents() {}
        TermCG(const unsigned char cat, const unsigned int freq): cg(cat), frequency(freq), documents() {}
        ~TermCG() {}
    };
    
    /**\brief Read a full term definition as a list of structures
    *\param ident ident of term
    *\param termDef structure to receive all datas of the specified term
    *\return true if term was found, false otherwise */
    bool getTermDef(const unsigned int ident,
                    std::list<struct TermCG> &termDef);

    /**\brief Read specifics as a list of words
    *\param specifics list to receive all specifics */
    void getSpecificWords(std::list<unsigned int> &specifics);

    /**\brief Write a full term definition as a list of structures
    *\param ident ident of term
    *\param termDef structure containing all datas of the specified term 
    *\param specifics list of specific unsigned int
    *\param fileIdentification unique identification of lexicon */
    void setTermDef(const unsigned int ident,
                    const std::list<struct TermCG> &termDef,
                    const Identification &fileIdentification,
                    const std::list<unsigned int> &specifics);
        
private:
    unsigned int m_specificsNumber;
    
    //brief write specifics footer and identification into write buffer
    //param specifics list of specific unsigned int
    //param fileIdentification unique identification of file */
    void writeSpecificsAndIdentification(const std::list<unsigned int> &specifics,
                                         const Identification &fileIdentification);

};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
