//
// C++ Interface: NindSignalCatcher
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
#ifndef NindSignalCatcher_H
#define NindSignalCatcher_H
////////////////////////////////////////////////////////////
#include "NindCommonExport.h"
#include <csignal>
////////////////////////////////////////////////////////////
namespace latecon {
    namespace nindex {
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
class DLLExportLexicon NindSignalCatcher {
public:
    /**\brief Return a pointer to the singleton class */
    static NindSignalCatcher* Instance();
    /**\brief Turn on the control-C catcher */    
    void setCatcher();
    /**\brief Turn off the control-C catcher, exit if control-C yet striked */    
    void resetCatcher();
protected:
    NindSignalCatcher(); 
private:
    static bool m_isUp;
    static bool m_ctrlC;
    static void attrapeCtrlC (int signum);
    static NindSignalCatcher* m_instance;
};
////////////////////////////////////////////////////////////
    } // end namespace
} // end namespace
#endif
////////////////////////////////////////////////////////////
