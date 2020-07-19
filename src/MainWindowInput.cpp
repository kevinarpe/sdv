//
// Created by kca on 14/7/2020.
//

#include "MainWindowInput.h"

namespace SDV {

// public static
NextIdService
MainWindowInput::
staticNextIdService = NextIdService::withFirstId(1);

// public static
const MainWindowInput MainWindowInput::kNone{MainWindowInput::Type::None, QString{}};

// public static
const MainWindowInput MainWindowInput::kStdin{MainWindowInput::Type::Stdin, QString{QLatin1String{"<stdin>"}}};

// public static
MainWindowInput
MainWindowInput::
createClipboard()
{
    const int clipboardId = staticNextIdService.nextId();
    // Ex: "<clipboard>:4"
    const QString& desc = QString{QLatin1String{"<clipboard>:"}} + QString::number(clipboardId);
    return MainWindowInput{Type::Clipboard, desc};
}

// public static
MainWindowInput
MainWindowInput::
createFile(const QString& absFilePath)
{
    assert(absFilePath.length() > 0);
    return MainWindowInput{Type::File, absFilePath};
}

// public
const QString&
MainWindowInput::
description()
const
{
    assert(false == isNone());
    return m_description;
}

// public
bool
MainWindowInput::
operator==(const MainWindowInput& rhs)
const
{
    const bool x = (m_type == rhs.m_type && m_description == rhs.m_description);
    return x;
}

}  // namespace SDV
