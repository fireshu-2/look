/*
 * @Filename: demo_face_detection.cpp
 * @Author: Hongying He
 * @Email: hongying.he@smartsenstech.com
 * @Date: 2026-05-02
 * @Copyright (c) 2026 SmartSens
 * @Description: YOLOv8 人脸检测演示程序 - 带UI界面 (干净版)
 */
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <fcntl.h>
#include <regex>
#include <dirent.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <atomic>
#include <array>
#include "utils.hpp"

using namespace std;

#define MAX_DETECTIONS 64

enum DemoPhase { PHASE_INIT, PHASE_RUNNING, PHASE_PAUSED, PHASE_EXIT };

// 全局控制变量
atomic<bool> g_exit_flag(false);
atomic<DemoPhase> g_phase(PHASE_INIT);
mutex g_mtx;

// 演示程序配置
struct {
	int img_width = 1920;
	int img_height = 1080;
	array<int, 2> det_shape = { 640, 640 };
	string model_path = "/app_demo/app_assets/models/yolov8n.m1model";
	float confidence_threshold = 0.45f;
	int display_interval = 30;
	bool show_detection_box = true;
} g_config;

/**
 * @brief 键盘监听线程逻辑
 */
void keyboard_listener()
{
	cout << "========================================" << endl;
	cout << "   YOLOv8 Face Detection Demo Controls" << endl;
	cout << "========================================" << endl;
	cout << "  's' - Start/Pause detection" << endl;
	cout << "  'q' - Quit program" << endl;
	cout << "  't' - Adjust confidence threshold" << endl;
	cout << "  'b' - Toggle bounding box display" << endl;
	cout << "========================================" << endl;
	cout << "Press key to control..." << endl;

	string input;
	while (true) {
		cin >> input;
		lock_guard<mutex> lock(g_mtx);

		if (input == "q" || input == "Q") {
			g_exit_flag = true;
			g_phase = PHASE_EXIT;
			cout << "[UI] Exit signal received!" << endl;
			break;
		} else if (input == "s" || input == "S") {
			if (g_phase == PHASE_RUNNING) {
				g_phase = PHASE_PAUSED;
				cout << "[UI] Detection PAUSED" << endl;
			} else if (g_phase == PHASE_PAUSED) {
				g_phase = PHASE_RUNNING;
				cout << "[UI] Detection RESUMED" << endl;
			}
		} else if (input == "t" || input == "T") {
			cout << "[UI] Enter new threshold (0.1-1.0): ";
			float new_th;
			cin >> new_th;
			if (new_th >= 0.1f && new_th <= 1.0f) {
				g_config.confidence_threshold = new_th;
				cout << "[UI] Threshold updated to: " << g_config.confidence_threshold << endl;
			}
		} else if (input == "b" || input == "B") {
			g_config.show_detection_box = !g_config.show_detection_box;
			cout << "[UI] Bounding box display: " << (g_config.show_detection_box ? "ON" : "OFF") << endl;
		}
	}
}

int main(int argc, char **argv)
{
	cout << "========================================" << endl;
	cout << "   YOLOv8 Face Detection System" << endl;
	cout << "========================================" << endl;

	// 1. 初始化 SSNE 推理引擎
	if (ssne_initial()) {
		fprintf(stderr, "[ERROR] SSNE initialization failed!\n");
		return -1;
	}

	array<int, 2> img_shape = { g_config.img_width, g_config.img_height };

	// 2. 初始化图像处理器 (Online Pipeline)
	IMAGEPROCESSOR processor;
	processor.Initialize(&img_shape);

	// 3. 初始化人脸检测器 (加载模型)
	FACE_DETECTOR detector;
	detector.Initialize(g_config.model_path, &img_shape, &g_config.det_shape);

	// 4. 初始化 OSD 可视化器
	VISUALIZER visualizer;
	visualizer.Initialize(img_shape, "shared_colorLUT.sscl");

	cout << "[SYS] Waiting for hardware stabilization..." << endl;
	sleep(1);

	ssne_tensor_t img_sensor;
	thread listener_thread(keyboard_listener);

	uint32_t frame_count = 0;
	g_phase = PHASE_RUNNING;

	// 5. 主处理循环
	while (!g_exit_flag) {
		// 安全获取图像，防止 SegFault
		int cap_ret = GetImageData(&img_sensor, kPipeline0, kSensor0, 0);
		if (cap_ret != 0) {
			usleep(10000);
			continue;
		}

		std::vector<std::array<float, 4> > osd_boxes;
		std::vector<FaceDetectionResult> results;

		if (g_phase == PHASE_RUNNING) {
			// 执行 AI 推理
			detector.Predict(&img_sensor, results);

			// 过滤并准备 OSD 绘制数据
			for (const auto &res : results) {
				if (res.confidence >= g_config.confidence_threshold) {
					float x1 = res.x;
					float y1 = res.y;
					float x2 = res.x + res.width;
					float y2 = res.y + res.height;

					// 边界裁剪
					x1 = max(0.0f, min(x1, (float)g_config.img_width - 1));
					y1 = max(0.0f, min(y1, (float)g_config.img_height - 1));
					x2 = max(0.0f, min(x2, (float)g_config.img_width - 1));
					y2 = max(0.0f, min(y2, (float)g_config.img_height - 1));

					osd_boxes.push_back({ x1, y1, x2, y2 });
				}
			}

			// 在屏幕上绘制检测框
			if (g_config.show_detection_box) {
				visualizer.Draw(osd_boxes);
			} else {
				visualizer.Draw({}); // 清空图层
			}
		}

		frame_count++;
		if (frame_count % g_config.display_interval == 0) {
			printf("[Frame %d] Active Detections: %zu | Conf: %.2f\n", frame_count, osd_boxes.size(),
			       g_config.confidence_threshold);
		}

		usleep(1000); // 稍微出让 CPU 资源
	}

	// 6. 资源释放
	if (listener_thread.joinable()) {
		listener_thread.join();
	}

	cout << "[SYS] Cleaning up resources..." << endl;
	detector.Release();
	processor.Release();
	visualizer.Release();
	ssne_release();

	cout << "[SYS] Demo exited successfully!" << endl;
	return 0;
}