
#include "Tools/ONNXRuntime.h"

namespace ldmx {
namespace ort {
using namespace ::Ort;
#if ORT_API_VERSION == 2
// version used when first integrated onnx into ldmx-sw
// and version downloaded by cmake infrastructure
// only support x86_64 architectures
std::string get_input_name(std::unique_ptr<Session>& s, size_t i,
                           AllocatorWithDefaultOptions a) {
  return s->GetInputName(i, a);
}
std::string get_output_name(std::unique_ptr<Session>& s, size_t i,
                            AllocatorWithDefaultOptions a) {
  return s->GetOutputName(i, a);
}
#else
// latest version with prebuilds for both x86_64 and arm64
// architectures but contains a slight API change
std::string getInputName(std::unique_ptr<Session>& s, size_t i,
                         AllocatorWithDefaultOptions a) {
  return s->GetInputNameAllocated(i, a).get();
}
std::string getOutputName(std::unique_ptr<Session>& s, size_t i,
                          AllocatorWithDefaultOptions a) {
  return s->GetOutputNameAllocated(i, a).get();
}
#if ORT_API_VERSION != 15
#pragma warning( \
    "Untested ONNX version, not certain of API, assuming API version 15.")
#endif
#endif

Env ONNXRuntime::env(ORT_LOGGING_LEVEL_WARNING, "");

ONNXRuntime::ONNXRuntime(const std::string& model_path,
                         const SessionOptions* session_options) {
  // create session
  if (session_options) {
    session_.reset(new Session(env, model_path.c_str(), *session_options));
  } else {
    SessionOptions sess_opts;
    sess_opts.SetIntraOpNumThreads(1);
    session_.reset(new Session(env, model_path.c_str(), sess_opts));
  }
  AllocatorWithDefaultOptions allocator;

  // get input names and shapes
  size_t num_input_nodes = session_->GetInputCount();
  input_node_strings_.resize(num_input_nodes);
  input_node_names_.resize(num_input_nodes);
  input_node_dims_.clear();

  for (size_t i = 0; i < num_input_nodes; i++) {
    // get input node names
    std::string input_name(getInputName(session_, i, allocator));
    input_node_strings_[i] = input_name;
    input_node_names_[i] = input_node_strings_[i].c_str();

    // get input shapes
    auto type_info = session_->GetInputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    size_t num_dims = tensor_info.GetDimensionsCount();
    input_node_dims_[input_name].resize(num_dims);
    const auto input_shape = tensor_info.GetShape();
    std::copy(input_shape.begin(), input_shape.end(),
              input_node_dims_[input_name].begin());

  }

  size_t num_output_nodes = session_->GetOutputCount();
  output_node_strings_.resize(num_output_nodes);
  output_node_names_.resize(num_output_nodes);
  output_node_dims_.clear();

  for (size_t i = 0; i < num_output_nodes; i++) {
    // get output node names
    std::string output_name(getOutputName(session_, i, allocator));
    output_node_strings_[i] = output_name;
    output_node_names_[i] = output_node_strings_[i].c_str();

    // get output node types
    auto type_info = session_->GetOutputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    size_t num_dims = tensor_info.GetDimensionsCount();
    output_node_dims_[output_name].resize(num_dims);
    const auto output_shape = tensor_info.GetShape();
    std::copy(output_shape.begin(), output_shape.end(),
              output_node_dims_[output_name].begin());

    // the 0th dim depends on the batch size
    output_node_dims_[output_name].at(0) = -1;
  }
}

FloatArrays ONNXRuntime::run(
    const std::vector<std::string>& input_names,
    FloatArrays& input_values,
    const std::vector<std::string>& output_names,
    int64_t batch_size) const {
  if (input_names.size() != input_values.size()) {
    throw std::runtime_error(
        "The numbers of input names and input values do not match.");
  }

  if (batch_size <= 0) {
    throw std::runtime_error("Batch size must be positive.");
  }

  ShapeArrays input_shapes;
  input_shapes.reserve(input_names.size());

  for (const auto& input_name : input_names) {
    auto shape_iter = input_node_dims_.find(input_name);

    if (shape_iter == input_node_dims_.end()) {
      throw std::runtime_error("Input name '" + input_name + "' is invalid.");
    }

    auto shape = shape_iter->second;

    if (shape.empty()) {
      throw std::runtime_error("Input '" + input_name +
                               "' has no tensor dimensions.");
    }

    shape.at(0) = batch_size;
    input_shapes.emplace_back(std::move(shape));
  }

  return run(input_names, input_values, input_shapes, output_names);
}

FloatArrays ONNXRuntime::run(
    const std::vector<std::string>& input_names,
    FloatArrays& input_values,
    const ShapeArrays& input_shapes,
    const std::vector<std::string>& output_names) const {
  if (input_names.size() != input_values.size()) {
    throw std::runtime_error(
        "The numbers of input names and input values do not match.");
  }

  if (input_names.size() != input_shapes.size()) {
    throw std::runtime_error(
        "The numbers of input names and input shapes do not match.");
  }

  std::vector<Value> input_tensors;
  input_tensors.reserve(input_node_strings_.size());

  auto memory_info =
      MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  for (const auto& model_input_name : input_node_strings_) {
    auto name_iter =
        std::find(input_names.begin(), input_names.end(), model_input_name);

    if (name_iter == input_names.end()) {
      throw std::runtime_error("Input '" + model_input_name +
                               "' is not provided.");
    }

    const auto input_index =
        static_cast<std::size_t>(name_iter - input_names.begin());

    auto& values = input_values.at(input_index);
    const auto& shape = input_shapes.at(input_index);

    if (shape.empty()) {
      throw std::runtime_error("Input shape for '" + model_input_name +
                               "' must not be empty.");
    }

    int64_t expected_length{1};

    for (const auto dimension : shape) {
      if (dimension <= 0) {
        throw std::runtime_error(
            "Input shape for '" + model_input_name +
            "' contains a non-positive dimension: " +
            std::to_string(dimension));
      }

      expected_length *= dimension;
    }

    if (expected_length != static_cast<int64_t>(values.size())) {
      throw std::runtime_error(
          "Input array '" + model_input_name + "' has size " +
          std::to_string(values.size()) + ", but its supplied shape requires " +
          std::to_string(expected_length) + " values.");
    }

    const auto& model_shape = input_node_dims_.at(model_input_name);

    if (shape.size() != model_shape.size()) {
      throw std::runtime_error(
          "Input '" + model_input_name + "' was given rank " +
          std::to_string(shape.size()) + ", but the model expects rank " +
          std::to_string(model_shape.size()) + ".");
    }

    for (std::size_t dimension_index = 0;
         dimension_index < shape.size();
         ++dimension_index) {
      const auto model_dimension = model_shape.at(dimension_index);
      const auto supplied_dimension = shape.at(dimension_index);

      // Negative model dimensions are dynamic and accept any positive value.
      if (model_dimension > 0 && model_dimension != supplied_dimension) {
        throw std::runtime_error(
            "Input '" + model_input_name + "' dimension " +
            std::to_string(dimension_index) + " was given as " +
            std::to_string(supplied_dimension) + ", but the model expects " +
            std::to_string(model_dimension) + ".");
      }
    }

    auto input_tensor = Value::CreateTensor<float>(
        memory_info,
        values.data(),
        values.size(),
        shape.data(),
        shape.size());

    if (!input_tensor.IsTensor()) {
      throw std::runtime_error("Failed to construct tensor for input '" +
                               model_input_name + "'.");
    }

    input_tensors.emplace_back(std::move(input_tensor));
  }

  std::vector<const char*> run_output_node_names;

  if (output_names.empty()) {
    run_output_node_names = output_node_names_;
  } else {
    run_output_node_names.reserve(output_names.size());

    for (const auto& output_name : output_names) {
      if (output_node_dims_.find(output_name) == output_node_dims_.end()) {
        throw std::runtime_error("Output name '" + output_name +
                                 "' is invalid.");
      }

      run_output_node_names.push_back(output_name.c_str());
    }
  }

  auto output_tensors = session_->Run(
      RunOptions{nullptr},
      input_node_names_.data(),
      input_tensors.data(),
      input_tensors.size(),
      run_output_node_names.data(),
      run_output_node_names.size());

  FloatArrays outputs;
  outputs.reserve(output_tensors.size());

  for (auto& output_tensor : output_tensors) {
    if (!output_tensor.IsTensor()) {
      throw std::runtime_error(
          "ONNX Runtime returned an output that is not a tensor.");
    }

    auto tensor_info = output_tensor.GetTensorTypeAndShapeInfo();
    const auto length = tensor_info.GetElementCount();

    auto* values = output_tensor.GetTensorMutableData<float>();
    outputs.emplace_back(values, values + length);
  }

  if (outputs.size() != run_output_node_names.size()) {
    throw std::runtime_error(
        "ONNX Runtime returned an unexpected number of outputs.");
  }

  return outputs;
}

const std::vector<std::string>& ONNXRuntime::getOutputNames() const {
  if (session_) {
    return output_node_strings_;
  } else {
    throw std::runtime_error("ONNXRuntime session is not initialized!");
  }
}

const std::vector<int64_t>& ONNXRuntime::getOutputShape(
    const std::string& output_name) const {
  auto iter = output_node_dims_.find(output_name);
  if (iter == output_node_dims_.end()) {
    throw std::runtime_error("Output name '" + output_name + "' is invalid!");
  } else {
    return iter->second;
  }
}

}  // namespace ort
}  // namespace ldmx
