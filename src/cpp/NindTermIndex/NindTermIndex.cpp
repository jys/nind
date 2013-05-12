//
// C++ Implementation: NindTermIndex
//
// Description: La gestion du fichier inversé
// Étude de la représentation du fichier inversé et des index locaux ANT2012.JYS.R358 revA
// N'importe quel jargonneux de la syntaxe C++ verra une invitation a mettre un niveau
// d'abstraction pour indexer n'importe quoi. Arriere, manants, seule compte l'algorithmique,
// la syntaxe C++, on s'en cogne.
//
// Cette classe ajoute une nouvelle correespondance entre un identifiant de terme et
// un identifiant de document. Elle donne aussi tous les identifiants de documents
// associés à un terme spécifique
//
// Author: Jean-Yves Sage <jean-yves.sage@antinno.fr>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindTermIndex.h"
using namespace antinno::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <fichierInverse>      ::= <identification> <donnees>
// <identification>      ::= <maxIdentifiant> <identifieurUnique>
// <maxIdentifiant>      ::= <Integer4>
// <identifieurUnique>   ::= <dateHeure>
// <dateHeure >          ::= <Integer4>
// <donnees>             ::= { <definitionTerme> }
// <definitionTerme>     ::= <longueur> { <definitionTermeCat> }
// <definitionTermeCat>  ::= <identTerme> <categorie> <frequence>
//                             <listeDocuments>
// <identTerme>          ::= <Integer4>
// <categorie>           ::= <Integer4>
// <frequence>           ::= <Integer4>
#define LOCATION_RESIZE 1000
////////////////////////////////////////////////////////////
// This class maintains correspondance between term idents and document idents
//brief Creates NindTermIndex.
//param fileName absolute path file name
//param isTermIndexWriter true if termIndex writer, false if termIndex reader  */
NindTermIndex::NindTermIndex(const std::string &fileName,
                             const bool isTermIndexWriter)
    throw(OpenFileException, EofException, ReadFileException, WriteFileException, InvalidFileException):
    m_isTermIndexWriter(isTermIndexWriter),
    m_fileName(fileName),
    m_file(fileName, isTermIndexWriter),
    m_termLocations(),
    m_freeLocations()
{
    if (m_isTermIndexWriter) {
        //si TermIndex ecrivain, ouvre en lecture + ecriture
        bool isOpened = m_file.open("r+b");
        //si fichier absent, cree un fichier vide en ecriture + lecture
        if (!isOpened) {
            isOpened = m_file.open("w+b");
            if (!isOpened) throw OpenFileException(m_fileName);
            //lui colle une identification bidon pour uniformiser les cas
            m_file.writeInt4(0);
            m_file.writeInt4((time_t)time(NULL));  //date de creation du fichier
            m_file.write();
        }
    }
    else {
        //si lexique memoire lecteur, ouvre en lecture seule
        bool isOpened = m_file.open("rb");
        if (!isOpened) throw OpenFileException(m_fileName);
    }
    //lit le fichier et initialise m_termLocations et m_freeLocations
    m_file.setPos(0, SEEK_SET);
    unsigned int maxIdent;
    m_file.readInt4(maxIdent);  //<maxIdentifiant>
    //donne la taille du vecteur
    const RecordLocation nullRecordLocation(0, 0);
    m_termLocations.resize(maxIdent, nullRecordLocation);
    m_file.setPos(4, SEEK_CUR);   //saute <identifieurUnique>
    try {
        while (true) {
            const long int filePos = m_file.getPos();  //la position de l'enregistrement dans le fichier
            unsigned int size;
            m_file.readInt4(size);  //<longueur>
            if (size == 0) throw InvalidFileException("term length = 0");
            size += 4;            //+ la longueur de <longueur> = la vraie longueur de l'enregistrement
            unsigned int termId;
            m_file.readInt4(termId);  //<identTerme>
            if (termId > maxIdent) throw InvalidFileException("termId > maxIdent");
            if (termId != 0) {
                //l'enregistrement du fichier est possiblement valide
                RecordLocation &termLocation = m_termLocations[termId];
                if (termLocation.size != 0) {
                    //on a deja trouve cet identifiant
                    if (termLocation.size < size) {
                        //on met l'ancien dans les zones libres
                        m_freeLocations.push_back(termLocation);
                        //on prend le nouveau parce qu'il est plus grand
                        termLocation.offset = filePos;
                        termLocation.size = size;
                    }
                    else {
                        //on garde l'ancien et on met l'enregistrement du fichier en libre
                        const RecordLocation freeLocation(filePos, size);
                        m_freeLocations.push_back(freeLocation);
                    }
                }
                else {
                    //on prend le nouveau parce qu'il est le premier
                    termLocation.offset = filePos;
                    termLocation.size = size;
                }
            }
            else {
                //l'enregistrement du fichier a ete invalide suite a un effacement
                //on le met simplement dans les zones libres
                const RecordLocation freeLocation(filePos, size);
                m_freeLocations.push_back(freeLocation);
            }
        }
    }
    catch (EofException) {
    }
}
////////////////////////////////////////////////////////////
NindTermIndex::~NindTermIndex()
{
    m_file.close();
}
////////////////////////////////////////////////////////////
//brief Add document ident to term ident
//param termId term ident where to add specified document ident
//param category grammatical category
//param documentId document ident to add to specified term ident  */
void NindTermIndex::addDocumentId(const unsigned int termId,
                                  const unsigned int category,
                                  const unsigned int documentId)
    throw (EofException, ReadFileException, WriteFileException, OutOfBoundException, DecodeErrorException, EncodeErrorException)
{
    const unsigned int maxTerms = m_termLocations.size();
    if (maxTerms <= termId) {
        //le vecteur est trop petit, on realloue pour 1000 termes supplementaires
        const RecordLocation nullRecordLocation(0, 0);
        m_termLocations.resize(maxTerms + LOCATION_RESIZE, nullRecordLocation);
    }
    //les termIds sont incrementes et, normalement, il ne peut y avoir un trou de 1000
    //c'est donc le test d'integrite des termId
    if (m_termLocations.size() <= termId) throw OutOfBoundException("termId");
    
    //1) lit l'enregistrement du terme
    RecordLocation &termLocation = m_termLocations[termId];
    m_file.setPos(termLocation.offset, SEEK_SET);
    unsigned char *termRecord = new unsigned char[termLocation.size];  
    m_file.readBytes(termRecord, termLocation.size);
    
    //2) l'enregistrement du terme en clair
    list<NindTerm::TermCatResult> termCatResultList;
    NindTerm::decode(termRecord, termLocation.size, termCatResultList);
    delete termRecord;    //libere la memoire
    
    //3) ajoute le document au bon endroit
    list<NindTerm::TermCatResult>::iterator it = termCatResultList.begin();
    for ( ; it != termCatResultList.end(); it++) {
        if (it->category == category) {
            it->documentsIdList.push_back(documentId);
            it->documentsIdList.sort();       //au cas ou les id de documents ne sont pas dans l'ordre
        }
    }
    if (it == termCatResultList.end()) {
        //la category n'existait pas sur ce terme
        termCatResultList.push_back(NindTerm::TermCatResult());
        NindTerm::TermCatResult &termCatResult = termCatResultList.back();
        termCatResult.termId = termId;
        termCatResult.category = category;
        termCatResult.documentsIdList.push_back(documentId);
    }
    
    //4) recode l'enregistrement du terme
    const unsigned int size = NindTerm::encodedSize(termCatResultList);
    termRecord = new unsigned char[size];
    NindTerm::encode(termRecord, termLocation.size, termCatResultList);
    
    //5) met l'espace occupe dans l'espace libre
    m_freeLocations.push_back(termLocation);

    //6) trouve un espace libre et met a jour les espaces libres
    termLocation.size = size;
    termLocation.offset = getFreeSpace(size);
    
    //7) ecrit l'enregistrement sur le fichier
    m_file.setPos(termLocation.offset, SEEK_SET);
    m_file.writeBytes(termRecord, termLocation.size);
    m_file.write();
    delete termRecord;    //libere la memoire
}
////////////////////////////////////////////////////////////
//brief Suppress document ident to term ident
//param termId term ident where to suppress specified document ident
//param documentId document ident to suppress to specified term ident  */
void NindTermIndex::suppressDocumentId(const unsigned int termId,
                                       const unsigned int documentId)
    throw (EofException, ReadFileException, WriteFileException, OutOfBoundException, DecodeErrorException, EncodeErrorException)
{
    //verifie le termId
    if (m_termLocations.size() <= termId) throw OutOfBoundException("termId");

    //1) lit l'enregistrement du terme
    RecordLocation &termLocation = m_termLocations[termId];
    m_file.setPos(termLocation.offset, SEEK_SET);
    unsigned char *termRecord = new unsigned char[termLocation.size];
    m_file.readBytes(termRecord, termLocation.size);

    //2) l'enregistrement du terme en clair
    list<NindTerm::TermCatResult> termCatResultList;
    NindTerm::decode(termRecord, termLocation.size, termCatResultList);
    delete termRecord;    //libere la memoire

    //3) enleve le document aux bons endroits
    for (list<NindTerm::TermCatResult>::iterator it = termCatResultList.begin(); it != termCatResultList.end(); it++) {
        for (list<unsigned int>:: iterator it2 = it->documentsIdList.begin(); it2 != it->documentsIdList.end(); it2++) {
            if ((*it2) == documentId) {
                it->documentsIdList.erase(it2);
            }
        }
    }

    //4) recode l'enregistrement du terme
    const unsigned int size = NindTerm::encodedSize(termCatResultList);
    termRecord = new unsigned char[size];
    NindTerm::encode(termRecord, termLocation.size, termCatResultList);

    //5) flaggue l'enregistrement comme invalide (uniquement a l'effacement)
    m_file.setPos(termLocation.offset + 4, SEEK_SET);
    m_file.writeInt4(0);              //<identTerme>
    m_file.write();

    //6) met l'espace occupe dans l'espace libre
    m_freeLocations.push_back(termLocation);

    //7) trouve un espace libre et met a jour les espaces libres
    termLocation.size = size;
    termLocation.offset = getFreeSpace(size);

    //8) ecrit l'enregistrement sur le fichier
    m_file.setPos(termLocation.offset, SEEK_SET);
    m_file.writeBytes(termRecord, termLocation.size);
    m_file.write();
    delete termRecord;    //libere la memoire
}
////////////////////////////////////////////////////////////
//brief set identification
//param wordsNb number of words contained in lexicon
//param identification unique identification of lexicon */
void NindTermIndex::setIdentification(const unsigned int wordsNb,
                                      const unsigned int identification)
    throw(WriteFileException)
{
    m_file.setPos(0, SEEK_SET);         //ecrit l'identification en tete
    m_file.writeInt4(wordsNb);          //<maxIdentifiant> 
    m_file.writeInt4(identification);   //<identifieurUnique>
    m_file.write();
}
////////////////////////////////////////////////////////////
//brief get all informations about the specified term
//param termId term ident to get all indexed informations
//param termCatResultList list of all results for the specified term  */
void NindTermIndex::getTermInfos(const unsigned int termId,
                                 list<NindTerm::TermCatResult> &termCatResultList)
    throw (EofException, ReadFileException, OutOfBoundException, DecodeErrorException)
{
    //raz resultat
    termCatResultList.clear();
    //verifie le termId
    if (m_termLocations.size() <= termId) throw OutOfBoundException("termId");
    
    //1) lit l'enregistrement du terme
    const RecordLocation &termLocation = m_termLocations[termId];
    m_file.setPos(termLocation.offset, SEEK_SET);
    unsigned char *termRecord = new unsigned char[termLocation.size];
    m_file.readBytes(termRecord, termLocation.size);

    //2) l'enregistrement du terme en clair
    NindTerm::decode(termRecord, termLocation.size, termCatResultList);
    delete termRecord;    //libere la memoire
}
////////////////////////////////////////////////////////////
//Met a jour la gestion des espaces libres et retourne l'offset ou ecrire
long int NindTermIndex::getFreeSpace(const unsigned int size)
{
    long int offset;
    m_freeLocations.sort();       //normalement ce tri suffit
    list<RecordLocation>::iterator freeIt = m_freeLocations.begin();
    for ( ; freeIt != m_freeLocations.end(); freeIt++) {
        if (freeIt->size >= size) {
            offset = freeIt->offset;
            //nouvel emplacement de l'espace libre
            freeIt->offset += size;
            freeIt->size -=size;
            if (freeIt->size == 0) {
                //cas de l'emplacement libre a 0
                m_freeLocations.erase(freeIt);
            }
            break;
        }
    }
    if (freeIt == m_freeLocations.end()) {
        //pas trouve d'espace libre assez grand, on ecrit a la fin du fichier
        offset = m_file.getFileSize();
    }
    return offset;
}
////////////////////////////////////////////////////////////
