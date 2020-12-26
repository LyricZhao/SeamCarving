#include <vector>

#include "scaling.h"

typedef long long cost_t;

enum Direction {
    Horizontal,
    Vertical
};

std::vector<int> find_seam(const QImage &image, Direction direction, cost_t* workspace) {
    // TODO: finish coding
}

QImage add_seam(const QImage &image, std::vector<int> &seam, Direction direction) {
    assert(direction == Horizontal or seam.size() == image.height());
    assert(direction == Vertical or seam.size() == image.width());
    // TODO: finish coding
}

QImage delete_seam(const QImage &image, std::vector<int> &seam, Direction direction) {
    assert(direction == Horizontal or seam.size() == image.height());
    assert(direction == Vertical or seam.size() == image.width());
    QPixmap scaled(image.size());
    // TODO: finish coding
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
