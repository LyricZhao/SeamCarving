#include <vector>

#include "scaling.h"

typedef long long cost_t;

enum Direction {
    Horizontal,
    Vertical
};

std::vector<int> find_seam(const QImage &image, Direction direction, cost_t* workspace) {
    int length = direction == Horizontal ? image.width() : image.height();
    int another = direction == Horizontal ? image.height() : image.width();
    std::vector<int> seam(length);
    seam[0] = rand() % another;
    for (int i = 1; i < length; ++ i) {
        int k;
        do {
            k =  rand() % 3 - 1 + seam[i - 1];
        } while (k < 0 or k >= another);
        seam[i] = k;
    }
    return seam;
}

QImage add_seam(const QImage &image, std::vector<int> &seam, Direction direction) {
    QSize size = direction == Horizontal ? QSize(image.width(), image.height() + 1):
                 QSize(image.width() + 1, image.height());
    QImage scaled(size, image.format());

    auto rgb_average = [](const QRgb &color_a, const QRgb &color_b) -> QRgb {
        int r = qRed(color_a); r += qRed(color_b); r /= 2;
        int g = qGreen(color_a); g += qGreen(color_b); g /= 2;
        int b = qBlue(color_a); b += qBlue(color_b); b /= 2;
        return qRgb(r, g, b);
    };

    if (direction == Horizontal) {
        assert(seam.size() == size.width());
        for (int x = 0; x < size.width(); ++ x) {
            for (int y = 0; y < size.height(); ++ y) {
                scaled.setPixel(x, y, image.pixel(x, y - (y > seam[x])));
            }
            if (seam[x]) {
                auto color = rgb_average(scaled.pixel(x, seam[x]), scaled.pixel(x, seam[x] - 1));
                scaled.setPixel(x, seam[x], color);
            }
        }
    } else {
        assert(seam.size() == size.height());
        for (int y = 0; y < size.height(); ++ y) {
            for (int x = 0; x < size.width(); ++ x) {
                scaled.setPixel(x, y, image.pixel(x - (x > seam[y]), y));
            }
            if (seam[y]) {
                auto color = rgb_average(scaled.pixel(seam[y], y), scaled.pixel(seam[y] - 1, y));
                scaled.setPixel(seam[y], y, color);
            }
        }
    }
    return scaled;
}

QImage delete_seam(const QImage &image, std::vector<int> &seam, Direction direction) {
    QSize size = direction == Horizontal ? QSize(image.width(), image.height() - 1):
            QSize(image.width() - 1, image.height());
    QImage scaled(size, image.format());

    if (direction == Horizontal) {
        assert(seam.size() == size.width());
        for (int x = 0; x < size.width(); ++ x) {
            for (int y = 0; y < size.height(); ++ y) {
                scaled.setPixel(x, y, image.pixel(x, y + (y >= seam[x])));
            }
        }
    } else {
        assert(seam.size() == size.height());
        for (int y = 0; y < size.height(); ++ y) {
            for (int x = 0; x < size.width(); ++ x) {
                scaled.setPixel(x, y, image.pixel(x + (x >= seam[y]), y));
            }
        }
    }
    return scaled;
}

QImage scale(const QImage &image, const QSize &size) {
    // Make workspace
    QImage scaled = image;
    int max_w = std::max(image.width(), size.width());
    int max_h = std::max(image.height(), size.height());
    auto *workspace = static_cast<cost_t*>(std::malloc(max_w * max_h * sizeof(cost_t)));

    // Scale by height
    while (scaled.height() != size.height()) {
        auto seam = find_seam(scaled, Horizontal, workspace);
        scaled = scaled.height() < size.height() ?
                 add_seam(scaled, seam, Horizontal):
                 delete_seam(scaled, seam, Horizontal);
    }

    // Scale by width
    while (scaled.width() != size.width()) {
        auto seam = find_seam(scaled, Vertical, workspace);
        scaled = scaled.width() < size.width() ?
                 add_seam(scaled, seam, Vertical):
                 delete_seam(scaled, seam, Vertical);
    }

    // Free and return
    std::free(workspace);
    return scaled;
}
