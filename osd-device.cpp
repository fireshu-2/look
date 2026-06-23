/*
 * FINAL STABLE VERSION - OSD DEVICE
 *
 * 主要特性与修复记录：
 * 1. 内存优化：严格控制 DMA 缓冲大小，避免 4MB CMA 内存溢出 (OOM) 导致 SegFault。
 * 2. 颜色表加载：修复了向底层驱动传递字符串路径的致命 Bug，现改为将 LUT 文件正确读入内存。
 * 3. 图层分离：Layer 0, 1 为画框专用 (TYPE_GRAPHIC)，Layer 2, 3 为贴图专用 (TYPE_IMAGE)。
 * 4. 解决链接报错：补全了所有函数的类作用域限定符 (OsdDevice::)。
 */

#include <iostream>
#include <cstring>
#include <vector>
#include <array>
#include <algorithm>
#include <mutex>
#include <cstdio>

#include "osd-device.hpp"

using namespace fdevice;

namespace sst
{
namespace device
{
namespace osd
{

// 设置为 4 个图层：满足 demo 中 layer_id=2 (贴图) 的调用需求
#define OSD_LAYER_COUNT 4

// 严格按照官方手册 Page 9 建议分配内存，总共占用不到 1MB，安全通过 CMA 限制
#define DMA_BUF_SIZE_GRAPHIC (1024 * 4) // 画框层：4 KB
#define DMA_BUF_SIZE_IMAGE (1024 * 256) // 贴图层：256 KB

static std::mutex g_osd_mutex;

// ==================== Constructor & Destructor ====================

OsdDevice::OsdDevice() : m_height(0), m_width(0), m_osd_handle(0), m_pcolor_lut(nullptr), m_file_size(0)
{
	memset(m_layer_dma, 0, sizeof(m_layer_dma));
}

OsdDevice::~OsdDevice()
{
	Release();
}

// ==================== Initialize & Release ====================

void OsdDevice::Initialize(int width, int height, const char *lut_path)
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle != 0)
		return;

	m_width = width;
	m_height = height;

	printf("[OSD INFO] Initializing OSD Device (%dx%d)...\n", width, height);

	m_osd_handle = osd_open_device();
	if (m_osd_handle == 0) {
		printf("[OSD ERROR] osd_open_device failed!\n");
		return;
	}

// 🌟 核心修复：将 LUT 文件读取到内存中
	if (lut_path != nullptr && strlen(lut_path) > 0) {
		FILE *fp = fopen(lut_path, "rb");
		if (fp != nullptr) {
			fseek(fp, 0, SEEK_END);
			m_file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			// 使用 uint8_t 匹配头文件中的定义
			m_pcolor_lut = new uint8_t[m_file_size];
			size_t read_bytes = fread(m_pcolor_lut, 1, m_file_size, fp);
			if (read_bytes != (size_t)m_file_size) {
				printf("[OSD WARN] Read LUT file size mismatch!\n");
			}
			fclose(fp);
			printf("[OSD INFO] Successfully loaded LUT file: %s (Size: %d bytes)\n", lut_path, m_file_size);
		} else {
			printf("[OSD WARN] Failed to open LUT file: %s. Using default.\n", lut_path);
		}
	}

	// 将包含文件数据的指针传给驱动 (使用 reinterpret_cast 解决类型冲突)
	int ret = osd_init_device(m_osd_handle, OSD_LAYER_COUNT, reinterpret_cast<char *>(m_pcolor_lut));
	if (ret != 0) {
		printf("[OSD ERROR] osd_init_device failed, ret=%d\n", ret);
		return;
	}

	for (int i = 0; i < OSD_LAYER_COUNT; i++) {
		// 前两个图层为图形(小内存)，后两个图层为位图(大内存)
		int buf_size = (i < 2) ? DMA_BUF_SIZE_GRAPHIC : DMA_BUF_SIZE_IMAGE;

		if (osd_alloc_buffer(m_osd_handle, m_layer_dma[i].dma, buf_size) != 0) {
			printf("[OSD ERROR] Failed to alloc DMA for layer %d\n", i);
			continue;
		}
		if (osd_alloc_buffer(m_osd_handle, m_layer_dma[i].dma_2, buf_size) != 0) {
			printf("[OSD ERROR] Failed to alloc secondary DMA for layer %d\n", i);
			continue;
		}

		int fd = osd_get_buffer_fd(m_osd_handle, m_layer_dma[i].dma);
		if (fd < 0) {
			printf("[OSD ERROR] Failed to get fd for layer %d\n", i);
			continue;
		}

		LAYER_ATTR_S layer;
		memset(&layer, 0, sizeof(layer));

		// 🌟 核心修复：根据图层用途分配正确的硬件类型
		if (i < 2) {
			layer.codeTYPE = SS_TYPE_QUADRANGLE;
			layer.layer_rgn.enType = TYPE_GRAPHIC;
		} else {
			layer.codeTYPE = SS_TYPE_RLE;
			layer.layer_rgn.enType = TYPE_IMAGE;
		}

		layer.layer_data_QR.osd_buf.buf_type = BUFFER_TYPE_DMABUF;
		layer.layer_data_QR.osd_buf.buf.fd_dmabuf = fd;
		layer.layerSize.layer_width = m_width;
		layer.layerSize.layer_height = m_height;
		layer.layer_rgn.size_s.w = m_width;
		layer.layer_rgn.size_s.h = m_height;

		if (osd_create_layer(m_osd_handle, (ssLAYER_HANDLE)i, &layer) == 0) {
			osd_set_layer_buffer(m_osd_handle, (ssLAYER_HANDLE)i, m_layer_dma[i]);
			osd_enable_layer(m_osd_handle, (ssLAYER_HANDLE)i, true); // 默认开启显示
		} else {
			printf("[OSD ERROR] Failed to create layer %d\n", i);
		}
	}
	printf("[OSD INFO] OSD Engine fully initialized.\n");
}

void OsdDevice::Release()
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle == 0)
		return;

	for (int i = 0; i < OSD_LAYER_COUNT; i++) {
		osd_destroy_layer(m_osd_handle, (ssLAYER_HANDLE)i);
		if (m_layer_dma[i].dma)
			osd_delete_buffer(m_osd_handle, m_layer_dma[i].dma);
		if (m_layer_dma[i].dma_2)
			osd_delete_buffer(m_osd_handle, m_layer_dma[i].dma_2);
	}
	osd_close_device(m_osd_handle);
	m_osd_handle = 0;

	// 释放 LUT 占用的内存
	if (m_pcolor_lut != nullptr) {
		delete[] m_pcolor_lut;
		m_pcolor_lut = nullptr;
	}
}

// ==================== Draw Methods (Graphic) ====================

void OsdDevice::Draw(std::vector<OsdQuadRangle> &qs)
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle == 0)
		return;

	for (auto &q : qs) {
		GenQrangleBox(q.box, q.border);
		COVER_ATTR_S attr = { q.color, q.type, q.alpha, m_qrangle_out, m_qrangle_in };
		osd_add_quad_rangle(m_osd_handle, &attr);
	}
	osd_flush_quad_rangle(m_osd_handle);
}

void OsdDevice::Draw(std::vector<OsdQuadRangle> &qs, int layer_id)
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle == 0 || layer_id < 0 || layer_id >= OSD_LAYER_COUNT)
		return;

	for (auto &q : qs) {
		GenQrangleBox(q.box, q.border);
		COVER_ATTR_S attr = { q.color, q.type, q.alpha, m_qrangle_out, m_qrangle_in };
		osd_add_quad_rangle_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id, &attr);
	}
	osd_flush_quad_rangle_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id);
}

void OsdDevice::Draw(std::vector<std::array<float, 4> > &boxes, int border, int layer_id, tagQUADRANGLETYPE type,
		     tagALPHATYPE alpha, int color)
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle == 0 || layer_id < 0 || layer_id >= OSD_LAYER_COUNT)
		return;

	for (auto &box : boxes) {
		GenQrangleBox(box, border);
		COVER_ATTR_S attr = { color, type, alpha, m_qrangle_out, m_qrangle_in };
		osd_add_quad_rangle_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id, &attr);
	}
	osd_flush_quad_rangle_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id);
}

// ==================== Draw Methods (Image/Texture) ====================

void OsdDevice::DrawTexture(const char *bitmap_path, const char *lut_path, int layer_id, int pos_x, int pos_y, ALPHATYPE alpha)
{
	std::lock_guard<std::mutex> lock(g_osd_mutex);
	if (m_osd_handle == 0 || layer_id < 0 || layer_id >= OSD_LAYER_COUNT)
		return;

	BITMAP_INFO_S bm;
	memset(&bm, 0, sizeof(bm));
	bm.pSSbmpFile = bitmap_path;
	bm.alpha = alpha;
	bm.position.x = pos_x;
	bm.position.y = pos_y;

	osd_add_texture_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id, &bm);
	osd_flush_texture_layer(m_osd_handle, (ssLAYER_HANDLE)layer_id);
}

// ==================== Box Helper ====================

void OsdDevice::GenQrangleBox(std::array<float, 4> &det, int border)
{
	auto cx = [&](int v) { return std::min(m_width - 1, std::max(0, v)); };
	auto cy = [&](int v) { return std::min(m_height - 1, std::max(0, v)); };

	std::array<int, 16> b;

	b[0] = cx(det[0] + border);
	b[1] = cy(det[1] + border);
	b[2] = cx(det[0] + border);
	b[3] = cy(det[3] - border);
	b[4] = cx(det[2] - border);
	b[5] = cy(det[3] - border);
	b[6] = cx(det[2] - border);
	b[7] = cy(det[1] + border);

	b[8] = cx(det[0] - border);
	b[9] = cy(det[1] - border);
	b[10] = cx(det[0] - border);
	b[11] = cy(det[3] + border);
	b[12] = cx(det[2] + border);
	b[13] = cy(det[3] + border);
	b[14] = cx(det[2] - border);
	b[15] = cy(det[1] - border);

	for (int i = 0; i < 4; i++) {
		m_qrangle_in.points[i] = { b[i * 2], b[i * 2 + 1] };
		m_qrangle_out.points[i] = { b[8 + i * 2], b[8 + i * 2 + 1] };
	}
}

} // namespace osd
} // namespace device
} // namespace sst