#pragma once

#include <iostream>

#include <QImage>
#include <QLabel>
#include <QResizeEvent>
#include <QWidget>

class Window: public QLabel {
    Q_OBJECT
private:
    QImage image;

public:
    explicit Window(const QImage &image, QWidget* parent=nullptr);

public slots:
    void resizeEvent(QResizeEvent *event) override;
};
