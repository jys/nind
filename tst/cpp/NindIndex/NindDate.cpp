//
// C++ Implementation: NindDate
//
// Description: Utilitaires pour afficher les dates en clair
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
#include "NindDate.h"
using namespace latecon::nindex;
using namespace std;
////////////////////////////////////////////////////////////
//brief Format time identification as string
//param identification time as number of seconds since 1970
//return date as formatted dtring */
string NindDate::date(const unsigned int identification)
{
    const time_t time = (time_t) identification;
    struct tm * timeinfo = localtime (&time);
    char buffer [80];
    strftime (buffer,80,"%Y-%m-%d %X",timeinfo);
    return string(buffer);
}
////////////////////////////////////////////////////////////
