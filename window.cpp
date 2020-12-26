#include "window.h"
#include "scaling.h"

Window::Window(const QImage &image, QWidget *parent): image(image.convertToFormat(QImage::Format_ARGB32)), QLabel(parent) {
    setMinimumSize(32, 32);
    setPixmap(QPixmap::fromImage(image));
}

void Window::resizeEvent(QResizeEvent *event) {
    // int h = event->size().height();
    // int w = event->size().width();
    // std::cout << h << " " << w << std::endl;
    setPixmap(QPixmap::fromImage(scale(image, event->size())));
    QLabel::resizeEvent(event);
}
