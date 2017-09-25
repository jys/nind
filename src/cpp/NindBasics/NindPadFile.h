//
// C++ Interface: NindPadFile
//
// Description: La gestion basique d'un fichier nind : correspondance ident -> donnejes et identification
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
#ifndef NindPadFile_H
#define NindPadFile_H
////////////////////////////////////////////////////////////
#include "NindFile.h"
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <string>
#include <list>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindPadFile {
    
public:
    /**\brief Structure to hold files identification */
    struct Identification {
        unsigned int lexiconWordsNb;
        unsigned int lexiconTime;
        unsigned int specificFileIdent;
        Identification(): lexiconWordsNb(0), lexiconTime(0), specificFileIdent(0) {}
        Identification(const unsigned int nb, const unsigned int id, const unsigned int spe): 
            lexiconWordsNb(nb), lexiconTime(id), specificFileIdent(spe) {}
        ~Identification() {}
        //specificFileIdent n'inervient pas dans les comparaisons
        bool operator==(const Identification &id2) const {
            return (this->lexiconWordsNb == id2.lexiconWordsNb && this->lexiconTime == id2.lexiconTime
                && this->specificFileIdent == id2.specificFileIdent); }
        bool operator!=(const Identification &id2) const {
            return (this->lexiconWordsNb != id2.lexiconWordsNb || this->lexiconTime != id2.lexiconTime
                && this->specificFileIdent != id2.specificFileIdent); }    
    };
protected:
#define FLAG_IDENTIFICATION 53
    //<flagIdentification>(1) <maxIdentifiant>(3) <identifieurUnique>(4) <identifieurSpecifique>(4) = 12
#define TAILLE_IDENTIFICATION 12
    //<flagIndexej=47>(1) <addrBlocSuivant>(5) <nombreIndex>(3) = 9
#define TETE_BLOC_INDEX 9

    /**\brief Creates NindPadFile with a specified name associated with.
    *\param fileName absolute path file name
    *\param isWriter true if writer, false if reader  
    *\param referenceIdentification unique identification for checking (if 0, no checks)
    *\param dataEntrySize size in bytes of one data entry (for neo writer)
    *\param dataEntriesBlocSize number of entries in a single block (for neo writer) */
    NindPadFile(const std::string &fileName,
                const bool isWriter,
                const Identification &referenceIdentification,
                const unsigned int dataEntrySize = 0,
                const unsigned int dataEntriesBlocSize = 0);

    virtual ~NindPadFile();
    
    /**\brief get position of specified entry (with reader synchronization)
    *\param ident ident of specified entry
    *\return offset into file of specified entry (0 if out of bounds)*/
    unsigned long int getEntryPos(const unsigned int ident);

    /**\brief add empty entries block 
    *\param fileIdentification unique identification of file */
    void addEntriesBlock(const Identification &fileIdentification);

    /**\brief get size of 1rst entries block
    *\return size of 1rst entries block */
    unsigned int getFirstEntriesBlockSize();
         
    //return l'identifiant maximum possible avec le systehme actuel d'indirection
    unsigned int getMaxIdent() const;

    /**\brief get identification of file
    *\param identification where unique identification of file is returned */
    void getFileIdentification(Identification &identification);
        
    //ejcrit l'identification du fichier ah l'adresse courente du fichier
    void addIdentification(const Identification &fileIdentification);
public:        
    NindFile m_file;                //pour l'ecrivain ou le lecteur
protected:
    std::string m_fileName;
    bool m_isWriter;                    //vrai si ejcrivain
    bool m_isExistingWriter;            //vrai si ejcrivain dejjah existant 
    std::list<std::pair<unsigned long int, unsigned int> > m_entriesBlocksMap;  //gestion des blocs d'entrejes
    
private:
    //retourne la position d'une entreje sans synchronisation
    unsigned long int getJustEntryPos(const unsigned int ident);
    
    //ejtablit la carte des blocs d'entrejes
    void mapEntriesBlocks();
    
    //verifie l'apairage avec la rejfejrence
    void checkIdentification(const Identification &referenceIdentification);

    unsigned int m_dataEntrySize;       //size in bytes of one data entry (for writer)
    unsigned int m_dataEntriesBlocSize;         //number of entries in a single block (for writer)
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
