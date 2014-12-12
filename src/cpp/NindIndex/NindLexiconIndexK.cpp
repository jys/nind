//
// C++ Implementation: NindLexiconIndexK
//
// Description: La gestion du lexique sous forme de fichier index avec les chaines representees par 2 clefs 32 bits
// voir "nind, indexation post-S2", LAT2014.JYS.440
//
// Cette classe gere la complexite du lexique qui doit rester coherent pour ses lecteurs
// pendant que son ecrivain l'enrichit en fonction des nouvelles indexations.
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) LATECON 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#include "NindLexiconIndexK.h"
#include <iostream>
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
// <definition>            ::= <flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
// <flagDefinition>        ::= <Integer1>
// <identifiantHash>       ::= <Integer3>
// <longueurDonnees>       ::= <Integer3>
// <donneesHash>           ::= { <terme> }
// <terme>                 ::= <clefA> <clefB> <identifiantS> <nbreComposes> <composes>
// <clefA>                 ::= <Integer4>
// <clefB>                 ::= <Integer4>
// <identifiantS>          ::= <Integer3>
// <nbreComposes>          ::= <IntegerULat>
// <composes>              ::= { <compose> } 
// <compose>               ::= <identifiantA> <identifiantRelC>
// <identifiantA>          ::= <Integer3>
// <identifiantRelC>       ::= <IntegerSLat>
////////////////////////////////////////////////////////////
#define FLAG_DEFINITION 17
//<flagDefinition> <identifiantHash> <longueurDonnees> = 7
#define TETE_DEFINITION 7
//<identTermeRelatif>(3) <categorie>(1) <nbreLocalisations>(1) = 5
//#define TETE_DEFINITION_MAXIMUM 5
//<flagDefinition>(1) <identifiantHash>(3) <longueurDonnees>(3) 
//<clefA>(4) <clefB>(4) <identifiantS>(3) <nbreComposes>(1) = 19
#define TAILLE_DEFINITION_MINIMUM 19
//<localisationRelatif>(2) <longueur>(1) = 3
//#define TAILLE_LOC_MAXIMUM 3
//<flagIdentification> <maxIdentifiant> <identifieurUnique> = 8
#define TAILLE_IDENTIFICATION 8
////////////////////////////////////////////////////////////
static void clefs(const string &mot,
                 unsigned int &clefA,
                 unsigned int &clefB);
////////////////////////////////////////////////////////////
//brief Creates NindLexiconIndexK.
//param fileName absolute path file name
//param isLexiconWriter true if lexicon writer, false if lexicon reader  
//param indirectionBlocSize number of entries in a single indirection block */
NindLexiconIndexK::NindLexiconIndexK(const string &fileName,
                                   const bool isLexiconWriter,
                                   const unsigned int indirectionBlocSize)
    throw(NindIndexException):
    NindIndex(fileName, 
        isLexiconWriter, 
        0, 
        0, 
        TAILLE_DEFINITION_MINIMUM, 
        indirectionBlocSize),
    m_modulo(0),
    m_currentId(0),
    m_identification(0)
{
    //la taille du bloc d'indirection du fichier reel est structurante
    m_modulo = getFirstIndirectionBlockSize(); 
    //l'identifiant de terme le plus eleve (pour l'ecrivain)
    getFileIdentification(m_currentId, m_identification);
}
////////////////////////////////////////////////////////////
NindLexiconIndexK::~NindLexiconIndexK()
{
}
////////////////////////////////////////////////////////////
//brief add specified word in lexicon and return its ident if word still exists in lexicon,
//else, word is created in lexicon
//in both cases, word ident is returned.
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconIndexK::addWord(const list<string> &componants)
    throw(NindLexiconIndexException)
{
    try {
        if (!m_isWriter) throw BadUseException("lexicon is not writable");
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
        unsigned int sousMotId = 0;
        for (list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
            bool estNouveau = false;
            const string &motSimple = *swIt;
            list<Terme> definition;
            list<Terme>::iterator termeIt;
            //cherche l'identifiant du terme sur le fichier et prepare la structure de reecriture si pas trouve
            const unsigned int termeId = getDefinitionTermes(motSimple, sousMotId, definition, termeIt);
            if (termeId != 0) {
                //trouve, on passe au suivant
                sousMotId = termeId;
                continue;
            }
            //pas trouve
            //si terme simple inconnu, on le cree
            if ((*termeIt).identifiantS == 0) (*termeIt).identifiantS = ++m_currentId;
            //si terme compose, on le cree
            if (sousMotId != 0) (*termeIt).composes.push_back(Compose(sousMotId, ++m_currentId));
            sousMotId = m_currentId;
            //et on ecrit sur le fichier
            m_identification = (time_t)time(NULL);
            setDefinitionTermes(definition, m_currentId, m_identification);
        }
        //retourne l'id du mot specifie
        return sousMotId;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get ident of the specified word
//if word exists in lexicon, its ident is returned
//else, return 0 (0 is not a valid ident !)
//param componants list of componants of a word (1 componant = simple word, more componants = compound word)
//return ident of word */
unsigned int NindLexiconIndexK::getId(const list<string> &componants)
    throw(NindLexiconIndexException)
{
    try {
        //identifiant du mot (simple ou compose) sous ensemble du mot examine
        unsigned int sousMotId = 0;
        for(list<string>::const_iterator swIt = componants.begin(); swIt != componants.end(); swIt++) {
            const string &motSimple = *swIt;
            const unsigned int termeId = getIdentifiant(motSimple, sousMotId);
            if (termeId == 0) return 0;     //mot inconnu
            sousMotId = termeId;
        }
        //retourne l'id du mot specifie
        return sousMotId;
    }
    catch (FileException &exc) {
        cerr<<"EXCEPTION :"<<exc.m_fileName<<" "<<exc.what()<<endl; 
        throw NindLexiconIndexException(m_fileName);
    }
}
////////////////////////////////////////////////////////////
//brief get identification of lexicon
//param wordsNb where number of words contained in lexicon is returned
//param identification where unique identification of lexicon is returned */
void NindLexiconIndexK::getIdentification(unsigned int &wordsNb, unsigned int &identification)
{
    wordsNb = m_currentId;
    identification = m_identification;
}
////////////////////////////////////////////////////////////
//Recupere l'identifiant d'un terme sur le fichier lexique
//retourne l'identifiant du terme s'il existe, 0 s'il n'existe pas
unsigned int NindLexiconIndexK::getIdentifiant(const string &termeSimple,
                                              const unsigned int sousMotId)
    throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException)
{
    //trouve les 2 clefs qui representent la chaine
    unsigned int clefA, clefB;
    clefs(termeSimple, clefA, clefB);
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const unsigned int ident =  clefB % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (!existe) return 0;              //pas trouve de definition => mot inconnu
    //<flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
    if (m_file.getInt1() != FLAG_DEFINITION) 
        throw InvalidFileException("NindLexiconIndexK::getDefinitionTerme A : " + m_fileName);
    if (m_file.getInt3() != ident) 
        throw InvalidFileException("NindLexiconIndexK::getDefinitionTerme B : " + m_fileName);
    const unsigned int longueurDonnees = m_file.getInt3();
    //positionne la fin de buffer en fonction de la longueur effective des donnees
    m_file.setEndInBuffer(longueurDonnees);
    while (!m_file.endOfInBuffer()) {
        //<clefA> <clefB> <identifiantS> <nbreComposes> <composes>
        const unsigned int fclefA = m_file.getInt4();
        const unsigned int fclefB = m_file.getInt4();
        if (fclefA == clefA && fclefB == clefB) {
            //c'est le bon terme
            const unsigned int identifiantS = m_file.getInt3();
            if (sousMotId == 0) return identifiantS;        //retourne l'id du mot simple
            const unsigned int nbreComposes = m_file.getUIntLat();
            unsigned int identifiantC = identifiantS;
            for (unsigned int i = 0; i != nbreComposes; i++) {
                //<identifiantA> <identifiantRelC>
                const unsigned int identifiantA = m_file.getInt3();
                identifiantC += m_file.getSIntLat();
                if (sousMotId == identifiantA) return identifiantC;     //retourne l'id du mot compose
            }
            return 0;           //mot compose inconnu           
        }
        //ce n'est pas le bon terme, on le saute
        m_file.getInt3();
        const unsigned int nbreComposes = m_file.getUIntLat();
        for (unsigned int i = 0; i != nbreComposes; i++) {
            //<identifiantA> <identifiantRelC>
            m_file.getInt3();
            m_file.getSIntLat();
        }        
    }
    return 0;           //pas trouve le terme simple => mot inconnu  
}
////////////////////////////////////////////////////////////
//recupere les donnees de tous les termes qui ont la meme clef modulo 
//retourne l'identifiant du terme s'il existe, sinon retourne 0
//si le terme n'existe pas, la structure retournée est valide, sinon elle ne l'est pas
unsigned int NindLexiconIndexK::getDefinitionTermes(const string &termeSimple,
                                                   const unsigned int sousMotId,
                                                   list<Terme> &definition,
                                                   list<Terme>::iterator &termeIt)
    throw(EofException, ReadFileException, OutReadBufferException, InvalidFileException)
{
    bool termeEstTrouve = false;
    //trouve les 2 clefs qui representent la chaine
    unsigned int clefA, clefB;
    clefs(termeSimple, clefA, clefB);
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const unsigned int ident =  clefB % m_modulo;
    //lit ce qui concerne cet identifiant
    const bool existe = getDefinition(ident);
    if (existe) {
        //<flagDefinition> <identifiantHash> <longueurDonnees> <donneesHash>
        if (m_file.getInt1() != FLAG_DEFINITION) 
            throw InvalidFileException("NindLexiconIndexK::getDefinitionTerme A : " + m_fileName);
        if (m_file.getInt3() != ident) 
            throw InvalidFileException("NindLexiconIndexK::getDefinitionTerme B : " + m_fileName);
        const unsigned int longueurDonnees = m_file.getInt3();
        //positionne la fin de buffer en fonction de la longueur effective des donnees
        m_file.setEndInBuffer(longueurDonnees);
        while (!m_file.endOfInBuffer()) {
            //cree une nouvelle structure pour un terme
            definition.push_front(Terme());
            Terme &termeCourant = definition.front();
            //<clefA> <clefB> <identifiantS> <nbreComposes> <composes>
            termeCourant.clefA = m_file.getInt4();
            termeCourant.clefB = m_file.getInt4();
            termeCourant.identifiantS = m_file.getInt3();
            const unsigned int nbreComposes = m_file.getUIntLat();
            unsigned int identifiantC = termeCourant.identifiantS;
            //est-ce le terme cherche ?
            if (termeCourant.clefA == clefA && termeCourant.clefB == clefB) {
                if (sousMotId == 0) return termeCourant.identifiantS; //retourne id terme simple
                termeEstTrouve = true;
                termeIt = definition.begin();
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    //si terme compose trouve, on s'arrete immediatement
                    if (sousMotId == identifiantA) return identifiantC;         //retourne id terme compose
                    termeCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
            else {
                for (unsigned int i = 0; i != nbreComposes; i++) {
                    //<identifiantA> <identifiantRelC>
                    const unsigned int identifiantA = m_file.getInt3();
                    identifiantC += m_file.getSIntLat();
                    termeCourant.composes.push_back(Compose(identifiantA, identifiantC));
                }
            }
        }
    }
    if (!termeEstTrouve) {
        //le terme n'est pas sur le fichier, il faut le creer
        //cree une nouvelle structure pour un terme
        definition.push_front(Terme());
        termeIt = definition.begin();
        (*termeIt).clefA = clefA;
        (*termeIt).clefB = clefB;
    }
    return 0;           //terme inconnu et structure coherente pour la reecriture
}
////////////////////////////////////////////////////////////
//Ecrit les donnees de tous les termes qui ont la meme clef modulo 
void NindLexiconIndexK::setDefinitionTermes(const list<Terme> &definition,
                                           const unsigned int lexiconWordsNb,
                                           const unsigned int lexiconIdentification)
{
    //1) calcule la taille max du buffer
    unsigned int tailleMaximum = TETE_DEFINITION + TAILLE_IDENTIFICATION;
    //cerr<<"NindLexiconIndexK::setDefinitionTermes definition.size()="<<definition.size()<<endl;
    for (list<Terme>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {
        //<clefA> <clefB> <identifiantS> <nbreComposes> = 14
        //<identifiantA> <identifiantRelC> = 7
        tailleMaximum += 14 + (*defIt).composes.size() * 7;
        //cerr<<"NindLexiconIndexK::setDefinitionTermes composes.size()="<<(*defIt).composes.size()<<endl;
    }
    //cerr<<"NindLexiconIndexK::setDefinitionTermes tailleMaximum="<<tailleMaximum<<endl;
    //2) forme le buffer a ecrire sur le fichier
    //l'identifiant dans le fichier est un modulo du bloc d'indirection
    const Terme &premierTerme = definition.front();
    const unsigned int ident =  premierTerme.clefB % m_modulo;
    m_file.createBuffer(tailleMaximum); 
    //<flagDefinition> <identifiantHash> <longueurDonnees> 
    m_file.putInt1(FLAG_DEFINITION);
    m_file.putInt3(ident);
    m_file.putInt3(0);         //la taille des donnees sera ecrite plus tard, quand elle sera connue
    for (list<Terme>::const_iterator defIt = definition.begin(); defIt != definition.end(); defIt++) {  
        //<clefA> <clefB> <identifiantS> <nbreComposes> 
        m_file.putInt4((*defIt).clefA);
        m_file.putInt4((*defIt).clefB);
        m_file.putInt3((*defIt).identifiantS);
        m_file.putUIntLat((*defIt).composes.size());
        unsigned int identifiantC = (*defIt).identifiantS;
        for (list<Compose>::const_iterator compIt = (*defIt).composes.begin(); compIt != (*defIt).composes.end(); compIt++) {
            //<identifiantA> <identifiantRelC>
            m_file.putInt3((*compIt).identA);
            m_file.putSIntLat((*compIt).identComp - identifiantC);
            identifiantC = (*compIt).identComp;
        }
    }
    //ecrit la taille reelle du buffer
    const unsigned int longueurDonnees = m_file.getOutBufferSize() - TETE_DEFINITION;
    m_file.putInt3(longueurDonnees, 4);  //la taille dans la trame
    //3) ecrit la definition du terme et gere le fichier
    setDefinition(ident, lexiconWordsNb, lexiconIdentification);   
}
////////////////////////////////////////////////////////////
//calcule les clefs de hachage
static void clefs(const string &mot,
                 unsigned int &clefA,
                 unsigned int &clefB)
{
    unsigned int shiftA = 0;       //nombre de shifts sur la gauche
    unsigned int shiftB = 0;       //nombre de shifts sur la gauche
    clefA = 0x55555555;        //0101...
    clefB = 0x55555555;        //0101...
    for (int i=0; i<mot.length(); i++) {
        const unsigned int oneChar = (unsigned int) ((unsigned char)mot[i]);
        clefA ^=  (oneChar << (shiftA%24));
        clefB ^=  (oneChar << (shiftB%23));
        shiftA += 7;
        shiftB += 5;
        //cerr<<"oneChar="<<oneChar<<hex<<"  clefA="<<clefA<<"  clefB="<<clefB<<endl;
    }
    //cerr<<"mot.length()="<<mot.length()<<hex<<"  clefA="<<clefA<<"  clefB="<<clefB<<endl;
}
////////////////////////////////////////////////////////////
