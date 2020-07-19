//
// Created by kca on 12/7/2020.
//

#include "QFileInfos.h"
#include <QFile>

namespace SDV {

/** {@inheritDoc} */
// public static
qint64
QFileInfos::
fileSizeBeforeRead(const QString& filePath, QString* errorString)
{
    assert(filePath.size() > 0);
    assert(nullptr != errorString);

    QFile file{filePath};
    // Ref: https://stackoverflow.com/questions/9465727/convert-qfile-to-file
    if (false == file.open(QIODevice::ReadOnly))
    {
        *errorString = file.errorString();
        return -1;
    }
    const qint64 size = file.size();
    return size;
}

}  // namespace SDV
