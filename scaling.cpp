#include <iostream>
#include <vector>

#include "scaling.h"

typedef int64_t cost_t;
typedef int8_t mark_t;

enum Direction {
    Horizontal,
    Vertical
};

std::vector<int> find_seam(const QImage &image, Direction direction, cost_t *dp_space, mark_t *mark_space) {
    int w = image.width(), h = image.height();
    std::memset(mark_space, 0, w * h * sizeof(mark_t));
    auto color_distance = [](const QRgb &color_a, const QRgb &color_b) -> cost_t {
        int r = qRed(color_a); r -= qRed(color_b); if (r < 0) r = -r;
        int g = qGreen(color_a); g -= qGreen(color_b); if (g < 0) g = -g;
        int b = qBlue(color_a); b -= qBlue(color_b); if (b < 0) b = -b;
        return r + g + b;
    };
    for (int y = 0, index = 0; y < h; ++ y) {
        for (int x = 0; x < w; ++ x, ++ index) {
            dp_space[index] = 0;
            int count = 0;
            if (x > 0) {
                dp_space[index] += color_distance(image.pixel(x, y), image.pixel(x - 1, y));
                ++ count;
            }
            if (x + 1 < w) {
                dp_space[index] += color_distance(image.pixel(x, y), image.pixel(x + 1, y));
                ++ count;
            }
            if (y > 0) {
                dp_space[index] += color_distance(image.pixel(x, y), image.pixel(x, y - 1));
                ++ count;
            }
            if (y + 1 < h) {
                dp_space[index] += color_distance(image.pixel(x, y), image.pixel(x, y + 1));
                ++ count;
            }
        }
    }

    auto update = [](cost_t &value, cost_t new_value, mark_t &mark, mark_t new_mark) {
        if (new_value < value) {
            value = new_value;
            mark = new_mark;
        }
    };
    std::vector<int> seam;
    if (direction == Horizontal) {
        for (int x = 1; x < w; ++ x) {
            int index = x;
            for (int y = 0; y < h; ++ y, index += w) {
                cost_t dp = 0x3fffffff, energy = dp_space[index];
                if (y > 0) {
                    update(dp, energy + dp_space[index - w - 1], mark_space[index], -1);
                }
                update(dp, energy + dp_space[index - 1], mark_space[index], 0);
                if (y + 1 < h) {
                    update(dp, energy + dp_space[index + w - 1], mark_space[index], 1);
                }
                dp_space[index] = dp;
            }
        }
        int y_pos = 0, index = w - 1;
        cost_t min_value = 0x3fffffff;
        for (int y = 0; y < h; ++ y, index += w) {
            if (dp_space[index] < min_value) {
                y_pos = y;
                min_value = dp_space[index];
            }
        }
        seam.reserve(w);
        for (int x = w - 1; x >= 0; -- x) {
            seam.push_back(y_pos);
            assert(y_pos >= 0 and y_pos < h);
            y_pos += mark_space[y_pos * w + x];
        }
        std::reverse(seam.begin(), seam.end());
    } else {
        for (int y = 1; y < h; ++ y) {
            int index = y * w;
            for (int x = 0; x < w; ++ x, ++ index) {
                cost_t dp = 0x3fffffff, energy = dp_space[index];
                if (x > 0) {
                    update(dp, energy + dp_space[index - w - 1], mark_space[index], -1);
                }
                update(dp, energy + dp_space[index - w], mark_space[index], 0);
                if (x + 1 < w) {
                    update(dp, energy + dp_space[index - w + 1], mark_space[index], 1);
                }
                dp_space[index] = dp;
            }
        }
        int x_pos = 0, index = (h - 1) * w;
        cost_t min_value = 0x3fffffff;
        for (int x = 0; x < w; ++ x, ++ index) {
            if (dp_space[index] < min_value) {
                x_pos = x;
                min_value = dp_space[index];
            }
        }
        seam.reserve(h);
        for (int y = h - 1; y >= 0; -- y) {
            seam.push_back(x_pos);
            assert(x_pos >= 0 and x_pos < w);
            x_pos += mark_space[y * w + x_pos];
        }
        std::reverse(seam.begin(), seam.end());
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
    auto *dp_space = static_cast<cost_t*>(std::malloc(max_w * max_h * sizeof(cost_t)));
    auto *mark_space = static_cast<mark_t*>(std::malloc(max_w * max_h * sizeof(mark_t)));

    // Scale by height
    while (scaled.height() != size.height()) {
        auto seam = find_seam(scaled, Horizontal, dp_space, mark_space);
        scaled = scaled.height() < size.height() ?
                 add_seam(scaled, seam, Horizontal):
                 delete_seam(scaled, seam, Horizontal);
    }

    // Scale by width
    while (scaled.width() != size.width()) {
        auto seam = find_seam(scaled, Vertical, dp_space, mark_space);
        scaled = scaled.width() < size.width() ?
                 add_seam(scaled, seam, Vertical):
                 delete_seam(scaled, seam, Vertical);
    }

    // Free and return
    std::free(dp_space);
    std::free(mark_space);
    return scaled;
}
