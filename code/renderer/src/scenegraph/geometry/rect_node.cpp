//
// Created by zhida.ji1 on 2022/9/2.
//
#include <scenegraph/geometry/rect_node.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/components/material.h>
#include <scenegraph/components/texture.h>
#include <memory>
#include <string.h>

using namespace zr::sg;

RectNode::RectNode(const std::string& name, float width, float height, std::shared_ptr<Texture> tex) :
    Node(name){
    std::shared_ptr<DynamicMesh> mesh = this->add_component<DynamicMesh>();

    std::vector<std::vector<VertexAttribute>> vtx_attrs(1, std::vector<VertexAttribute>());
    vtx_attrs[0].emplace_back(VertexAttribute(5 * sizeof(float), 0, VERTEX_ATTRI_POS));
    vtx_attrs[0].emplace_back(VertexAttribute{5 * sizeof(float), sizeof(float) * 3, VERTEX_ATTRI_UV});
    mesh->set_vtx_attrs(vtx_attrs);

    float uv_margin = 0.0f;

    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();
    float half_width = width * 0.5f;
    float half_height = height * 0.5;
    float data_buf[5] = {0.0f};
    // Mesh::Vertex vtx;
    memset(data_buf, 0, sizeof(float) * 5);
    data_buf[0] = -half_width;
    data_buf[1] = -half_height;
    // vtx.set_data()
    data_buf[3] = uv_margin;
    data_buf[4] = uv_margin;
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));
    memset(data_buf, 0, sizeof(float) * 5);
    data_buf[0] = half_width;
    data_buf[1] = -half_height;
    data_buf[3] = 1.0f - uv_margin;
    data_buf[4] = uv_margin;
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));
    memset(data_buf, 0, sizeof(float) * 5);
    data_buf[0] = half_width;
    data_buf[1] = half_height;
    data_buf[3] = 1.0f - uv_margin;
    data_buf[4] = 1.0f - uv_margin;
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));
    memset(data_buf, 0, sizeof(float) * 5);
    data_buf[0] = -half_width;
    data_buf[1] = half_height;
    data_buf[3] = uv_margin;
    data_buf[4] = 1.0f - uv_margin;
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    triangles.emplace_back(DynamicMesh::Triangle{0, 1, 2});
    triangles.emplace_back(DynamicMesh::Triangle{0, 2, 3});
    // triangles.emplace_back(DynamicMesh::Triangle{0, 3, 2});

    std::shared_ptr<Material> mat = this->add_component<Material>();
    mat->set_light_enabled(false);
    mat->update_texture(TEXTURE_TYPE_DIFFUSE, tex);
}
