#ifndef OSD_LIB_API_H
#define OSD_LIB_API_H

#include <stdint.h>

typedef void* handle_t;
typedef int ssLAYER_HANDLE;

#define TYPE_GRAPHIC 0
#define TYPE_IMAGE 1

namespace fdevice {
    typedef int ALPHATYPE;
    typedef int QUADRANGLETYPE;

    const int TYPE_ALPHA100 = 0;
    const int TYPE_ALPHA75  = 1;
    const int TYPE_SOLID    = 0;
    const int TYPE_HOLLOW   = 1;

    typedef struct {
        int x;
        int y;
    } POINT_S;

    typedef struct {
        POINT_S points[4];
    } VERTEXS_S;

    typedef struct {
        void* dma;
        void* dma_2;
    } DMA_BUFFER_ATTR_S;
}

typedef struct {
    int codeTYPE;
    struct {
        int enType;
        struct { int w; int h; } size_s;
    } layer_rgn;
    struct {
        struct { int buf_type; struct { int fd_dmabuf; } buf; } osd_buf;
    } layer_data_QR;
    struct { int layer_width; int layer_height; } layerSize;
} LAYER_ATTR_S;

#define SS_TYPE_QUADRANGLE 1
#define SS_TYPE_RLE 2

typedef struct {
    int color;
    fdevice::QUADRANGLETYPE type;
    fdevice::ALPHATYPE alpha;
    fdevice::VERTEXS_S qrangle_out;
    fdevice::VERTEXS_S qrangle_in;
} COVER_ATTR_S;

typedef struct {
    const char* pSSbmpFile;
    fdevice::ALPHATYPE alpha;
    struct { int x; int y; } position;
} BITMAP_INFO_S;

#ifdef __cplusplus
extern "C" {
#endif

handle_t osd_open_device();
int osd_init_device(handle_t handle, int layer_count, char* lut);
int osd_alloc_buffer(handle_t handle, void*& dma, int size);
int osd_get_buffer_fd(handle_t handle, void* dma);
int osd_create_layer(handle_t handle, ssLAYER_HANDLE layer, LAYER_ATTR_S* attr);
void osd_set_layer_buffer(handle_t handle, ssLAYER_HANDLE layer, fdevice::DMA_BUFFER_ATTR_S dma);
void osd_enable_layer(handle_t handle, ssLAYER_HANDLE layer, bool enable);
void osd_destroy_layer(handle_t handle, ssLAYER_HANDLE layer);
void osd_delete_buffer(handle_t handle, void* dma);
void osd_close_device(handle_t handle);
void osd_add_quad_rangle(handle_t handle, COVER_ATTR_S* attr);
void osd_flush_quad_rangle(handle_t handle);
void osd_add_quad_rangle_layer(handle_t handle, ssLAYER_HANDLE layer, COVER_ATTR_S* attr);
void osd_flush_quad_rangle_layer(handle_t handle, ssLAYER_HANDLE layer);
void osd_add_texture_layer(handle_t handle, ssLAYER_HANDLE layer, BITMAP_INFO_S* info);
void osd_flush_texture_layer(handle_t handle, ssLAYER_HANDLE layer);

#ifdef __cplusplus
}
#endif

#endif
