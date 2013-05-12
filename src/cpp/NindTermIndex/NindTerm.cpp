//
// C++ Implementation: NindTerm
//
// Description: Le decodage et l'encodage des enregistrements du fichier inverse
// C'est une classe statique
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindTerm.h"
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definitionTerme>     ::= <longueur> { <definitionTermeCat> }
// <definitionTermeCat>  ::= <identTerme> <categorie> <frequence>
//                             <listeDocuments>
// <identTerme>          ::= <Integer4>
// <categorie>           ::= <Integer4>
// <frequence>           ::= <Integer4>
// <listeDocuments>      ::= <longueur> { <identDocument> }
// <longueur>            ::= <Integer4>
// <identDocument>       ::= <Integer4>
////////////////////////////////////////////////////////////
//brief Decode term index record
//param bytes address of bytes to decode
//param bytesNb number of bytes to decode
//param termCatResultList where to write decoded result */
void NindTerm::decode(unsigned char *bytes,
                      const unsigned int bytesNb,
                      list<NindTerm::TermCatResult> &termCatResultList)
    throw(DecodeErrorException)
{
    //raz resultat
    termCatResultList.clear();
    //decode
    unsigned char *ptr = bytes;
    unsigned char *endTermPtr = bytes + bytesNb;
    //verif taille globale
    unsigned int length;
    decodeInt4(ptr, length);
    if (ptr + length != endTermPtr) throw DecodeErrorException("term length");
    while (ptr != endTermPtr) {
        termCatResultList.push_back(NindTerm::TermCatResult());
        NindTerm::TermCatResult &termCatResult = termCatResultList.back();
        decodeInt4(ptr, termCatResult.termId);
        decodeInt4(ptr, termCatResult.category);
        decodeInt4(ptr, termCatResult.frequency);
        unsigned int length;
        decodeInt4(ptr, length);
        unsigned char *endTermCatPtr = ptr + length;
        if (endTermCatPtr > endTermPtr) throw DecodeErrorException("term cat length");
        while (ptr != endTermCatPtr) {
            unsigned int documentId;
            decodeInt4(ptr, documentId);
            termCatResult.documentsIdList.push_back(documentId);
        }
    }
}
////////////////////////////////////////////////////////////
//brief Encode term index record
//param bytes address of bytes where to encode
//param bytesNb number of bytes to encode
//param termCatResultList datas to encode */
void NindTerm::encode(unsigned char *bytes,
                      const unsigned int bytesNb,
                      const list<NindTerm::TermCatResult> &termCatResultList)

{
    unsigned char *ptr = bytes;
    //ecrit taille globale
    encodeInt4(ptr, encodedSize(termCatResultList) -4);     //<longueur>
    for (list<NindTerm::TermCatResult>::const_iterator it = termCatResultList.begin(); it != termCatResultList.end(); it++) {
        encodeInt4(ptr, it->termId);      //<identTerme>
        encodeInt4(ptr, it->category);    //<categorie>
        encodeInt4(ptr, it->frequency);   //<frequence>
        encodeInt4(ptr, it->documentsIdList.size() * 4);  //<longueur>
        for (list<unsigned int>::const_iterator it2 = it->documentsIdList.begin(); it2 != it->documentsIdList.end(); it2++) {
            encodeInt4(ptr, (*it2));
        }
    }
    if (ptr != bytes + bytesNb) throw EncodeErrorException("term length");
}
////////////////////////////////////////////////////////////
//brief Return size of encoded term index record
//param termCatResultList datas to encode
//return size of encoded record */
unsigned int NindTerm::encodedSize(const list<NindTerm::TermCatResult> &termCatResultList)
{
    unsigned int size = 4;    //<longueur>
    for (list<NindTerm::TermCatResult>::const_iterator it = termCatResultList.begin(); it != termCatResultList.end(); it++) {
        size += 16;           //<identTerme> <categorie> <frequence> <longueur>
        size += it->documentsIdList.size() * 4; //{ <identDocument> }
    }
    return size;
}
////////////////////////////////////////////////////////////
//brief Read a 4-bytes integer
//param ptr where to read integer
//param int4 4-bytes integer where to write read integer */
void NindTerm::decodeInt4(unsigned char *&ptr,
                          unsigned int &int4)
{
    //un entier, c'est 4 octets en little-endian quelle que soit la plate-forme
    int4 = ((ptr[3]*256 + ptr[2])*256 + ptr[1])*256 + ptr[0];
    ptr += 4;
}
////////////////////////////////////////////////////////////
//brief Write a 4-bytes integer 
//param ptr where to write integer
//param int4 4-bytes integer to write */
void NindTerm::encodeInt4(unsigned char *&ptr,
                          const unsigned int int4)
{
    //un entier, c'est 4 octets en little-endian quelle que soit la plate-forme
    ptr[0]=(int4)&0xFF;
    ptr[1]=(int4>>8)&0xFF;
    ptr[2]=(int4>>16)&0xFF;
    ptr[3]=(int4>>24)&0xFF;
    ptr += 4;
}
////////////////////////////////////////////////////////////
