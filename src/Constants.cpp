//
// Created by kca on 25/4/2020.
//

#include "Constants.h"
#include <QSysInfo>

namespace SDV {

// public static
const IsRichText IsRichText::kYes{true};

// public static
const IsRichText IsRichText::kNo{false};

// public static
const QString Constants::kStdinFileName{QLatin1Char{'-'}};

static bool isMicrosoftWindows()
{
    // Docs say: <<On Windows, this function returns the type of Windows kernel, like "winnt".>>
    const QString& kernelType = QSysInfo::kernelType();
    const bool x = (kernelType == QString{QLatin1String{"winnt"}});
    return x;
}

// public static
const bool Constants::kIsMicrosoftWindows{isMicrosoftWindows()};

static QString newLine()
{
    if (isMicrosoftWindows()) {
        return QString{QLatin1String{"\r\n"}};
    }
    else {
        return QString{QLatin1Char{'\n'}};
    }
}

// public static
const QString Constants::kNewLine{newLine()};

// public static
const QString Constants::Json::kObjectBegin{QLatin1Char{'{'}};

// public static
const QString Constants::Json::kObjectEnd{QLatin1Char{'}'}};

// public static
const QString Constants::Json::kArrayBegin{QLatin1Char{'['}};

// public static
const QString Constants::Json::kArrayEnd{QLatin1Char{']'}};

// public static
const QString Constants::IconThemeName::kEditCopy{QLatin1String{"edit-copy"}};

// public static
const QString Constants::IconThemeName::kEditSelectAll{QLatin1String{"edit-select-all"}};

}  // namespace SDV
