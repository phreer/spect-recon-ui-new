#pragma once
#ifndef SCASCNET_H
#define SCASCNET_H

#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include <string>
#include <vector>
#include <array>
#include <utility>
#include <assert.h>

#include "sinogram.h"

const int64_t kNumSlices = 16;
const int64_t kNumAngles = 120;
const int64_t kNumDetectors = 128;

class Scascnet
{
public:
    Scascnet(const ORTCHAR_T* model_path):
        env_(ORT_LOGGING_LEVEL_WARNING, "test"), 
        sess_options_(nullptr),
        sess_(env_, model_path, sess_options_)
    {}
    Sinogram<float> Run(Sinogram<float> input)
    {
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        size_t num_elements = num_slices_ * num_angles_ * num_detectors_;
        std::array<int64_t, 4> input_shape{ 1, num_slices_, num_angles_, num_detectors_ };
        auto input_tensor = Ort::Value::CreateTensor(
            memory_info, input.GetData(), num_elements, input_shape.data(), input_shape.size());
        assert(input_tensor.IsTensor());

        std::vector<const char*> input_node_names(sess_.GetInputCount());
        Ort::AllocatorWithDefaultOptions allocator;
        std::vector<int64_t> input_node_dims;
        // iterate over all input nodes
        for (int i = 0; i < input_node_names.size(); i++) {
            // print input node names
            char* input_name = sess_.GetInputName(i, allocator);
            printf("Input %d : name=%s\n", i, input_name);
            input_node_names[i] = input_name;

            // print input node types
            Ort::TypeInfo type_info = sess_.GetInputTypeInfo(i);
            auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

            ONNXTensorElementDataType type = tensor_info.GetElementType();
            printf("Input %d : type=%d\n", i, type);

            // print input shapes/dims
            input_node_dims = tensor_info.GetShape();
            printf("Input %d : num_dims=%zu\n", i, input_node_dims.size());
            for (int j = 0; j < input_node_dims.size(); j++)
                printf("Input %d : dim %d=%jd\n", i, j, input_node_dims[j]);
        }
        std::vector<const char*> output_node_names(sess_.GetOutputCount());
        for (int i = 0; i < output_node_names.size(); i++) {
            output_node_names[i] = sess_.GetOutputName(i, allocator);
        }
        auto output_tensors = sess_.Run(Ort::RunOptions{ nullptr }, input_node_names.data(),
            &input_tensor, input_node_names.size(),
            output_node_names.data(), output_node_names.size());
        assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());
        float* floatarr = output_tensors.front().GetTensorMutableData<float>();
        return Sinogram<float>(std::vector<float>(
            floatarr, floatarr + num_elements), 
            num_slices_, num_angles_, num_detectors_);
    }
private:
    Ort::Env env_;
    Ort::SessionOptions sess_options_;
    Ort::Session sess_;
    int64_t num_slices_ = 16;
    int64_t num_angles_ = 120;
    int64_t num_detectors_ = 128;
};


#endif // !SCASCNET_H
