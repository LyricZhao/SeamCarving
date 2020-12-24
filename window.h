#pragma once

#include <iostream>

#include <QLabel>
#include <QResizeEvent>
#include <QWidget>

class Window: public QLabel {
    Q_OBJECT
private:
    QPixmap image;

public:
    explicit Window(const QPixmap &image, QWidget* parent=nullptr);

public slots:
    void resizeEvent(QResizeEvent *event) override;
};
