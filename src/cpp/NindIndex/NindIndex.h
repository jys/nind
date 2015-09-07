//
// C++ Interface: NindIndex
//
// Description: La gestion d'un fichier (inverse ou d'index locaux)
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du fichier qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
// Cette classe est dérivable.
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
#ifndef NindIndex_H
#define NindIndex_H
////////////////////////////////////////////////////////////
#include "NindBasics/NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindIndex {
public:
    
    void dumpEmptyAreas();

protected:

    /**\brief Creates NindIndex with a specified name associated with.
    *\param fileName absolute path file name
    *\param isWriter true if writer, false if reader  
    *\param lexiconWordsNb number of words contained in lexicon  (if 0, no checks)
    *\param lexiconIdentification unique identification of lexicon  (if 0, no checks)
    *\param definitionMinimumSize size in bytes of the smallest definition
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindIndex(const std::string &fileName,
              const bool isWriter,
              const unsigned int lexiconWordsNb,
              const unsigned int lexiconIdentification,
              const unsigned int definitionMinimumSize = 0,
              const unsigned int indirectionBlocSize = 0)
        throw(NindIndexException);

    virtual ~NindIndex();
    
    /**\brief Read from file datas of specified definition and leave result into read buffer 
    *\param ident ident of definition
    *\return true if ident was found, false otherwise */
    bool getDefinition(const unsigned int ident)
        throw(EofException, ReadFileException, OutReadBufferException);
        
    /**\brief Write on file datas of specified definition yet constructed into write buffer
    *\param ident ident of definition
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon */
    void setDefinition(const unsigned int ident,
                       const unsigned int lexiconWordsNb,
                       const unsigned int lexiconIdentification)
        throw(EofException, WriteFileException, BadAllocException, OutWriteBufferException, 
              OutReadBufferException, OutOfBoundException);
        
    /**\brief check if indirection exists and create indirection block if necessary
    *\param ident ident of definition
    *\param lexiconWordsNb number of words contained in lexicon 
    *\param lexiconIdentification unique identification of lexicon */
    void checkExtendIndirection(const unsigned int ident,
                      const unsigned int lexiconWordsNb,
                      const unsigned int lexiconIdentification)
        throw(BadAllocException, OutWriteBufferException, WriteFileException, 
              OutOfBoundException);
        
    /**\brief get size of 1rst indirection block
    *\return size of 1rst indirection block */
    unsigned int getFirstIndirectionBlockSize()
    throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, 
          InvalidFileException);
    
    /**\brief get identification of lexicon
    *\param wordsNb where number of words contained in lexicon is returned
    *\param identification where unique identification of lexicon is returned */
    void getFileIdentification(unsigned int &wordsNb, unsigned int &identification) 
        throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, 
              InvalidFileException);
        
    NindFile m_file;                //pour l'ecrivain ou le lecteur
    std::string m_fileName;
    bool m_isWriter;

private:
    //etablit la carte des indirections  
    void mapIndirection()
        throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, InvalidFileException);
        
    //return l'offset de l'indirection de la definition specifiee, 0 si hors limite
    unsigned long int  getIndirection(const unsigned int ident);
    
    //ajoute un bloc d'indirection vide suivi d'une identification a la position courante du fichier
    void addIndirection(const unsigned int lexiconWordsNb,
                        const unsigned int lexiconIdentification)
        throw(BadAllocException, OutWriteBufferException, WriteFileException);
        
    //verifie l'apairage avec le lexique
    void checkIdentification(const unsigned int lexiconWordsNb,
                             const unsigned int lexiconIdentification)
        throw(OutReadBufferException, EofException, ReadFileException, BadAllocException, 
              InvalidFileException, IncompatibleFileException);


    unsigned int m_definitionMinimumSize;       //taille minimum admissible pour une definition
    unsigned int m_indirectionBlocSize;         //nbre d'entrees d'un bloc d'indirection
    std::list<std::pair<unsigned long int, unsigned int> > m_indirectionMapping;  //gestion des indirections
    std::list<std::pair<unsigned long int, unsigned int> > m_emptyAreas;         //gestion des zones libres
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
