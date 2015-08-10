#ifndef LIMITSTRING
#define LIMITSTRING

#include <QString>
#include <QtCore>

// this function can be found here:
// https://gist.github.com/lethal-guitar/b9b55313eba00085590c

namespace dunnart {

const QString ELLIPSIS("...");


QString limitString(const QString& aString, int maxLength) {
    if (aString.length() <= maxLength)
        return aString;

    qreal spacePerPart = (maxLength - ELLIPSIS.length()) / 2.0;
    QString beforeEllipsis = aString.left(qCeil(spacePerPart));
    QString afterEllipsis = aString.right(qFloor(spacePerPart));

    return beforeEllipsis + ELLIPSIS + afterEllipsis;
}

}
#endif // LIMITSTRING

