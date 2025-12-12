//
// Created by zhida.ji1 on 2022/9/21.
//
#include <core/core.h>
#include <scenegraph/components/material.h>
#include <scenegraph/components/mesh.h>
#include <core/pipeline_manager.h>

using namespace zr::core;

std::shared_ptr<Pipeline> PipelineManager::get_pipeline(LightInfo light_info,
                                                        std::shared_ptr<sg::Material> material,
                                                        std::vector<std::vector<sg::VertexAttribute>> vtx_attrs) {
    PipelineFeature feature(light_info, material, vtx_attrs);
    uint64_t key = feature.get_val();
    if (_pipeline_map.find(feature) == _pipeline_map.end()) {
        auto pipeline = create_pipeline(feature);
        _pipeline_map.insert(std::make_pair(feature, pipeline));
    }
    return _pipeline_map[feature];
}

/*std::shared_ptr<Pipeline> PipelineManager::create_pipeline(PipelineFeature feature) {

    return std::make_shared<BasePipeline>(_device, _display_size, _render_pass, feature);
}*/

std::shared_ptr<Pipeline> PipelineManager::create_pipeline(PipelineFeature feature) {
    if (_create_pipeline == nullptr) {
        return std::make_shared<BasePipeline>(_device,  _fg_render_pass.lock(), 0, feature);
    }
    return _create_pipeline(_device, feature, _fg_render_pass.lock(), _subpass_idx);
}


