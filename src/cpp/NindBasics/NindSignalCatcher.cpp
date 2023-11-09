//
// C++ Implementation: NindSignalCatcher
//
// Description: Captation provisoire des control-C pour protejger d'iceux les sections
// critiques
//
// Author: jys <jy.sage@orange.fr>, (C) LATEJCON 2023
//
// Copyright: 2014-2023 LATEJCON. See LICENCE.md file that comes with this distribution
// This file is part of NIND (as "nouvelle indexation").
// NIND is free software: you can redistribute it and/or modify it under the terms of the 
// GNU Less General Public License (LGPL) as published by the Free Software Foundation, 
// (see <http://www.gnu.org/licenses/>), either version 3 of the License, or any later version.
// NIND is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Less General Public License for more details.
////////////////////////////////////////////////////////////
#include "NindSignalCatcher.h"
#include <stdlib.h>
////////////////////////////////////////////////////////////
using namespace latecon::nindex;
////////////////////////////////////////////////////////////
bool NindSignalCatcher::m_isUp = false;
bool NindSignalCatcher::m_ctrlC = false;
NindSignalCatcher* NindSignalCatcher::m_instance = 0;
////////////////////////////////////////////////////////////
NindSignalCatcher::NindSignalCatcher() { 
    signal(SIGINT, attrapeCtrlC); 
}
// Return a pointer to the singleton class 
NindSignalCatcher* NindSignalCatcher::Instance() {
    if (m_instance == 0) m_instance = new NindSignalCatcher;
    return m_instance;
}
// Turn on the control-C catcher  
void NindSignalCatcher::setCatcher() { 
    m_isUp = true; 
}
// Turn off the control-C catcher, exit if control-C yet striked    
void NindSignalCatcher::resetCatcher() {
    if (m_ctrlC) exit(SIGINT);
    m_isUp = false; 
}
void NindSignalCatcher::attrapeCtrlC (int signum) {
    if (signum == SIGINT) {
        if (m_isUp) m_ctrlC = true;
        else exit(SIGINT); }
    else exit(signum); 
}
////////////////////////////////////////////////////////////
