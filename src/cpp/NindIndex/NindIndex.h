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
#ifndef NindIndex_H
#define NindIndex_H
////////////////////////////////////////////////////////////
#include "NindBasics/NindPadFile.h"
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
class DLLExportLexicon NindIndex : public NindPadFile {
protected:
    /**\brief Creates NindIndex with a specified name associated with.
    *\param fileName absolute path file name
    *\param isWriter true if writer, false if reader  
    *\param lexiconIdentification unique identification of lexicon  (if 0, no checks)
    *\param specificsSize size in bytes of specific datas
    *\param definitionMinimumSize size in bytes of the smallest definition
    *\param indirectionBlocSize number of entries in a single indirection block */
    NindIndex(const std::string &fileName,
              const bool isWriter,
              const Identification &lexiconIdentification,
              const unsigned int specificsSize,
              const unsigned int definitionMinimumSize = 0,
              const unsigned int indirectionBlocSize = 0);

    virtual ~NindIndex();
    
    /**\brief Read from file datas of specified definition and leave result into read buffer 
    *\param ident ident of definition
    *\param bytesNb optional number of bytes to read, in case of partial read
    *\return true if ident was found, false otherwise */
    bool getDefinition(const unsigned int ident,
                       const unsigned int bytesNb = 0);
        
    /**\brief Write on file datas of specified definition yet constructed into write buffer
    * buffer contains datas of specified definition + specific datas + identification
    *\param ident ident of definition */
    void setDefinition(const unsigned int ident);
        
    /**\brief check if indirection exists and create indirection block if necessary
    *\param ident ident of definition
    *\param fileIdentification unique identification of file */
    void checkExtendIndirection(const unsigned int ident,
                                const Identification &fileIdentification);
    
private:
    //ejcrit une nouvelle indirection dans l'index
    void setIndirection(const unsigned long int indirection,
                        const unsigned long int offsetDejfinition,
                        const unsigned int longueurDejfinition);
    
    //ejtablit la carte des vides
    void mapEmptySpaces();
    
    //trouve une nouvelle zone pour les nouvelles donnejs
    //retourne true si l'identification est dejjah ejcrite
    bool findNewArea(const unsigned int dataSize,
                     unsigned long int &offsetDejfinition,
                     unsigned int &longueurDejfinition);
    
    //brief Place l'ancienne zone de donnejes dans la gestion du vide
    void vacateOldArea(const unsigned long int oldOffsetEntry,
                       const unsigned int oldLengthEntry);
    
    //dumpe la map des indirections (uniquement pour debogue)
    void dumpIndirection();

    unsigned int m_definitionMinimumSize;       //taille minimum admissible pour une definition
    std::list<std::pair<unsigned long int, unsigned int> > m_emptyAreas;         //gestion des zones libres
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
