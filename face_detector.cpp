#include "common.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "math_utils.hpp"

// YOLOv8 640x640 标准 anchor 数
#define NUM_ANCHORS 8400
#define DETECTION_THRESHOLD 0.45f
#define NMS_THRESHOLD 0.45f

void FACE_DETECTOR::Initialize(std::string &model_path, std::array<int, 2> *in_img_shape, std::array<int, 2> *in_det_shape)
{
	img_shape = *in_img_shape;
	det_shape = *in_det_shape;

	char *model_path_char = const_cast<char *>(model_path.c_str());
	model_id = ssne_loadmodel(model_path_char, SSNE_STATIC_ALLOC);

	uint32_t det_width = static_cast<uint32_t>(det_shape[0]);
	uint32_t det_height = static_cast<uint32_t>(det_shape[1]);

	// 创建 1 个输入 tensor
	inputs[0] = create_tensor(det_width, det_height, SSNE_RGB, SSNE_BUF_AI);

	// 🌟 解决 Wrong input tensor: 强制匹配模型数据类型
	int model_dtype = -1;
	ssne_get_model_input_dtype(model_id, &model_dtype);
	set_data_type(inputs[0], model_dtype);

	// 默认离线预处理：全屏缩放
	SetCrop(pipe_offline, 0, 0, img_shape[0], img_shape[1]);
	SetNormalize(pipe_offline, model_id);

	printf("[INFO] FACE_DETECTOR Initialized (Single Output Mode). DType: %d\n", model_dtype);
}

void FACE_DETECTOR::Predict(ssne_tensor_t *img, std::vector<FaceDetectionResult> &out_results)
{
	out_results.clear();

	// 1. 预处理
	int ret = RunAiPreprocessPipe(pipe_offline, *img, inputs[0]);
	if (ret != 0)
		return;

	// 推理前再次确认 DType 属性
	int model_dtype = -1;
	ssne_get_model_input_dtype(model_id, &model_dtype);
	set_data_type(inputs[0], model_dtype);

	// 2. NPU 推理
	if (ssne_inference(model_id, 1, inputs)) {
		fprintf(stderr, "NPU Inference failed!\n");
		return;
	}

	// 3. 获取单输出张量
	ssne_getoutput(model_id, 6, outputs);
	float *data = (float *)get_data(outputs[5]);

	std::vector<FaceDetectionResult> proposals;

	// 4. 解析 YOLOv8 数据格式 [5, 8400]
	float sx = (float)img_shape[0] / (float)det_shape[0];
	float sy = (float)img_shape[1] / (float)det_shape[1];

	for (int i = 0; i < NUM_ANCHORS; i++) {
		float score = data[4 * NUM_ANCHORS + i];
		if (score < DETECTION_THRESHOLD)
			continue;

		float cx = data[0 * NUM_ANCHORS + i];
		float cy = data[1 * NUM_ANCHORS + i];
		float w = data[2 * NUM_ANCHORS + i];
		float h = data[3 * NUM_ANCHORS + i];

		FaceDetectionResult res;
		res.x = (cx - w * 0.5f) * sx;
		res.y = (cy - h * 0.5f) * sy;
		res.width = w * sx;
		res.height = h * sy;
		res.confidence = score;
		proposals.push_back(res);
	}

	// 5. NMS 过滤
	std::sort(proposals.begin(), proposals.end(),
		  [](const FaceDetectionResult &a, const FaceDetectionResult &b) { return a.confidence > b.confidence; });

	std::vector<bool> suppressed(proposals.size(), false);
	for (size_t i = 0; i < proposals.size(); i++) {
		if (suppressed[i])
			continue;
		out_results.push_back(proposals[i]);

		for (size_t j = i + 1; j < proposals.size(); j++) {
			if (suppressed[j])
				continue;
			float ov = iou(proposals[i].x, proposals[i].y, proposals[i].width, proposals[i].height, proposals[j].x,
				       proposals[j].y, proposals[j].width, proposals[j].height);
			if (ov > NMS_THRESHOLD)
				suppressed[j] = true;
		}
	}
}

void FACE_DETECTOR::Release()
{
	release_tensor(inputs[0]);
	for (int i = 0; i < 6; i++) {
		release_tensor(outputs[i]);
	}
	ReleaseAIPreprocessPipe(pipe_offline);
}