//
// Created by zhida.ji1 on 2022/9/21.
//

#pragma once

#include <memory>
#include <map>
#include <vulkan/vulkan.h>

#include "pipeline.h"
#include <scenegraph/components/mesh.h>
namespace zr{

    namespace sg{
        class Node;
        class Material;
    }

    namespace core{

        class FgRenderPass;

        typedef std::shared_ptr<Pipeline> (*CreatePipelineFunc)(std::shared_ptr<Device> device, PipelineFeature feature,
                     std::shared_ptr<FgRenderPass> renderpass ,uint16_t subpass_idx);

        class PipelineManager{
        public:
            PipelineManager() : _create_pipeline(nullptr){ } ;
            std::shared_ptr<Pipeline> get_pipeline(LightInfo light_info, std::shared_ptr<sg::Material> material,
                                                   std::vector<std::vector<sg::VertexAttribute>> vtx_attrs);

            inline void set_pipeline_create(CreatePipelineFunc cp) {
                _create_pipeline = cp;
            }
        private:
            
            std::shared_ptr<Pipeline> create_pipeline(PipelineFeature feature);
            // std::shared_ptr<Pipeline> create_pipeline(PipelineFeature feature, uint16_t subpass_idx);
            std::map<PipelineFeature , std::shared_ptr<Pipeline>, PipelineFeature::PipelineFeatureComp> _pipeline_map;
            

           
            // VkExtent2D _display_size;
            CreatePipelineFunc _create_pipeline;
            // std::map<std::string, std::shared_ptr<Pipeline>> _pipeline_map;
        };
    }
}
