#pragma once

#include <QImage>
#include <QSize>

void preprocess_seams(const QImage &image, int min_w, int min_h, int max_w, int max_h,
                      std::vector<std::vector<int>> &h_seams, std::vector<std::vector<int>> &v_seams);

QImage scale(const QImage &image, const QSize &size,
             const std::vector<std::vector<int>> &h_seams, const std::vector<std::vector<int>> &v_seams);
