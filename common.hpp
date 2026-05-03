#pragma once

#include <stdio.h>
#include <vector>
#include <array>
#include <string>
#include <math.h>
#include "smartsoc/ssne_api.h"

struct FaceDetectionResult {
    float x;
    float y;
    float width;
    float height;
    float confidence;
};

class IMAGEPROCESSOR {
  public:
    void Initialize(std::array<int, 2>* in_img_shape);
    void GetImage(ssne_tensor_t* img_sensor);
    void Release();

    std::array<int, 2> img_shape;

  private:
    uint8_t format_online;
};

class FACE_DETECTOR {
  public:
    std::string ModelName() const { return "face_detector"; }

    void Predict(ssne_tensor_t* img_in, std::vector<FaceDetectionResult>& out_results);

    void Initialize(std::string& model_path, std::array<int, 2>* in_img_shape,
                    std::array<int, 2>* in_det_shape);

    void Release();

  private:
    uint16_t model_id = 0;
    ssne_tensor_t inputs[1];
    ssne_tensor_t outputs[6];
    AiPreprocessPipe pipe_offline = GetAIPreprocessPipe();

    std::array<int, 2> img_shape;
    std::array<int, 2> det_shape;
};
