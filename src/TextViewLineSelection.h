//
// Created by kca on 4/7/2020.
//

#ifndef SDV_TEXTVIEWLINESELECTION_H
#define SDV_TEXTVIEWLINESELECTION_H

class QString;

namespace SDV {

class TextViewSelection;

struct TextViewLineSelection
{
    /** QChar count before selection */
    const int beforeLength;
    /** QChar count of selection */
    const int length;
    /** QChar count after selection */
    const int afterLength;
    /**
     * Does selection end on current line?
     * <br>If true, do not highlight to right edge of viewport.
     * <br>If false, highlight to right edge of viewport.
     */
    const bool isEnd;

    static TextViewLineSelection
    none(const QString& line);

    static TextViewLineSelection
    create(const TextViewSelection& selection,
           const int lineIndex,
           const QString& line);
};

}  // namespace SDV

#endif //SDV_TEXTVIEWLINESELECTION_H
