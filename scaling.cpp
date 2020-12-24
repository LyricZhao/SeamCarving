#include "scaling.h"

QPixmap scale(const QPixmap &image, const QSize &size) {
    // TODO: add algorithms
    return image.scaled(size, Qt::IgnoreAspectRatio);
}
