//
// Created by kca on 14/7/2020.
//

#ifndef SDV_MAINWINDOWINPUT_H
#define SDV_MAINWINDOWINPUT_H

#include <atomic>
#include <QString>
#include "NextIdService.h"

namespace SDV {

/**
 * Tip: This is a value-based class.
 */
class MainWindowInput
{
public:
    enum class Type { None, Clipboard, Stdin, File };

    static const MainWindowInput kNone;
    static const MainWindowInput kStdin;
    static MainWindowInput createClipboard();
    static MainWindowInput createFile(const QString& absFilePath);

    Type type() const { return m_type; }
    bool isNone() const { return (Type::None == m_type); }
    bool isClipboard() const { return (Type::Clipboard == m_type); }
    bool isStdin() const { return (Type::Stdin == m_type); }
    bool isFile() const { return (Type::File == m_type); }

    const QString& description() const;

    // Intentional: Only implement the minimum req'd operators.  Example: Less-than is not req'd.
    bool operator==(const MainWindowInput& rhs) const;

private:
    static NextIdService staticNextIdService;  // NextIdService::withFirstId(1)

    MainWindowInput(const Type type, const QString& description)
        : m_type{type}, m_description{description}
    {}

    MainWindowInput(const Type type, QString&& description)
        : m_type{type}, m_description{std::move(description)}
    {}

    Type m_type;
    /**
     * If {@link #m_type} is {@link Type#File}, this is an absolute file path.
     * <br>If {@link #m_type} is {@link Type#Clipboard}, this is {@code kClipboardDescription + (++staticNextIdService)}.
     */
    QString m_description;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWINPUT_H
