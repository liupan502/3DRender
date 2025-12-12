//
// Created by zhida.ji1 on 2022/10/9.
//

#include <sstream>


#include <scenegraph/geometry/gltf_node.h>
#include <utils/file_helper.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/components/material.h>
#include <scenegraph/components/skin.h>
#include <scenegraph/components/animation/animation_manager.h>

#ifndef CGLTF_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#endif
#include "cgltf/cgltf.h"
#include "../../../../include/renderer/scenegraph/geometry/gltf_node.h"

#include <json/json.h>
#include <utils/log.h>
#include <iostream>
#include <fstream>
#include <functional>

using namespace zr::sg;

GltfNode::GltfNode(const std::string& path, const std::string& material_file_path, const std::string& name) : Node(name) {
    _gltf_data = nullptr;
    _gltf_path=path;
    _base_dir = utils::FileHelper().get_dir_name_from_path(path);
    _is_valid = parse_gltf(path);
    if (material_file_path.length() > 0) {
        apply_external_material(material_file_path);
    }
    _tex_map.clear();
}

bool GltfNode::apply_external_material(const std::string& material_path) {
    std::stringstream ss;
    ss << _base_dir << material_path;
    std::string content = utils::FileHelper().load_content(ss.str().c_str());
    Json::Value root;
    Json::Reader jr;
    if (!jr.parse(content, root)) {
        return false;
    }
    _material_path=material_path;
    auto mats = root["mats"];
    for (uint16_t i = 0; i < mats.size(); i++) {
        auto json_mat = mats[i];
        std::string node_name = json_mat["name"].asString().c_str();
        auto node = find_node(node_name);
        if (!node) {
            continue;
        }
        auto mat = node->get_component<sg::Material>();
        mat->loadjson(json_mat);
    }
    return true;
}
bool GltfNode::save_external_material() {
    Json::Value root(Json::objectValue); // 创建一个空的 JSON 对象
    Json::Value mats(Json::arrayValue); // 创建一个空的 JSON 数组
    std::function<void(std::shared_ptr<Node>,Json::Value&)> pass_mat=[&](std::shared_ptr<Node> node,Json::Value& mats_t){
        auto node_m= node->get_component<sg::Material>();
        if(node_m){
            Json::Value mat=node_m->tojson();
            mat["name"]=node->get_name();
            mats_t.append(mat);
        }
        for (auto sub_node: node->children())
        {
           pass_mat(sub_node,mats_t);
        }
    };
    for (auto node :children())
    {
        pass_mat(node,mats);
    }
   
    root["mats"]=mats;
    // 输出生成的 JSON 字符串
    Json::StreamWriterBuilder writer;
    std::string jsonString = Json::writeString(writer, root);
    if(_material_path.empty()){
        _material_path=get_name()+".mat";
    }
    std::stringstream ss;
    ss << utils::FileHelper().assert_base_dir<<_base_dir << _material_path;
    //std::cout << ss.str()<< std::endl;
    std::ofstream outputFile( ss.str()); // 打开输出文件
    if (outputFile.is_open()) { // 检查文件是否成功打开
        outputFile << jsonString; // 将字符串写入文件
        outputFile.close(); // 关闭文件
        return true;
    } else {
        return false;
    }
}

GltfNode::~GltfNode() {
    if (_gltf_data) {
        cgltf_free(_gltf_data);
        _gltf_data = nullptr;
    }
}

bool GltfNode::parse_attribute(const cgltf_attribute& attribute, std::shared_ptr<Mesh> sg_mesh) {
    VertexAttributeType sg_attri_type;
    switch(attribute.type) {
        case cgltf_attribute_type_position:
            sg_attri_type = VERTEX_ATTRI_POS;
            break;
        case cgltf_attribute_type_normal:
            sg_attri_type = VERTEX_ATTRI_NORMAL;
            break;
        case cgltf_attribute_type_texcoord:
            sg_attri_type = VERTEX_ATTRI_UV;
            break;
        case cgltf_attribute_type_joints:
            if (attribute.data->component_type == cgltf_component_type_r_8u) {
                sg_attri_type = VERTEX_ATTRI_JOINT_8;
            }
            else {
                sg_attri_type = VERTEX_ATTRI_JOINT_16;
            }
            break;
        case cgltf_attribute_type_weights:
            sg_attri_type = VERTEX_ATTRI_WEIGHT;
            break;
        default:
            sg_attri_type = VERTEX_ATTRI_NONE;
            break;
    }

    // unsupport attribute
    if (sg_attri_type == VERTEX_ATTRI_NONE) {
        return true;
    }
    if (_buf_view_idx_map.find(attribute.data->buffer_view) == _buf_view_idx_map.end()) {
        _buf_view_idx_map.insert(std::make_pair(attribute.data->buffer_view, _buf_view_idx_map.size()));
        _vertex_attrs.emplace_back(std::vector<VertexAttribute>());
    }

    uint32_t idx = _buf_view_idx_map[attribute.data->buffer_view];

    VertexAttribute vtx_attr(attribute.data->stride, attribute.data->offset, sg_attri_type);
    _vertex_attrs[idx].emplace_back(vtx_attr);
    return true;
}

const std::string& GltfNode::get_uri_data(const std::string& uri) {
    if (_external_data_map.find(uri) == _external_data_map.end()) {
        std::stringstream ss;
        ss.str("");
        ss << _base_dir << uri;
        _external_data_map.insert(std::make_pair(uri,
                                                 utils::FileHelper().load_content(ss.str().c_str())));
    }
    return _external_data_map[uri];
}

StaticMesh::MeshBuffer GltfNode::get_mesh_buffer(const cgltf_buffer_view *buf_view) {
    const std::string& buffer_data = get_uri_data(buf_view->buffer->uri);

    StaticMesh::MeshBuffer mesh_buf;
    mesh_buf.len = buf_view->size;
    mesh_buf.ptr = buffer_data.c_str() + buf_view->offset;
    return mesh_buf;
}

bool GltfNode::try_parse_mesh(const cgltf_node *gltf_node, std::shared_ptr<Node> sg_node) {
    const cgltf_mesh* mesh = gltf_node->mesh;
    if (!mesh) {
        return true;
    }
    _buf_view_idx_map.clear();
    _vertex_attrs.clear();

    std::shared_ptr<StaticMesh> sg_mesh = sg_node->add_component<StaticMesh>();
    assert(mesh->primitives_count == 1);
    _cgltf_indice.clear();
    for (uint32_t i = 0; i < mesh->primitives_count; i++) {
        for (uint32_t j = 0; j < mesh->primitives[i].attributes_count; j++) {
            parse_attribute(mesh->primitives[i].attributes[j], sg_mesh);
        }
        assert(mesh->primitives[i].indices != nullptr);
        _cgltf_indice.emplace_back(mesh->primitives[i].indices);
        try_parse_material(mesh->primitives[i].material, sg_node);
    }

    add_vtx_bufs(sg_mesh);

    StaticMesh::MeshBuffer indice_mesh_buf = get_mesh_buffer(_cgltf_indice[0]->buffer_view);
    sg_mesh->set_indice_mesh_buffer(indice_mesh_buf);
    sg_mesh->set_indice_count(_cgltf_indice[0]->count);
    sg_mesh->set_vtx_attrs(_vertex_attrs);
    return true;
}

void GltfNode::add_vtx_bufs(std::shared_ptr<StaticMesh> sg_mesh) {
    std::vector<cgltf_buffer_view*> buf_views( _vertex_attrs.size(),nullptr);
    auto pair = _buf_view_idx_map.begin();
    for (; pair != _buf_view_idx_map.end(); pair++) {
        buf_views[pair->second] = pair->first;
    }

    for (uint32_t i = 0; i < buf_views.size(); i++) {
        StaticMesh::MeshBuffer mesh_buf = get_mesh_buffer(buf_views[i]);
        sg_mesh->add_vtx_mesh_buffer(mesh_buf);
    }

    uint32_t vertex_attrs_size = _vertex_attrs.size();
    for (uint32_t i = 0; i < vertex_attrs_size; i++) {
        if (_vertex_attrs[i].size() == 1) {
            continue;
        }
        uint8_t need_split = 0;
        for (uint16_t j = 0; j < _vertex_attrs[i].size(); j++) {
            auto buf_view = buf_views[i];
            if (_vertex_attrs[i][j].offset >= buf_view->stride) {
                ++need_split;
            }
        }
        if (need_split == 0) {
            continue;
        }

        std::vector<VertexAttribute> stable_vtx_attrs;
        for (uint16_t j = 0; j < _vertex_attrs[i].size(); j++) {
            auto buf_view = buf_views[i];
            if (_vertex_attrs[i][j].offset >= buf_view->stride) {

                StaticMesh::MeshBuffer mesh_buf = get_mesh_buffer(buf_views[i]);
                mesh_buf.ptr = mesh_buf.ptr + _vertex_attrs[i][j].offset;
                mesh_buf.len = mesh_buf.len - _vertex_attrs[i][j].offset;
                sg_mesh->add_vtx_mesh_buffer(mesh_buf);
                _vertex_attrs[i][j].offset = 0;
                _vertex_attrs.emplace_back(std::vector<VertexAttribute>(1, _vertex_attrs[i][j]));

                if (_vertex_attrs[i][j].type == VERTEX_ATTRI_WEIGHT) {
                    float* tmp_ptr = (float*)(mesh_buf.ptr);
                    float a = tmp_ptr[0] + 0.5;
                }
            }
            else {
                stable_vtx_attrs.push_back(_vertex_attrs[i][j]);
            }
        }
        _vertex_attrs[i] = stable_vtx_attrs;
    }
}

bool GltfNode::try_parse_transform(const cgltf_node *gltf_node, std::shared_ptr<Node> sg_node) {

    std::shared_ptr<Transform> transform = sg_node->get_component<Transform>();
    if (gltf_node->has_rotation) {
        transform->set_rotation(glm::quat(
                gltf_node->rotation[3], gltf_node->rotation[0],
                gltf_node->rotation[1], gltf_node->rotation[2]));
    }

    if (gltf_node->has_scale) {
        transform->set_scale(glm::vec3(gltf_node->scale[0], gltf_node->scale[1], gltf_node->scale[2]));
    }

    if (gltf_node->has_translation) {
        transform->set_translation(glm::vec3(gltf_node->translation[0],
                                             gltf_node->translation[1], gltf_node->translation[2]));
    }

    if (gltf_node->has_matrix) {
        const float* ptr = gltf_node->matrix;
        glm::mat4 mat = glm::mat4(
                glm::vec4(ptr[0], ptr[1], ptr[2], ptr[3]),
                glm::vec4(ptr[4], ptr[5], ptr[6], ptr[7]),
                glm::vec4(ptr[8], ptr[9], ptr[10], ptr[11]),
                glm::vec4(ptr[12], ptr[13], ptr[14], ptr[15]));
        transform->set_matrix(mat);
    }
    return true;
}

void GltfNode::parse_skin(const cgltf_skin &skin, std::shared_ptr<sg::Skin> sg_skin) {
    const std::string& data = get_uri_data(skin.inverse_bind_matrices->buffer_view->buffer->uri);
    uint32_t offset = skin.inverse_bind_matrices->offset + skin.inverse_bind_matrices->buffer_view->offset;
    uint8_t* data_ptr = (uint8_t*)data.c_str() + offset;
    uint32_t size = skin.inverse_bind_matrices->stride * skin.inverse_bind_matrices->count;
    sg_skin->set_inverse_bind_mats_info(data_ptr, size);

    std::vector<std::shared_ptr<sg::Node>> joint_nodes;
    for (uint32_t idx = 0; idx < skin.joints_count; idx++) {
        std::string node_name = create_sub_node_name(get_node_idx(skin.joints[idx]));
        auto node = find_node(node_name);
        if (!node) {
            continue;
        }
        node->set_is_joint(true);
        joint_nodes.emplace_back(node);
    }
    sg_skin->set_joint_nodes(joint_nodes);
}

bool GltfNode::try_parse_skins() {
    if (!_gltf_data->skins_count) {
        return true;
    }

    for (uint32_t i = 0; i < _gltf_data->skins_count; i++) {
        const cgltf_skin& skin = _gltf_data->skins[i];

        for (uint32_t j = 0; j < _skin_nodes_arr[i].size(); j++) {
            std::shared_ptr<sg::Skin> sg_skin = _skin_nodes_arr[i][j]->add_component<sg::Skin>();
            parse_skin(skin, sg_skin);
            _skin_nodes_arr[i][j]->get_component<Material>()->set_skin_enabled(true);
        }
    }

    return true;
}

bool GltfNode::try_parse_weight(const cgltf_node *gltf_node, std::shared_ptr<Node> sg_node) {
    return true;
}

bool GltfNode::try_parse_material(const cgltf_material *gltf_material, std::shared_ptr<Node> sg_node) {
    if (!gltf_material) {
        return true;
    }

    std::shared_ptr<Material> sg_material = sg_node->add_component<Material>();
    sg_material->set_light_enabled(!gltf_material->unlit);

    cgltf_texture* gltf_tex = gltf_material->pbr_metallic_roughness.base_color_texture.texture;
    if (!gltf_tex) {
        return false;
    }

    auto sg_diff_tex = get_texture(gltf_tex);
    sg_material->update_texture(TEXTURE_TYPE_DIFFUSE, sg_diff_tex);

    return true;
}

bool GltfNode::parse_node(const cgltf_node *gltf_node, std::shared_ptr<Node> sg_node) {

    // parse children nodes
    for (uint32_t i = 0; i < gltf_node->children_count; i++) {
        std::string sub_node_name = create_sub_node_name(get_node_idx(gltf_node->children[i]));
        std::shared_ptr<Node> child_node = std::make_shared<Node>(sub_node_name);
        if (!parse_node(gltf_node->children[i], child_node)) {
            return false;
        }
        sg_node->add_child(child_node);
    }
    if (gltf_node->skin != nullptr) {
        for (uint32_t i = 0; i < _gltf_data->skins_count; i++) {
            if (&_gltf_data->skins[i] == gltf_node->skin) {
                _skin_nodes_arr[i].emplace_back(sg_node);
                break;
            }
        }
    }
    try_parse_mesh(gltf_node, sg_node);
    try_parse_transform(gltf_node, sg_node);
    // try_parse_skin(gltf_node, sg_node);
    try_parse_animations(gltf_node, sg_node);
    return true;
}

bool GltfNode::parse_scene(const cgltf_scene* scene) {
    if (!scene) {
        return false;
    }

    if (_gltf_data->skins_count > 0) {
        _skin_nodes_arr = std::vector<std::vector<std::shared_ptr<sg::Node>>>(
                _gltf_data->skins_count, std::vector<std::shared_ptr<sg::Node>>());
    }
    for (uint32_t i = 0; i < scene->nodes_count; i++) {
        std::string node_name = create_sub_node_name(get_node_idx(scene->nodes[i]));
        if (find_node(node_name)) {
            continue;
        }
        std::shared_ptr<Node> node = std::make_shared<Node>(node_name);
              if (!parse_node(scene->nodes[i], node)) {
            return false;
        }
        add_child(node);
    }

    try_parse_skins();
    return true;
}

bool GltfNode::parse_gltf(const std::string& path) {
    cgltf_options options;
    memset(&options, 0, sizeof(cgltf_options));
    std::string content = utils::FileHelper().load_content(path.c_str());
    // cgltf_data* _gltf_data = nullptr;
    cgltf_result ret = cgltf_parse(&options, content.c_str(), content.length(), &_gltf_data);
    if (ret != cgltf_result_success) {
        return false;
    }
    return parse_scene(_gltf_data->scene);
}

void GltfNode::try_parse_animations(const cgltf_node* node, std::shared_ptr<sg::Node> sg_node) {
    if (_gltf_data->animations_count <= 0) {
        return;
    }

    std::vector<std::vector<cgltf_animation_channel>> channels(_gltf_data->animations_count, std::vector<cgltf_animation_channel>());
    for (uint32_t i = 0; i < _gltf_data->animations_count; i++) {
        const cgltf_animation& animation = _gltf_data->animations[i];
        for (uint32_t j = 0; j < animation.channels_count; j++) {
            const cgltf_animation_channel & channel = animation.channels[j];
            if (channel.target_node != node) {
                continue;
            }
            channels[i].emplace_back(channel);
        }
    }

    auto animation_mgr = sg_node->get_component<AnimationManager>();
    if (!animation_mgr) {
        animation_mgr = sg_node->add_component<AnimationManager>();
        animation_mgr = sg_node->get_component<AnimationManager>();
    }
    for (uint32_t i = 0; i < channels.size(); i++) {
        if (channels[i].size() == 0) {
            continue;
        }
        const std::string& animation_name = _gltf_data->animations[i].name;
        auto animation = std::make_shared<Animation>(animation_name);
        animation_mgr->add_animation(animation_name, animation);
        for (uint32_t j = 0; j < channels[i].size(); j++) {
            const cgltf_animation_channel  channel = channels[i][j];
            std::shared_ptr<AnimationChannel> sg_channel = std::make_shared<AnimationChannel>();
            parse_animation_channel(channel, sg_channel);
            animation->add_channel(sg_channel);
        }
    }
}

void GltfNode::parse_animation_channel(const cgltf_animation_channel& channel,
                             std::shared_ptr<AnimationChannel> sg_channel) {
    switch (channel.target_path) {
        case cgltf_animation_path_type_translation:
            sg_channel->set_type(ANIMATION_CHANNEL_TYPE_TRANSLATION);
            break;
        case cgltf_animation_path_type_rotation:
            sg_channel->set_type(ANIMATION_CHANNEL_TYPE_ROTATION);
            break;
        case cgltf_animation_path_type_scale:
            sg_channel->set_type(ANIMATION_CHANNEL_TYPE_SCALE);
            break;
        case cgltf_animation_path_type_weights:
            sg_channel->set_type(ANIMATION_CHANNEL_TYPE_WEIGHT);
            break;
        default:
            sg_channel->set_type(ANIMATION_CHANNEL_TYPE_NONE);
            break;
    }

    switch (channel.sampler->interpolation) {
        case cgltf_interpolation_type_linear:
            sg_channel->set_interpolation_type(ANIMATION_INTERPOLATION_TYPE_LINEAR);
            break;
        case cgltf_interpolation_type_step:
            sg_channel->set_interpolation_type(ANIMATION_INTERPOLATION_TYPE_STEP);
            break;
        case cgltf_interpolation_type_cubic_spline:
            sg_channel->set_interpolation_type(ANIMATION_INTERPOLATION_TYPE_CUBIC_SPLINE);
            break;
        default:
            sg_channel->set_interpolation_type(ANIMATION_INTERPOLATION_TYPE_NONE);
            break;
    }


    auto input = channel.sampler->input;
    uint8_t * input_data = (uint8_t*)get_uri_data(input->buffer_view->buffer->uri).c_str();
    uint8_t * input_buf_ptr = input_data + (input->offset + input->buffer_view->offset);
    sg_channel->set_input_buf(input_buf_ptr, input->buffer_view->size, input->stride, input->count);

    auto output = channel.sampler->output;
    uint8_t* output_data = (uint8_t*) get_uri_data(output->buffer_view->buffer->uri).c_str();
    uint8_t* output_buf_ptr = output_data + (output->offset + output->buffer_view->offset);
    sg_channel->set_output_buf(output_buf_ptr, output->buffer_view->size, output->stride, output->count);
}

std::string GltfNode::create_sub_node_name(uint32_t idx) {
    auto sub_node = _gltf_data->nodes[idx];
    std::stringstream ss;

    if (sub_node.name) {
        ss << sub_node.name;
    }
    else {
        ss << get_name() << "_";
        ss << idx;
    }
    return ss.str();
}

uint32_t GltfNode::get_node_idx(const cgltf_node *node) {
    uint32_t idx = 0;
    for (; idx < _gltf_data->nodes_count; idx++) {
        if (&(_gltf_data->nodes[idx]) == node) {
            break;
        }
    }
    return idx;
}

std::shared_ptr<Texture> GltfNode::get_texture(cgltf_texture* gltf_tex) {
    cgltf_image* gltf_img = gltf_tex->image;
    std::stringstream ss;
    ss << _base_dir << gltf_img->uri;
    std::string path = ss.str();

    if (_tex_map.find(path) == _tex_map.end()) {
        
        std::shared_ptr<Texture> sg_diff_tex = std::make_shared<SingleLayerTexture>(ss.str().c_str());
        _tex_map.insert(std::make_pair(path, sg_diff_tex));
    }
    return _tex_map[path];
}


