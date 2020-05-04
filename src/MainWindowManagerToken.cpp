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
    mainWindowManager.remove_(mainWindow);
}

}  // namespace SDV
