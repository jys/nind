//
// C++ Implementation: NindTestControlC
//
// Description: un test pour vejrifier l'attrapage des control-C
//
#include <iostream>
#include <string>
#include "NindBasics/NindSignalCatcher.h"

using namespace latecon::nindex;
using namespace std;

int main () {
    bool estProtejgej = true;
    string prot;
    NindSignalCatcher *nindSignalCatcher = NindSignalCatcher::Instance();
    while(1) {
        estProtejgej = not estProtejgej;
        prot = (estProtejgej) ? "  P" : " NP";
        if (estProtejgej) nindSignalCatcher->setCatcher();
        else nindSignalCatcher->resetCatcher();
        
        for(int i=0; i<5; i++) {
            cout << prot << i << flush;
            sleep(1); }
        cout << endl;
    }
   return 0;
}

