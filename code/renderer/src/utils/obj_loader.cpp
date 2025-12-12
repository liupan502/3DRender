//
// Created by zhida.ji1 on 2022/8/22.
//
#include <utils/obj_loader.h>
#include "tinyobj/tiny_obj_loader.h"
#include <scenegraph/node.h>
#include <memory>
#include <sstream>
#include <string.h>

#include <scenegraph/scene_graph.h>
#include <scenegraph/components/mesh.h>

using namespace zr;
using namespace zr::utils;

std::shared_ptr<sg::Scene> ObjLoader::create_scene(const char* obj_dir, const char *obj_name, const char *mtl_name) {

    std::stringstream  ss;
    ss << obj_dir << obj_name;
    std::string obj_content = load_content(ss.str().c_str());
    ss.str("");
    ss << obj_dir << mtl_name;
    std::string mtl_content = load_content(ss.str().c_str());

    std::shared_ptr<sg::Scene> scene = std::make_shared<sg::Scene>();
    tinyobj::ObjReader reader;
    reader.ParseFromString(obj_content, mtl_content);
    auto shapes = reader.GetShapes();
    for (auto& shape : shapes) {
        create_node(reader, shape, scene, obj_dir);
    }

    return scene;
}

std::string ObjLoader::load_content(const char *file_path) {
    std::string content = FileHelper().load_content(file_path);
    return content;
}

void ObjLoader::create_node(const tinyobj::ObjReader &reader, const tinyobj::shape_t &shape,
                            std::shared_ptr<sg::Scene> scene, const char* obj_dir) {
    std::shared_ptr<sg::Node> node = scene->add_node(shape.name);
    create_mesh(reader, shape, node);
    create_material(reader, shape, node, obj_dir);
}

void ObjLoader::create_mesh(const tinyobj::ObjReader &reader, const tinyobj::shape_t &shape,
                            std::shared_ptr<sg::Node> node) {
    std::shared_ptr<sg::DynamicMesh> mesh = node->add_component<sg::DynamicMesh>();
    std::vector<std::vector<sg::VertexAttribute>> vtx_attrs(1, std::vector<sg::VertexAttribute>());
    vtx_attrs[0].emplace_back(sg::VertexAttribute(8 * sizeof(float), 0, sg::VertexAttributeType::VERTEX_ATTRI_POS));
    vtx_attrs[0].emplace_back(sg::VertexAttribute{8 * sizeof(float), sizeof(float) * 3, sg::VertexAttributeType::VERTEX_ATTRI_UV});
    vtx_attrs[0].emplace_back(sg::VertexAttribute{8 * sizeof(float), sizeof(float) * 5, sg::VertexAttributeType::VERTEX_ATTRI_NORMAL});

    mesh->set_vtx_attrs(vtx_attrs);

    std::vector<sg::DynamicMesh::Vertex>& vertices = mesh->get_vertices();
    std::vector<sg::DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    sg::DynamicMesh::Vertex vertex;
    uint32_t index_offset = 0;

    uint32_t data_buf_size = 8;
    float* data_buf = new float[data_buf_size];

    uint32_t vertex_count = reader.GetAttrib().GetVertices().size();
    uint32_t  normal_count = reader.GetAttrib().normals.size();
    uint32_t uv_count = reader.GetAttrib().texcoords.size();

    std::map<uint64_t, uint16_t> index_map;
    for (uint32_t i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
        // only support triangle list
        if (shape.mesh.num_face_vertices[i] != 3) {
            index_offset += shape.mesh.num_face_vertices[i];
            continue;
        }

        std::vector<uint16_t> sub_indices(3);
        std::vector<float> vertex_data;
        for (uint32_t j = 0; j < 3; j++) {
            uint64_t vertex_index = shape.mesh.indices[index_offset + j].vertex_index;
            // assert(vertex_index < UINT16_MAX);
            uint64_t texcoord_index = shape.mesh.indices[index_offset + j].texcoord_index;
            // assert(texcoord_index < UINT16_MAX);
            uint64_t  normal_index = shape.mesh.indices[index_offset + j].normal_index;
            // assert(normal_index < UINT16_MAX);
            uint64_t key = (((vertex_index << 16) + normal_index) << 16) + texcoord_index;

            if (index_map.find(key) != index_map.end()) {
                sub_indices[j] = index_map[key];
                continue;
            }

            memset(data_buf, 0, sizeof(float) * 8);
            // pos
            memcpy(data_buf, reader.GetAttrib().GetVertices().data() + vertex_index * 3, sizeof(tinyobj::real_t) * 3);
            // uv
            memcpy(data_buf + 3, reader.GetAttrib().texcoords.data() + texcoord_index * 2, sizeof(tinyobj::real_t) * 2);
            data_buf[4] = 1.0f - data_buf[4];
            // normal
            memcpy(data_buf + 5, reader.GetAttrib().normals.data() + normal_index * 3, sizeof(tinyobj::real_t) * 3);

            index_map.insert(std::make_pair(key, vertices.size()));// vertices.size();
            sub_indices[j] = index_map[key];
            vertices.emplace_back(sg::DynamicMesh::Vertex(data_buf, 8 * sizeof(float)));
        }
        index_offset += 3;
        triangles.emplace_back(sg::DynamicMesh::Triangle{sub_indices[0], sub_indices[1], sub_indices[2]});
    }

}

void ObjLoader::create_material(const tinyobj::ObjReader &reader, const tinyobj::shape_t &shape,
                                std::shared_ptr<sg::Node> node, const char* obj_dir) {
    auto mat = node->add_component<sg::Material>();
    // without material
    if (shape.mesh.material_ids[0] < 0) {
            return ;
    }

    uint32_t material_id = shape.mesh.material_ids[0];
    auto obj_mat = reader.GetMaterials()[material_id];
    std::stringstream  ss;
    if (obj_mat.diffuse_texname.size() > 0) {
        ss << obj_dir << obj_mat.diffuse_texname;
        mat->update_texture(sg::TEXTURE_TYPE_DIFFUSE, std::make_shared<sg::SingleLayerTexture>(ss.str().c_str()));
        ss.str("");
    }

}
