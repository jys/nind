//
// C++ Interface: NindCommonExport
//
// Description: pour compiler en DLL windows
//
// Author: Jean-Yves Sage <jean-yves.sage@orange.fr>, (C) 2014
//
// Copyright: See COPYING file that comes with this distribution
//
////////////////////////////////////////////////////////////
#ifndef NindCommonExport_H
#define NindCommonExport_H
////////////////////////////////////////////////////////////
#ifdef _MSC_VER

#ifdef NindLexicon_EXPORTS
#define DLLExportLexicon _declspec (dllexport)
#else
#define DLLExportLexicon _declspec (dllimport)
#endif
////////////////////////////////////////////////////////////
#else /* Pas _MSC_VER*/

#define DLLExportLexicon

#endif /* _MSC_VER*/
////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////
