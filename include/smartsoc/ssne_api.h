#ifndef SSNE_API_H
#define SSNE_API_H

#include <stdint.h>

#define SSNE_STATIC_ALLOC 0
#define SSNE_RGB 1
#define SSNE_BUF_AI 2
#define SSNE_YUV422_16 3

typedef void* ssne_tensor_t;
typedef void* AiPreprocessPipe;

#define kPipeline0 0
#define kSensor0 0

uint16_t ssne_loadmodel(char* model_path, int alloc_type);
ssne_tensor_t create_tensor(int w, int h, int format, int buf_type);
void ssne_get_model_input_dtype(uint16_t model_id, int* dtype);
void set_data_type(ssne_tensor_t t, int dtype);
AiPreprocessPipe GetAIPreprocessPipe();
void SetCrop(AiPreprocessPipe p, int x, int y, int w, int h);
void SetNormalize(AiPreprocessPipe p, uint16_t model_id);
int RunAiPreprocessPipe(AiPreprocessPipe p, ssne_tensor_t in, ssne_tensor_t out);
int ssne_inference(uint16_t model_id, int num_in, ssne_tensor_t* in);
void ssne_getoutput(uint16_t model_id, int num_out, ssne_tensor_t* out);
void* get_data(ssne_tensor_t t);
void release_tensor(ssne_tensor_t t);
void ReleaseAIPreprocessPipe(AiPreprocessPipe p);

void OnlineSetOutputImage(int pipe, int format, int w, int h);
int OpenOnlinePipeline(int pipe);
int GetImageData(ssne_tensor_t* out, int pipe, int sensor, int timeout);
void CloseOnlinePipeline(int pipe);
int ssne_initial();
void ssne_release();

#endif
