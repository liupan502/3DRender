#include <scenegraph/geometry/geometry_node.h>
#include <scenegraph/components/mesh.h>
#include <scenegraph/components/material.h>
#include <memory>
#include <string.h>

using namespace zr::sg;

GeometryNode::GeometryNode(const std::string& name, std::shared_ptr<Texture> tex, uint16_t tri_num) : Node(name) {
    auto mat = add_component<Material>();
    mat->update_texture(TextureType::TEXTURE_TYPE_DIFFUSE, tex);
    mat->set_light_enabled(false);
    _tri_num = tri_num;
    _use_vertex_color = false;
}

GeometryNode::GeometryNode(const std::string& name, uint16_t tri_num) : Node(name) {
    auto mat = add_component<Material>();
    mat->set_color_enabled(true);
    mat->set_light_enabled(false);
    _tri_num = tri_num;
    _use_vertex_color = true;
}

void GeometryNode::reset() {
    get_dynamic_mesh()->reset();
}

std::shared_ptr<DynamicMesh> GeometryNode::get_dynamic_mesh() {
    auto mesh = get_component<DynamicMesh>();
    if (!mesh) {
        mesh = add_component<DynamicMesh>();
        mesh->set_tri_size(_tri_num);
        std::vector<std::vector<VertexAttribute>> vtx_attrs(1, std::vector<VertexAttribute>());
        if (_use_vertex_color) {
            vtx_attrs[0].emplace_back(VertexAttribute(7 * sizeof(float), 0, VERTEX_ATTRI_POS));
            vtx_attrs[0].emplace_back(VertexAttribute{ 7 * sizeof(float), sizeof(float) * 3, VERTEX_ATTRI_COLOR });
        }
        else {
            vtx_attrs[0].emplace_back(VertexAttribute(5 * sizeof(float), 0, VERTEX_ATTRI_POS));
            vtx_attrs[0].emplace_back(VertexAttribute{ 5 * sizeof(float), sizeof(float) * 3, VERTEX_ATTRI_UV });
        }
        
        mesh->set_vtx_attrs(vtx_attrs);
    }
    return mesh;
}

void GeometryNode::compute_data_buf(const glm::mat4& transform_mat,
                                                   float data_buf[5], glm::vec4 pos, glm::vec2 uv) const {
    pos = transform_mat * pos;
    data_buf[0] = pos.x;
    data_buf[1] = pos.y;
    data_buf[2] = pos.z;
    data_buf[3] = uv.x;
    data_buf[4] = uv.y;
}

void GeometryNode::compute_data_buf(const glm::mat4& transform_mat, float data_buf[7], glm::vec4 pos, glm::vec4 color) const {
    pos = transform_mat * pos;
    data_buf[0] = pos.x;
    data_buf[1] = pos.y;
    data_buf[2] = pos.z;
    data_buf[3] = color.x;
    data_buf[4] = color.y;
    data_buf[5] = color.z;
    data_buf[6] = color.w;

}

void GeometryNode::compute_uv(glm::vec2& uv, glm::vec2 uv_offset, glm::vec2 uv_range) const {
    uv.x = uv.x * uv_range.x + uv_offset.x;
    uv.y = uv.y * uv_range.y + uv_offset.y;
}

void GeometryNode::add_rect(const glm::mat4& transform_mat, float width, float height, glm::vec2 uv_offset, glm::vec2 uv_range) {
    // 此方法在 use vertex color 时无效
    if (_use_vertex_color) {
        return;
    }
    auto mesh = get_dynamic_mesh();

    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();

    uint16_t vtx_offset = vertices.size();
    float half_width = width * 0.5f;
    float half_height = height * 0.5;
    float data_buf[5] = {0.0f};

    float offset = 0.0f;

    glm::vec4 pos = glm::vec4(-half_width, -half_height, 0.0, 1.0f);
    glm::vec2 uv = glm::vec2(offset, offset);
    compute_uv(uv, uv_offset, uv_range);
    compute_data_buf(transform_mat,data_buf, pos, uv);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

    pos = glm::vec4(half_width, -half_height, 0.0, 1.0f);
    uv = glm::vec2(1.0 - offset, offset);
    compute_uv(uv, uv_offset, uv_range);
    compute_data_buf(transform_mat,data_buf, pos, uv);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

    pos = glm::vec4(half_width, half_height, 0.0, 1.0f);
    uv = glm::vec2(1.0 - offset, 1.0f - offset);
    compute_uv(uv, uv_offset, uv_range);
    compute_data_buf(transform_mat,data_buf, pos, uv);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

    pos = glm::vec4(-half_width, half_height, 0.0, 1.0f);
    uv = glm::vec2(offset, 1.0 - offset);
    compute_uv(uv, uv_offset, uv_range);
    compute_data_buf(transform_mat,data_buf, pos, uv);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    triangles.emplace_back(DynamicMesh::Triangle{
                               (uint16_t)(0 + vtx_offset), (uint16_t)(1 + vtx_offset), (uint16_t)(2 + vtx_offset)});
    triangles.emplace_back(DynamicMesh::Triangle{
                               (uint16_t)(0 + vtx_offset), (uint16_t)(2 + vtx_offset), (uint16_t)(3 + vtx_offset)});

//    triangles.emplace_back(DynamicMesh::Triangle{
//            (uint16_t)(2 + vtx_offset), (uint16_t)(1 + vtx_offset), (uint16_t)(0 + vtx_offset)});
//    triangles.emplace_back(DynamicMesh::Triangle{
//            (uint16_t)(3 + vtx_offset), (uint16_t)(2 + vtx_offset), (uint16_t)(0 + vtx_offset)});
}

void GeometryNode::add_rect(const glm::mat4& transform_mat, float width, float height, const glm::vec4& color) {

    // 此方法在 !use vertex color 时无效
    if (!_use_vertex_color) {
        return;
    }

    auto mesh = get_dynamic_mesh();

    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();

    uint16_t vtx_offset = vertices.size();
    float half_width = width * 0.5f;
    float half_height = height * 0.5;
    float data_buf[7] = { 0.0f };

    float offset = 0.0f;

    glm::vec4 pos = glm::vec4(-half_width, -half_height, 0.0, 1.0f);
    
    compute_data_buf(transform_mat, data_buf, pos, color);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

    pos = glm::vec4(half_width, -half_height, 0.0, 1.0f);
    
    compute_data_buf(transform_mat, data_buf, pos, color);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

    pos = glm::vec4(half_width, half_height, 0.0, 1.0f);
    compute_data_buf(transform_mat, data_buf, pos, color);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

    pos = glm::vec4(-half_width, half_height, 0.0, 1.0f);
    compute_data_buf(transform_mat, data_buf, pos, color);
    vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    triangles.emplace_back(DynamicMesh::Triangle{
                               (uint16_t)(0 + vtx_offset), (uint16_t)(1 + vtx_offset), (uint16_t)(2 + vtx_offset) });
    triangles.emplace_back(DynamicMesh::Triangle{
                               (uint16_t)(0 + vtx_offset), (uint16_t)(2 + vtx_offset), (uint16_t)(3 + vtx_offset) });
}

void GeometryNode::add_line_path(const glm::mat4& transform_mat, const std::vector<float> line_path_data,
                                 float width, glm::vec2 uv_offset, glm::vec2 uv_range,
                                 bool use_tex_len, float tex_len) {

    // 此方法在 use vertex color 时无效
    if (_use_vertex_color) {
        return;
    }

    int point_count = line_path_data.size() / 2;
    std::shared_ptr<DynamicMesh> mesh = get_dynamic_mesh();
    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();
    uint16_t vtx_offset = vertices.size();

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

            glm::vec4 pos = glm::vec4(x - 0.5 * width * normal_x, y - 0.5 * width * normal_y, 0.0, 1.0f);
            glm::vec2 uv = glm::vec2(0.0, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            pos = glm::vec4(x + 0.5 * width * normal_x, y + 0.5 * width * normal_y, 0.0, 1.0f);
            uv = glm::vec2(1.0f, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
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

            glm::vec4 pos = glm::vec4(x - 0.5 * width * normal_x, y - 0.5 * width * normal_y, 0.0, 1.0f);
            glm::vec2 uv = glm::vec2(0.0, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            pos = glm::vec4(x + 0.5 * width * normal_x, y + 0.5 * width * normal_y, 0.0, 1.0f);
            uv = glm::vec2(1.0f, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
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

            glm::vec4 pos = glm::vec4(x - flag * offset_x, y - flag * offset_y, 0.0, 1.0f);
            glm::vec2 uv = glm::vec2(0.0, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            pos = glm::vec4(x + flag * offset_x, y + flag * offset_y, 0.0, 1.0f);
            uv = glm::vec2(1.0f, use_tex_len ? total_len / tex_len : i * 1.0f/ (point_count-1));
            compute_uv(uv, uv_offset, uv_range);
            compute_data_buf(transform_mat,data_buf, pos, uv);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

            pre_normal_x = normal_x;
            pre_normal_y = normal_y;
        }

    }

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    for (int i = 0; i < (vertices.size() - vtx_offset) / 2 - 1; i++) {
        uint16_t base = i * 2 + vtx_offset;
        // triangles.emplace_back(Mesh::Triangle{base + 0, base + 2, base + 1});
        // triangles.emplace_back(Mesh::Triangle{base + 1, base + 2, base + 3});

        triangles.emplace_back(DynamicMesh::Triangle{(uint16_t)(base + 2), (uint16_t)(base + 1), (uint16_t)(base + 0)});
        triangles.emplace_back(DynamicMesh::Triangle{(uint16_t)(base + 2), (uint16_t)(base + 3), (uint16_t)(base + 1)});
    }
}

void GeometryNode::add_line_path(const glm::mat4& transform_mat,
    const std::vector<float> line_path_data, float width, const glm::vec4& color) {

    // 此方法在 !use vertex color 时无效
    if (!_use_vertex_color) {
        return;
    }

    int point_count = line_path_data.size() / 2;
    std::shared_ptr<DynamicMesh> mesh = get_dynamic_mesh();
    std::vector<DynamicMesh::Vertex>& vertices = mesh->get_vertices();
    uint16_t vtx_offset = vertices.size();

    float pre_normal_x = 0.0f;
    float pre_normal_y = 0.0f;
    float data_buf[7] = { 0.0f };
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

            glm::vec4 pos = glm::vec4(x - 0.5 * width * normal_x, y - 0.5 * width * normal_y, 0.0, 1.0f);
            
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

            pos = glm::vec4(x + 0.5 * width * normal_x, y + 0.5 * width * normal_y, 0.0, 1.0f);
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));
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

            glm::vec4 pos = glm::vec4(x - 0.5 * width * normal_x, y - 0.5 * width * normal_y, 0.0, 1.0f);
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

            pos = glm::vec4(x + 0.5 * width * normal_x, y + 0.5 * width * normal_y, 0.0, 1.0f);
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));
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

            glm::vec4 pos = glm::vec4(x - flag * offset_x, y - flag * offset_y, 0.0, 1.0f);
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

            pos = glm::vec4(x + flag * offset_x, y + flag * offset_y, 0.0, 1.0f);
            compute_data_buf(transform_mat, data_buf, pos, color);
            vertices.emplace_back(DynamicMesh::Vertex(data_buf, 7 * sizeof(float)));

            pre_normal_x = normal_x;
            pre_normal_y = normal_y;
        }

    }

    std::vector<DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    for (int i = 0; i < (vertices.size() - vtx_offset) / 2 - 1; i++) {
        uint16_t base = i * 2 + vtx_offset;
        // triangles.emplace_back(Mesh::Triangle{base + 0, base + 2, base + 1});
        // triangles.emplace_back(Mesh::Triangle{base + 1, base + 2, base + 3});

        triangles.emplace_back(DynamicMesh::Triangle{ (uint16_t)(base + 2), (uint16_t)(base + 1), (uint16_t)(base + 0) });
        triangles.emplace_back(DynamicMesh::Triangle{ (uint16_t)(base + 2), (uint16_t)(base + 3), (uint16_t)(base + 1) });
    }
}
