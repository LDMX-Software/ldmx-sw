
#ifndef TOOLS_ONNXRUNTIME_H
#define TOOLS_ONNXRUNTIME_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "onnxruntime_cxx_api.h"

namespace ldmx::Ort {

typedef std::vector<std::vector<float>> FloatArrays;

/**
 * @class ONNXRuntime
 * @brief A convenience wrapper of the ONNXRuntime C++ API.
 */
class ONNXRuntime {
 public:
  /**
   * Class constructor.
   * @param model_path Path to the ONNX model file.
   * @param session_options Configuration options of the ONNXRuntime Session.
   * Leave empty to use the default.
   */
  ONNXRuntime(const std::string& model_path,
              const ::Ort::SessionOptions* session_options = nullptr);
  ONNXRuntime(const ONNXRuntime&) = delete;
  ONNXRuntime& operator=(const ONNXRuntime&) = delete;
  ~ONNXRuntime();

  /**
   * Run model inference and get outputs.
   * @param input_names List of the names of the input nodes.
   * @param input_values List of input arrays for each input node. The order of
   * `input_values` must match `input_names`.
   * @param output_names Names of the output nodes to get outputs from. Empty
   * list means all output nodes.
   * @param batch_size Number of samples in the batch. Each array in
   * `input_values` must have a shape layout of (batch_size, ...).
   * @return A std::vector<std::vector<float>>, with the order matched to
   * `output_names`. When `output_names` is empty, will return all outputs
   * ordered as in `getOutputNames()`.
   */
  FloatArrays run(const std::vector<std::string>& input_names,
                  FloatArrays& input_values,
                  const std::vector<std::string>& output_names = {},
                  int64_t batch_size = 1) const;

  /**
   * Get the names of all the output nodes.
   * @return A list of names of all the output nodes.
   */
  const std::vector<std::string>& getOutputNames() const;

  /**
   * Get the shape of a output node.
   * The 0th dim depends on the batch size, therefore is set to -1.
   * @param output_name Name of the output node.
   * @return The shape of the output node as a vector of integers.
   */
  const std::vector<int64_t>& getOutputShape(
      const std::string& output_name) const;

 private:
  static ::Ort::Env env_;
  std::unique_ptr<::Ort::Session> session_;

  std::vector<std::string> input_node_strings_;
  std::vector<const char*> input_node_names_;
  std::map<std::string, std::vector<int64_t>> input_node_dims_;

  std::vector<std::string> output_node_strings_;
  std::vector<const char*> output_node_names_;
  std::map<std::string, std::vector<int64_t>> output_node_dims_;
};

}  // namespace ldmx::Ort

#endif /* TOOLS_ONNXRUNTIME_H_ */
