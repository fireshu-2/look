/*
 * @Filename: utils.hpp
 * @Author: Hongying He
 * @Email: hongying.he@smartsenstech.com
 * @Date: 2025-12-30 14-57-47
 * @Copyright (c) 2025 SmartSens
 */
#pragma once

#include "osd-device.hpp"
#include <algorithm>

class VISUALIZER {
  public:
    void Initialize(std::array<int, 2>& in_img_shape, const std::string& bitmap_lut_path = "");
    void Release();
    void Draw();
    void Draw(const std::vector<std::array<float, 4>>& boxes);
    static const int DETECTION_LAYER_ID = 0;
    void DrawFixedSquare(int x_min, int y_min, int x_max, int y_max, int layer_id = 1);
    void DrawBitmap(const std::string& bitmap_path, const std::string& lut_path = "",
                    int pos_x = 0, int pos_y = 0, int layer_id = 2);
    void ClearLayer(int layer_id = 2);
  private:
    sst::device::osd::OsdDevice osd_device;
    int m_width;
    int m_height;
    std::string m_bitmap_lut_path_full;
};