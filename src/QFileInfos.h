//
// Created by kca on 12/7/2020.
//

#ifndef SDV_QFILEINFOS_H
#define SDV_QFILEINFOS_H

#include <QString>

namespace SDV {

struct QFileInfos
{
    /**
     * Why is this method called "...BeforeRead"?  The intention is to call this function *before* reading a file.  On some file systems,
     * it is possible to stat a file without read access!  A secondary goal of this function is to provide a resonable output
     * {@code errorString} upon failure (return -1).  This can be useful for error reporting.
     * <p>
     * This function should be preferred over {@link QFileInfo#size()} that returns zero on error and does not provide an error string.
     * The worst of both worlds!
     *
     * @param errorString
     *        required: must not be {@code nullptr}
     *        <br>Upon return -1 (failure), this pointer will hold result of {@link QFile#errorString()}.
     *
     * @return -1 on failure
     *
     * @assert if {@code filePath} is empty
     *         <br>if {@code errorString} is {@code nullptr}
     */
    static qint64
    fileSizeBeforeRead(const QString& filePath, QString* errorString);
};

}  // namespace SDV

#endif //SDV_QFILEINFOS_H
