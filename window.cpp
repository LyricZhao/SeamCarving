#include "window.h"
#include "scaling.h"

Window::Window(const QPixmap &image, QWidget *parent): image(image), QLabel(parent) {
    setMinimumSize(32, 32);
    setPixmap(image);
}

void Window::resizeEvent(QResizeEvent *event) {
    // int h = event->size().height();
    // int w = event->size().width();
    // std::cout << h << " " << w << std::endl;
    setPixmap(scale(image, event->size()));
    QLabel::resizeEvent(event);
}
