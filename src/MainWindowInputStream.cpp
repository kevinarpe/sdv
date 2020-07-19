//
// Created by kca on 12/7/2020.
//

#include "MainWindowInputStream.h"
#include <iostream>
#include <fstream>
#include <QFileInfo>
#include <rapidjson/stringbuffer.h>
#include "Constants.h"
#include "SmartPointers.h"
#include "QFileInfos.h"

namespace SDV {

/** {@inheritDoc} */
// public
MainWindowInputStream::
MainWindowInputStream()
    : m_input{MainWindowInput::kNone}, m_bufferCapacity{-1}
{}

/** {@inheritDoc} */
// public
MainWindowInputStream::
MainWindowInputStream(const MainWindowInput& input)
    : m_input{input}
{
    // Dear Reader: Sorry this code is so complex!  There are many states to handle.  :(
    const MainWindowInput::Type type = input.type();
    switch(type)
    {
        case MainWindowInput::Type::Stdin:
        {
            m_inputStream = SmartPointers::Shared::createWithoutDeleter(&std::cin);
            // We have no idea how large is the input.  Use a reasonable default.
            m_bufferCapacity = rapidjson::StringBuffer::kDefaultCapacity;
            break;
        }
        case MainWindowInput::Type::File:
        {
            const QString& absFilePath = input.description();
            const qint64 fileSize = QFileInfos::fileSizeBeforeRead(absFilePath, &m_errorString);
            if (-1 == fileSize)
            {
                m_bufferCapacity = -1;
                m_errorString = "Failed to get file size";
                return;
            }
            if (0 == fileSize)
            {
                m_bufferCapacity = -1;
                m_errorString = "File is empty";
                return;
            }
            // Ref: qUtf8Printable(): https://wiki.qt.io/Technical_FAQ#How_can_I_convert_a_QString_to_char.2A_and_vice_versa.3F
            // Ref: https://stackoverflow.com/a/37133527/257299
            std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>(qUtf8Printable(absFilePath));

            // Ref: https://stackoverflow.com/a/2954161/257299
            if (false == ifs->is_open())
            {
                m_bufferCapacity = -1;
                // Ref: https://stackoverflow.com/questions/17337602/how-to-get-error-message-when-ifstream-open-fails
                m_errorString = QLatin1String{"Failed to open file: %1"}.arg(std::strerror(errno));
                return;
            }
            m_inputStream = ifs;
            m_bufferCapacity = fileSize;
            assert(m_errorString.isEmpty());
            break;
        }
        default: assert(false);
    }
}

// public
MainWindowInputStream::
~MainWindowInputStream() = default;

}  // namespace SDV
