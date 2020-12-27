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
    QPixmap display;
    const char *output_path;

public:
    explicit Window(const QImage &image, const char *output_path, QWidget* parent=nullptr);
    ~Window() override;

public slots:
    void resizeEvent(QResizeEvent *event) override;
};
