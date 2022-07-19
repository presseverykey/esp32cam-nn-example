
#include "model_settings.h"
#include "person_detect_model_data.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <esp_heap_caps.h>
#include "config.h"


tflite::ErrorReporter*    error_reporter = nullptr;
const tflite::Model*      model          = nullptr;
tflite::MicroInterpreter* interpreter    = nullptr;
TfLiteTensor*             input          = nullptr;
constexpr int kTensorArenaSize = 81 * 1024;
static uint8_t *tensor_arena;     //memory for input, output and intermediate arrays

void ai_init() {
  //error reporter for logging
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  //get the model from our binary model data
  model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  //allocate some memory to work on
  tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  //operations needed by the graph
  static tflite::MicroMutableOpResolver<5> micro_op_resolver;
  micro_op_resolver.AddAveragePool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();

  //we need an interpreter to interpret the model
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
}

float ai_detect(uint8_t *bitmap) {
  //copy image to input as signed values
  int numPixels = IMAGE_WIDTH * IMAGE_HEIGHT;
  for (int i=0; i<numPixels; i++) {
    input->data.int8[i] = bitmap[i] - 128;
  }  

  //invoke the interpreter with the image
  TfLiteStatus status = interpreter->Invoke();
  if (status != kTfLiteOk) {
    error_reporter->Report("Invoke failed.");
    return -1;
  }

  //get results
  TfLiteTensor* output = interpreter->output(0);
  int8_t person_score_i = output->data.uint8[kPersonIndex];
  float person_score_f = (person_score_i - output->params.zero_point) * output->params.scale;
  return person_score_f;
}