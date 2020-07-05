//
// Created by kca on 5/7/2020.
//

#ifndef SDV_QKEYEVENTS_H
#define SDV_QKEYEVENTS_H

class QKeyEvent;

namespace SDV {

struct QKeyEvents
{
    static bool matches(QKeyEvent* event, unsigned int key);
};

}  // namespace SDV

#endif //SDV_QKEYEVENTS_H
