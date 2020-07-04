//
// Created by kca on 4/7/2020.
//

#include "TextViewSelectionRange.h"
#include <algorithm>
#include <cassert>
#include "TextViewSelection.h"

namespace SDV {

// public explicit
TextViewSelectionRange::
TextViewSelectionRange(const TextViewSelection& p_selection)
    : selection{p_selection},

      firstLineIndex{std::min(p_selection.begin.lineIndex,
                              p_selection.end.lineIndex)},

      lastLineIndex{std::max(p_selection.begin.lineIndex,
                             p_selection.end.lineIndex)}
{
    if (selection.begin.isValid()) {
        assert(false == selection.begin.isEqual(selection.end));
    }
}

}  // namespace SDV
