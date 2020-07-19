//
// Created by kca on 12/7/2020.
//

#ifndef SDV_TEXTVIEWTEXTSTATSSERVICE_H
#define SDV_TEXTVIEWTEXTSTATSSERVICE_H

#include <memory>
#include <vector>
#include "TextBoundaryFinder.h"
#include "TextViewSelection.h"

namespace SDV {

class TextViewDocument;
class TextViewDocumentView;
class TextViewSelection;

// Scope: Per MainWindow/TextViewDocumentView/TextViewDocument
// @NotThreadSafe
class TextViewTextStatsService final
{
public:
    TextViewTextStatsService(const std::shared_ptr<TextViewDocumentView>& docView);

    struct Result
    {
        int lineCount;
        int graphemeCount;
        int utf8ByteCount;

        bool isValid() const { return (lineCount > 0 && graphemeCount > 0 && utf8ByteCount > 0); }

        void invalidate()
        {
            lineCount = -1;
            graphemeCount = -1;
            utf8ByteCount = -1;
        }
    };
    /**
     * Calc all stats and cache results.
     * <p>
     * Slow.  This method may be called from non-GUI thread.
     */
    Result setDoc(const std::shared_ptr<TextViewDocument>& doc);

    /**
     * Calc stats for text selection using cached results from {@link #setDocView(...)}.
     * <p>
     * Fast.  This method may be called from the GUI thread.
     */
    Result calcStats(const TextViewSelection& selection);

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    TextBoundaryFinder m_textBoundaryFinder;
    /**
     * Caches stats for each text line.
     * <p>
     * Each member does not include trailing new-lines
     */
    struct Counts
    {
        int graphemeCount;
        int utf8ByteCount;
    };
    std::vector<Counts> m_lineIndex_To_Counts;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWTEXTSTATSSERVICE_H
