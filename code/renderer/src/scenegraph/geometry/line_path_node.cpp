//
// Created by zhida.j1 on 2022/9/2.
//
#include <scenegraph/geometry/line_path_node.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/components/material.h>
#include <memory>
#include <string.h>
using namespace zr::sg;

LinePathNode::LinePathNode(const std::string &name, const std::vector<float> line_path_data,
                           float width, std::shared_ptr<Texture> tex,  bool use_tex_len, float tex_len) : Node(name){
    int point_count = line_path_data.size() / 2;
    std::shared_ptr<DynamicMesh> mesh = this->add_component<DynamicMesh>();
    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();

    std::vector<std::vector<VertexAttribute>> vtx_attrs(1, std::vector<VertexAttribute>());
    vtx_attrs[0].emplace_back(VertexAttribute(5 * sizeof(float), 0, VERTEX_ATTRI_POS));
    vtx_attrs[0].emplace_back(VertexAttribute{5 * sizeof(float), sizeof(float) * 3, VERTEX_ATTRI_UV});
    mesh->set_vtx_attrs(vtx_attrs);
    float pre_normal_x = 0.0f;
    float pre_normal_y = 0.0f;
    float data_buf[5] = {0.0f};
    float total_len = 0.0;
    for (int i = 0; i < point_count; i++) {
        int nxt_idx = i + 1;
        int pre_idx = i - 1;

        if (i == 0) {
            float normal_x = 0.0f;
            float normal_y = 0.0f;
            float offset_x = line_path_data[nxt_idx * 2] - line_path_data[i * 2];
            float offset_y = line_path_data[nxt_idx * 2 + 1] - line_path_data[i * 2 + 1];
            normal_x = -offset_y;
            normal_y = offset_x;
            float length = sqrt(offset_x * offset_x + offset_y * offset_y);
            normal_x /= length;
            normal_y /= length;
            float x = line_path_data[i * 2];
            float y = line_path_data[i * 2 + 1];

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x - 0.5 * width * normal_x;
            data_buf[1] = y - 0.5 * width * normal_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 0.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x + 0.5 * width * normal_x;
            data_buf[1] = y + 0.5 * width * normal_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 1.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));
        }

        else if (nxt_idx == point_count) {
            float normal_x = 0.0f;
            float normal_y = 0.0f;
            float offset_x = line_path_data[i * 2] - line_path_data[pre_idx * 2];
            float offset_y = line_path_data[i * 2 + 1] - line_path_data[pre_idx * 2 + 1];

            normal_x = -offset_y;
            normal_y = offset_x;
            float length = sqrt(offset_x * offset_x + offset_y * offset_y);
            total_len += length;
            normal_x /= length;
            normal_y /= length;
            float x = line_path_data[i * 2];
            float y = line_path_data[i * 2 + 1];

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x - 0.5 * width * normal_x;
            data_buf[1] = y - 0.5 * width * normal_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 0.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x + 0.5 * width * normal_x;
            data_buf[1] = y + 0.5 * width * normal_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 1.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));
        }
        else {
            float x0 = line_path_data[pre_idx * 2] - line_path_data[i * 2];
            float y0 = line_path_data[pre_idx * 2 + 1] - line_path_data[i * 2 + 1];
            float len0 = sqrt(x0 * x0 + y0 * y0);
            total_len += len0;
            x0 /= len0;
            y0 /= len0;
            float x1 = line_path_data[nxt_idx * 2] - line_path_data[i * 2];
            float y1 = line_path_data[nxt_idx * 2 + 1] - line_path_data[i * 2 + 1];
            float len1 = sqrt(x1 * x1 + y1 * y1);
            x1 /= len1;
            y1 /= len1;
            float cos_theta = x0 * x1 + y0 * y1;
            cos_theta = cos_theta < -1.0f ? -1.0f : cos_theta;
            if (cos_theta > 0.99) {
                continue;
            }
            cos_theta = cos_theta > 1.0f ? 1.0f : cos_theta;
            float sin_theta = x0 * y1 - x1 * y0;
            sin_theta = sin_theta < -1.0f ? -1.0f : sin_theta;
            sin_theta = sin_theta > 1.0f ? 1.0f : sin_theta;
            float half_theta = acos(cos_theta) * 0.5;

            float normal_x = y0;
            float normal_y = -x0;

            if (sin_theta > 0) {
                normal_y = -normal_y;
                normal_x = -normal_x;
            }

            float val = 0.5 * width / tan(half_theta);
            float offset_x = val * x0 + 0.5 * width * normal_x;
            float offset_y = val * y0 + 0.5 * width * normal_y;
            float flag = sin_theta > 0 ? -1.0f : 1.0f;

            float x = line_path_data[i * 2];
            float y = line_path_data[i * 2 + 1];

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x - flag * offset_x;
            data_buf[1] = y - flag * offset_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 0.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0] = x + flag * offset_x;
            data_buf[1] = y + flag * offset_y;
            data_buf[2] = 0.0f;
            data_buf[3] = 1.0f;
            data_buf[4] = use_tex_len ? total_len / tex_len : i * 1.0f/ point_count;
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            pre_normal_x = normal_x;
            pre_normal_y = normal_y;
        }
    }

    auto compute_ret = [&](int idx0, int idx1, int offset)->float {
        // uint16_t base = i * 2;
        float* p0_ptr = (float*)vertices[idx0 * 2 + offset].get_data();
        glm::vec2 p00 = glm::vec2(p0_ptr[0], p0_ptr[1]);
        float* p1_ptr = (float*)vertices[idx1 * 2 + offset].get_data();
        glm::vec2 p01 = glm::vec2(p1_ptr[0], p1_ptr[1]);
        glm::vec2 dir0 = glm::normalize(p01 - p00);


        glm::vec2 p10 = glm::vec2(line_path_data[idx0 * 2], line_path_data[idx0 * 2 + 1]);

        glm::vec2 p11 = glm::vec2(line_path_data[idx1 * 2], line_path_data[idx1 * 2 + 1]);
        glm::vec2 dir1 = glm::normalize(p11 - p10);
        float ret = glm::dot(dir0, dir1);
        if (ret < 0.0) {
            int a = 0;
            a++;
        }
        return ret;
    };

    for (int i = 0; i < vertices.size() / 2 - 1; i++) {

        int idx0 = i;
        int idx1 = i + 1;
        while(true) {
            if (idx0 == 0) {
                break;
            }
            float ret = compute_ret(idx0, idx1, 0);
            if (ret >= 0.0) {
                break;
            }
            vertices[idx0 * 2].set_data((float*)vertices[idx1 * 2].get_data(),
                                vertices[idx1 * 2].get_size());
            idx0--;
        }
    }

    for (int i = 0; i < vertices.size() / 2 - 1; i++) {

        int idx0 = i;
        int idx1 = i + 1;
        while(true) {
            if (idx0 == 0) {
                break;
            }
            float ret = compute_ret(idx0, idx1, 1);
            if (ret >= 0.0) {
                break;
            }
            float* ptr_pre = (float*)vertices[idx0 * 2 + 1].get_data();

            vertices[idx0 * 2 + 1].set_data((float*)vertices[idx1 * 2 + 1].get_data(),
                                vertices[idx1 * 2 + 1].get_size());
            float* ptr_nxt = (float*)vertices[idx0 * 2 + 1].get_data();
            idx0--;

        }
    }

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    for (int i = 0; i < vertices.size() / 2 - 1; i++) {
        uint16_t base = i * 2;
        // triangles.emplace_back(Mesh::Triangle{base + 0, base + 2, base + 1});
        // triangles.emplace_back(Mesh::Triangle{base + 1, base + 2, base + 3});

        triangles.emplace_back(DynamicMesh::Triangle{(uint16_t)(base + 0), (uint16_t)(base + 1), (uint16_t)(base + 2)});
        triangles.emplace_back(DynamicMesh::Triangle{(uint16_t)(base + 1), (uint16_t)(base + 3), (uint16_t)(base + 2)});
    }

    std::shared_ptr<Material> mat = this->add_component<Material>();
    mat->set_light_enabled(false);
    mat->update_texture(TEXTURE_TYPE_DIFFUSE, tex);
}
