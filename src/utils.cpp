/*
 * @Filename: utils.cpp
 * @Author: Hongying He
 * @Email: hongying.he@smartsenstech.com
 * @Date: 2025-12-30 14-57-47
 * @Copyright (c) 2025 SmartSens
 */
#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>

void VISUALIZER::Initialize(std::array<int, 2>& in_img_shape, const std::string& bitmap_lut_path) {
    m_width = in_img_shape[0];
    m_height = in_img_shape[1];

    const char* lut_path = nullptr;
    if (!bitmap_lut_path.empty()) {
        m_bitmap_lut_path_full = "/app_demo/app_assets/" + bitmap_lut_path;
        lut_path = m_bitmap_lut_path_full.c_str();
    }
    osd_device.Initialize(m_width, m_height, lut_path);
}

void VISUALIZER::Draw() {
    std::vector<sst::device::osd::OsdQuadRangle> quad_rangle_vec;

    sst::device::osd::OsdQuadRangle q;
    q.color = 0;
    q.box = {100, 100, 200, 200};
    q.border = 3;
    q.alpha = fdevice::TYPE_ALPHA75;
    q.type = fdevice::TYPE_HOLLOW;
    quad_rangle_vec.emplace_back(q);

    osd_device.Draw(quad_rangle_vec);
}

void VISUALIZER::Draw(const std::vector<std::array<float, 4>>& boxes) {
    printf("Drawing %zu detection boxes\n", boxes.size());

    std::vector<sst::device::osd::OsdQuadRangle> quad_rangle_vec;

    for (size_t i = 0; i < boxes.size(); i++) {
        sst::device::osd::OsdQuadRangle q;

        float xmin = static_cast<float>(boxes[i][0]);
        float ymin = static_cast<float>(boxes[i][1]);
        float xmax = static_cast<float>(boxes[i][2]);
        float ymax = static_cast<float>(boxes[i][3]);

        q.box = {xmin, ymin, xmax, ymax};

        q.color = 2;
        q.border = 3;
        q.alpha = fdevice::TYPE_ALPHA75;
        q.type = fdevice::TYPE_HOLLOW;
        q.layer_id = DETECTION_LAYER_ID;
        quad_rangle_vec.emplace_back(q);
    }
    osd_device.Draw(quad_rangle_vec, DETECTION_LAYER_ID);
}

void VISUALIZER::DrawFixedSquare(int x_min, int y_min, int x_max, int y_max, int layer_id) {
    int abs_x_min = x_min;
    int abs_y_min = y_min;
    int abs_x_max = x_max;
    int abs_y_max = y_max;

    if (abs_x_min > abs_x_max) std::swap(abs_x_min, abs_x_max);
    if (abs_y_min > abs_y_max) std::swap(abs_y_min, abs_y_max);

    abs_x_min = std::max(0, std::min(abs_x_min, m_width - 1));
    abs_y_min = std::max(0, std::min(abs_y_min, m_height - 1));
    abs_x_max = std::max(0, std::min(abs_x_max, m_width - 1));
    abs_y_max = std::max(0, std::min(abs_y_max, m_height - 1));

    std::vector<std::array<float, 4>> square_box;
    square_box.push_back({static_cast<float>(abs_x_min),
                         static_cast<float>(abs_y_min),
                         static_cast<float>(abs_x_max),
                         static_cast<float>(abs_y_max)});

    osd_device.Draw(square_box, 0, layer_id, fdevice::TYPE_SOLID, fdevice::TYPE_ALPHA100, 2);
    std::cout << "[VISUALIZER] Fixed square drawn: (" << abs_x_min << ", " << abs_y_min
              << ") to (" << abs_x_max << ", " << abs_y_max << "), layer_id=" << layer_id << std::endl;
}

void VISUALIZER::DrawBitmap(const std::string& bitmap_path, const std::string& lut_path,
                            int pos_x, int pos_y, int layer_id) {
    std::string full_bitmap_path = "/app_demo/app_assets/" + bitmap_path;
    const char* full_lut_path = nullptr;

    osd_device.DrawTexture(full_bitmap_path.c_str(), full_lut_path, layer_id, pos_x, pos_y);
}

void VISUALIZER::ClearLayer(int layer_id) {
    std::vector<std::array<float, 4>> empty;
    osd_device.Draw(empty, 0, layer_id, fdevice::TYPE_SOLID, fdevice::TYPE_ALPHA100, 0);
}

void VISUALIZER::Release() {
    osd_device.Release();
}