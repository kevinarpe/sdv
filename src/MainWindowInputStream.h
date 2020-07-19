//
// Created by kca on 12/7/2020.
//

#ifndef SDV_MAINWINDOWINPUTSTREAM_H
#define SDV_MAINWINDOWINPUTSTREAM_H

#include <memory>
#include <iosfwd>
#include <QString>
#include "MainWindowInput.h"

namespace SDV {

class MainWindowInputStream
{
public:
    /**
     * Constructed object state via {@link #isValid()} is always {@code false}.
     */
    MainWindowInputStream();
    /**
     * Check constructed object state with {@link #isValid()}.
     *
     * @param input
     *        must be {@code Stdin} or {@code File}
     */
    MainWindowInputStream(const MainWindowInput& input);
    ~MainWindowInputStream();

    bool isValid() const { return (m_bufferCapacity >= 0); }

    const MainWindowInput& input() const { return m_input; }
    /** Check {@link #isValid()} before calling this method */
    std::istream& inputStream() const { return *m_inputStream; }
    qint64 bufferCapacity() const { return m_bufferCapacity; }
    const QString& errorString() const { return m_errorString; }

private:
    MainWindowInput m_input;
    // Why a shared_ptr?  std::istream (std::cin) is weird about coping.
    // Ref: https://stackoverflow.com/questions/2159452/c-assign-cin-to-an-ifstream-variable
    // This might be &std::cin or std::ifstream (open file handle).
    std::shared_ptr<std::istream> m_inputStream;
    qint64 m_bufferCapacity;
    QString m_errorString;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWINPUTSTREAM_H
