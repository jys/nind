//
// C++ Interface: NindCommonExport
//
// Description: pour compiler en DLL windows
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
