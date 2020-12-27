#pragma ide diagnostic ignored "openmp-use-default-none"

#include <iostream>
#include <omp.h>
#include <vector>

#include "scaling.h"

typedef int8_t mark_t;

enum Direction {
    Horizontal,
    Vertical
};

std::vector<int> find_seam(const QImage &image, Direction direction, int *gray_map, int *dp_space, mark_t *mark_space) {
    int w = image.width(), h = image.height();

    // Gray map
#pragma omp parallel for
    for (int y = 0; y < h; ++ y) {
        int index = y * w;
        for (int x = 0; x < w; ++ x, ++ index) {
            gray_map[index] = qGray(image.pixel(x, y));
        }
    }

    // DP with forward energy
    std::vector<int> seam;
    if (direction == Horizontal) {
        for (int y = 0; y < h; ++ y) {
            dp_space[y * w] = 0;
            mark_space[y * w] = 0;
        }
        for (int x = 1; x < w; ++ x) {
            int last_x = x - 1;
#pragma omp parallel for
            for (int y = 0; y < h; ++ y) {
                int left = w * (y - 1), right = w * (y + 1), here = y * w;

                // Decide m_U
                int c_U;
                if (y == 0) {
                    c_U = abs(gray_map[x] - gray_map[x + w]);
                } else if (y == h - 1) {
                    c_U = abs(gray_map[x + here] - gray_map[x + left]);
                } else {
                    c_U = abs(gray_map[x + left] - gray_map[x + right]);
                }
                int m_U = dp_space[last_x + here] + c_U;

                // Decide m_L and m_R
                int m_L = 0x3fffffff, m_R = 0x3fffffff;
                if (y > 0) {
                    m_L = dp_space[last_x + left];
                    m_L += abs(gray_map[last_x + here] - gray_map[x + left]) + c_U;
                } else if (y + 1 < h) {
                    m_R = dp_space[last_x + right];
                    m_R += abs(gray_map[last_x + here] - gray_map[x + right]) + c_U;
                }

                // DP
                int m = std::min(m_U, std::min(m_L, m_R));
                if (m == m_U) mark_space[x + here] = 0;
                if (m == m_L) mark_space[x + here] = -1;
                if (m == m_R) mark_space[x + here] = 1;
                dp_space[x + here] = m;
            }
        }
        int y_pos = 0, index = w - 1;
        int min_value = 0x3fffffff;
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
        for (int x = 0; x < w; ++ x) {
            dp_space[x] = 0;
            mark_space[x] = 0;
        }
        for (int y = 1; y < h; ++ y) {
            int y_index = y * w;
            int last_y_index = (y - 1) * w;
#pragma omp parallel for
            for (int x = 0; x < w; ++ x) {
                int left = x - 1, right = x + 1;

                // Decide m_U
                int c_U;
                if (x == 0) {
                    c_U = abs(gray_map[y_index] - gray_map[y_index + 1]);
                } else if (x == w - 1) {
                    c_U = abs(gray_map[y_index + x] - gray_map[y_index + x - 1]);
                } else {
                    c_U = abs(gray_map[y_index + left] - gray_map[y_index + right]);
                }
                int m_U = dp_space[last_y_index + x] + c_U;

                // Decide m_L and m_R
                int m_L = 0x3fffffff, m_R = 0x3fffffff;
                if (x > 0) {
                    m_L = dp_space[last_y_index + left];
                    m_L += abs(gray_map[last_y_index + x] - gray_map[y_index + left]) + c_U;
                } else if (x + 1 < w) {
                    m_R = dp_space[last_y_index + right];
                    m_R += abs(gray_map[last_y_index + x] - gray_map[y_index + right]) + c_U;
                }

                // DP
                int m = std::min(m_U, std::min(m_L, m_R));
                if (m == m_U) mark_space[y_index + x] = 0;
                if (m == m_L) mark_space[y_index + x] = -1;
                if (m == m_R) mark_space[y_index + x] = 1;
                dp_space[y_index + x] = m;
            }
        }
        int x_pos = 0, index = (h - 1) * w;
        int min_value = 0x3fffffff;
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

QImage add_seam(const QImage &image, const std::vector<int> &seam, Direction direction) {
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
#pragma omp parallel for
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
#pragma omp parallel for
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

QImage delete_seam(const QImage &image, const std::vector<int> &seam, Direction direction) {
    QSize size = direction == Horizontal ? QSize(image.width(), image.height() - 1):
                 QSize(image.width() - 1, image.height());
    QImage scaled(size, image.format());

    if (direction == Horizontal) {
        assert(seam.size() == size.width());
#pragma omp parallel for
        for (int x = 0; x < size.width(); ++ x) {
            for (int y = 0; y < size.height(); ++ y) {
                scaled.setPixel(x, y, image.pixel(x, y + (y >= seam[x])));
            }
        }
    } else {
        assert(seam.size() == size.height());
#pragma omp parallel for
        for (int y = 0; y < size.height(); ++ y) {
            for (int x = 0; x < size.width(); ++ x) {
                scaled.setPixel(x, y, image.pixel(x + (x >= seam[y]), y));
            }
        }
    }
    return scaled;
}

void convert_mark_seam(int w, int h, std::vector<int> &seam, bool *deleted, Direction direction) {
    if (direction == Horizontal) {
        assert(seam.size() == w);
#pragma omp parallel for
        for (int x = 0; x < w; ++ x) {
            int index = x;
            for (int y = 0, removal_y = 0; y < h; ++ y, index += w) {
                if (not deleted[index]) {
                    if (removal_y == seam[x]) {
                        seam[x] = y + y - removal_y;
                        deleted[index] = true;
                        break;
                    }
                    ++ removal_y;
                }
            }
        }
    }

    if (direction == Vertical) {
        assert(seam.size() == h);
#pragma omp parallel for
        for (int y = 0; y < h; ++ y) {
            int index = y * w;
            for (int x = 0, removal_x = 0; x < w; ++ x, ++ index) {
                if (not deleted[index]) {
                    if (removal_x == seam[y]) {
                        seam[y] = x + x - removal_x;
                        deleted[index] = true;
                        break;
                    }
                    ++ removal_x;
                }
            }
        }
    }
}

QImage scale(const QImage &image, const QSize &size) {
    // Make workspace
    QImage scaled = image;
    int max_w = std::max(image.width(), size.width());
    int max_h = std::max(image.height(), size.height());
    auto *gray_map = static_cast<int*>(std::malloc(max_w * max_h * sizeof(int)));
    auto *dp_space = static_cast<int*>(std::malloc(max_w * max_h * sizeof(int)));
    auto *mark_space = static_cast<mark_t*>(std::malloc(max_w * max_h * sizeof(mark_t)));
    auto *deleted = static_cast<bool*>(std::malloc(max_w * max_h * sizeof(bool)));

    // Scale by height
    if (scaled.height() > size.height()) {
        for (int i = 0, length = scaled.height() - size.height(); i < length; ++ i) {
            auto seam = find_seam(scaled, Horizontal, gray_map, dp_space, mark_space);
            scaled = delete_seam(scaled, seam, Horizontal);
        }
    } else if (scaled.height() < size.height()) {
        auto removal = scaled;
        int w = scaled.width(), h = scaled.height();
        std::memset(deleted, false, w * h * sizeof(bool));
        for (int i = 0, length = size.height() - scaled.height(); i < length; ++ i) {
            auto seam = find_seam(removal, Horizontal, gray_map, dp_space, mark_space);
            removal = delete_seam(removal, seam, Horizontal);
            convert_mark_seam(w, h, seam, deleted, Horizontal);
            scaled = add_seam(scaled, seam, Horizontal);
        }
    }

    // Scale by width
    if (scaled.width() > size.width()) {
        for (int i = 0, length = scaled.width() - size.width(); i < length; ++ i) {
            auto seam = find_seam(scaled, Vertical, gray_map, dp_space, mark_space);
            scaled = delete_seam(scaled, seam, Vertical);
        }
    } else if (scaled.width() < size.width()) {
        auto removal = scaled;
        int w = scaled.width(), h = scaled.height();
        std::memset(deleted, false, w * h * sizeof(bool));
        for (int i = 0, length = size.width() - scaled.width(); i < length; ++ i) {
            auto seam = find_seam(removal, Vertical, gray_map, dp_space, mark_space);
            removal = delete_seam(removal, seam, Vertical);
            convert_mark_seam(w, h, seam, deleted, Vertical);
            scaled = add_seam(scaled, seam, Vertical);
        }
    }

    // Free and return
    std::free(gray_map);
    std::free(dp_space);
    std::free(mark_space);
    std::free(deleted);
    return scaled;
}