//
// Created by zhida.ji1 on 2022/9/14.
//

#pragma once

#include <string>
#include <memory>

namespace zr {
    namespace core {
        class PipelineFeature;
    }

    namespace utils {

        class ShaderGenerator {
        public:

            /// Compute a deterministic variant key string from a PipelineFeature.
            /// Matches gen_shader.py's make_variant_key() format:
            ///   d{DIR}_p{PT}_s{SP}_sk{SKIN}_env{ENV}_vc{VC}
            static std::string compute_variant_key(core::PipelineFeature& feature);

            /// Build the .spv path for a variant shader.
            /// For variant shaders (textured):  "shaders/spv/{shader_name}.{key}.spv"
            /// For simple shaders:              "shaders/spv/{shader_name}.spv"
            static std::string get_shader_path(const std::string& shader_name,
                                               core::PipelineFeature& feature);

            /// Build the .spv path for a simple (non-variant) shader.
            static std::string get_simple_shader_path(const std::string& shader_name);
        };

    }
}
