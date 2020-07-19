//
// Created by kca on 3/4/2020.
//

#include "MainWindowManagerToken.h"
#include "MainWindowManager.h"

namespace SDV {

// public
MainWindowManagerToken::
~MainWindowManagerToken()
{
    // TODO: Sometimes crashes b/c 'mainWindowManager' has a shorter lifetime than this instance!
    mainWindowManager.remove(mainWindow);
}

}  // namespace SDV
