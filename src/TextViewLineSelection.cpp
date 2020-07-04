//
// Created by kca on 4/7/2020.
//

#include <cassert>
#include "TextViewLineSelection.h"
#include <QString>
#include "TextViewSelectionRange.h"

namespace SDV {

// public static
TextViewLineSelection
TextViewLineSelection::
none(const QString& line)
{
    return TextViewLineSelection{.beforeLength = line.length(), .length = 0, .afterLength = 0, .isEnd = true};
}

/**
 * @param line
 *        only line.length() is needed, but full line of text is useful for debugging! :)
 */
// public static
TextViewLineSelection
TextViewLineSelection::
create(const TextViewSelectionRange& selectionRange,
       const int lineIndex,
       const QString& line)
{
    const TextViewSelection& selection = selectionRange.selection;
    if (false == selection.isValid() || false == selectionRange.contains(lineIndex))
    {
        const TextViewLineSelection& x = TextViewLineSelection::none(line);
        return x;
    }
    // Forward selection
    if (selection.begin.isLessThan(selection.end))
    {
        if (lineIndex == selection.begin.lineIndex)
        {
            if (lineIndex == selection.end.lineIndex)
            {
                // Forward selection: First and last line are same.
                const TextViewLineSelection x{
                    .beforeLength = selection.begin.charIndex,
                    .length = selection.end.charIndex - selection.begin.charIndex,
                    .afterLength = line.length() - selection.end.charIndex,
                    .isEnd = true
                };
                return x;
            }
            else if (lineIndex < selection.end.lineIndex)
            {
                // Forward selection: First line, but more than one line.
                const TextViewLineSelection x{
                    .beforeLength = selection.begin.charIndex,
                    .length = line.length() - selection.begin.charIndex,
                    .afterLength = 0,
                    .isEnd = false
                };
                return x;
            }
            else {
                assert(false);
            }
        }
        else if (lineIndex < selection.end.lineIndex)
        {
            // Forward selection: Middle line, but not last.
            const TextViewLineSelection x{
                .beforeLength = 0,
                .length = line.length(),
                .afterLength = 0,
                .isEnd = false
            };
            return x;
        }
        else if (lineIndex == selection.end.lineIndex)
        {
            // Forward selection: Last line of many.
            const TextViewLineSelection x{
                .beforeLength = 0,
                .length = selection.end.charIndex,
                .afterLength = line.length() - selection.end.charIndex,
                .isEnd = true
            };
            return x;
        }
        else {
            assert(false);
        }
    }
    // Backward selection
    else {
        if (lineIndex == selection.end.lineIndex)
        {
            if (lineIndex == selection.begin.lineIndex)
            {
                // Backward selection: First and last line are same.
                const TextViewLineSelection x{
                    .beforeLength = selection.end.charIndex,
                    .length = selection.begin.charIndex - selection.end.charIndex,
                    .afterLength = line.length() - selection.begin.charIndex,
                    .isEnd = true
                };
                return x;
            }
            else if (lineIndex < selection.begin.lineIndex)
            {
                // Backward selection: First line, but more than one line.
                const TextViewLineSelection x{
                    .beforeLength = selection.end.charIndex,
                    .length = line.length() - selection.end.charIndex,
                    .afterLength = 0,
                    .isEnd = false
                };
                return x;
            }
            else {
                assert(false);
            }
        }
        else if (lineIndex < selection.begin.lineIndex)
        {
            // Backward selection: Middle line, but not last.
            const TextViewLineSelection x{
                .beforeLength = 0,
                .length = line.length(),
                .afterLength = 0,
                .isEnd = false
            };
            return x;
        }
        else if (lineIndex == selection.begin.lineIndex)
        {
            // Backward selection: Last line of many.
            const TextViewLineSelection x{
                .beforeLength = 0,
                .length = selection.begin.charIndex,
                .afterLength = line.length() - selection.begin.charIndex,
                .isEnd = true
            };
            return x;
        }
        else {
            assert(false);
        }
    }
    assert(false);
}

}  // namespace SDV
