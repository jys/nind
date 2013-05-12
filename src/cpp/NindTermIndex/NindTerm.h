//
// C++ Interface: NindTerm
//
// Description: Le decodage et l'encodage des enregistrements du fichier inverse
// C'est une classe statique
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindTerm_H
#define NindTerm_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include "NindExceptions.h"
#include <stdio.h>
#include <list>
////////////////////////////////////////////////////////////
namespace antinno {
    namespace nindex {
////////////////////////////////////////////////////////////
class DLLExportLexicon NindTerm {
public:

    /**\brief structure to hold termIndex results for a specific term ident and a specific category
     * category ident of macrocategory
     * frequency frequency of the term + category
     * documentsIdList list of document idents */
    struct TermCatResult {
        unsigned int termId;
        unsigned int category;
        unsigned int frequency;
        std::list<unsigned int> documentsIdList;
        TermCatResult(): termId(0), category(0), frequency(0), documentsIdList() {};
    };

    /**\brief Decode term index record
    *\param bytes address of bytes to decode
    *\param bytesNb number of bytes to decode
    *\param termCatResultList where to write decoded result */
    static void decode(unsigned char *bytes,
                       const unsigned int bytesNb,
                       std::list<TermCatResult> &termCatResultList)
        throw(DecodeErrorException);

     /**\brief Encode term index record
    *\param bytes address of bytes where to encode
    *\param bytesNb number of bytes to encode
    *\param termCatResultList datas to encode */
    static void encode(unsigned char *bytes,
                       const unsigned int bytesNb,
                       const std::list<TermCatResult> &termCatResultList);

     /**\brief Return size of encoded term index record
    *\param termCatResultList datas to encode
    *\return size of encoded record */
    static unsigned int encodedSize(const std::list<TermCatResult> &termCatResultList);

    /**\brief Read a 4-bytes integer 
    *\param ptr where to read integer
    *\param int4 4-bytes integer where to write read integer */
    static void decodeInt4(unsigned char *&ptr,
                           unsigned int &int4);

     /**\brief Write a 4-bytes integer
     *\param ptr where to write integer
     *\param int4 4-bytes integer to write */
    static void encodeInt4(unsigned char *&ptr,
                           const unsigned int int4);
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
