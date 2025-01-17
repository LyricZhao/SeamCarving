#include <iostream>

#include <QApplication>
#include <QPixmap>

#include "window.h"

int main(int argc, char *argv[]) {
    // Check args
    if (argc != 3) {
        std::cerr << "Usage: seam_carving <image_path> <output_path>" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Read image
    QApplication app(argc, argv);
    QImage image;
    if (not image.load(argv[1])) {
        std::cerr << "Failed to load image " << argv[1] << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Show UI
    Window window(image, argv[2]);
    window.show();
    return QApplication::exec();
}
