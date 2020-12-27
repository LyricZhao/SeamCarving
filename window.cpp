#include "window.h"
#include "scaling.h"

Window::Window(const QImage &image, QWidget *parent): image(image.convertToFormat(QImage::Format_ARGB32)), QLabel(parent) {
    int min_w = 32, min_h = 32;
    setMinimumSize(min_w, min_h);
    int max_w = static_cast<int>(image.width() * 1.5), max_h = static_cast<int>(image.height() * 1.5);
    setMaximumSize(max_w, max_h);
    setPixmap(QPixmap::fromImage(image));
}

void Window::resizeEvent(QResizeEvent *event) {
    // int h = event->size().height();
    // int w = event->size().width();
    // std::cout << h << " " << w << std::endl;
    setPixmap(QPixmap::fromImage(scale(image, event->size())));
    QLabel::resizeEvent(event);
}
