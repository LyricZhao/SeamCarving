#include "window.h"
#include "scaling.h"

Window::Window(const QImage &image, const char *output_path, QWidget *parent):
        image(image.convertToFormat(QImage::Format_ARGB32)), output_path(output_path), QLabel(parent) {
    int min_w = 32, min_h = 32;
    setMinimumSize(min_w, min_h);
    int max_w = static_cast<int>(image.width() * 1.5), max_h = static_cast<int>(image.height() * 1.5);
    setMaximumSize(max_w, max_h);
    display = QPixmap::fromImage(image);
    setPixmap(display);
}

void Window::resizeEvent(QResizeEvent *event) {
    display = QPixmap::fromImage(scale(image, event->size()));
    setPixmap(display);
    QLabel::resizeEvent(event);
}

Window::~Window() {
    display.save(output_path);
}
