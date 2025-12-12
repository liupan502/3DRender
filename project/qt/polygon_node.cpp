//
// Created by root on 2022/11/18.
//

#include "polygon_node.h"

#include <scenegraph/components/mesh.h>
#include <scenegraph/components/material.h>
#include "scenegraph/node.h"


#include "message.h"
#include <vector>
#include <glm/glm.hpp>
using namespace hmi_app;


void format_target_lane_infos(std::vector<HdMapMessage::TargetLaneInfo>& target_lane_infos) {
    assert(target_lane_infos.size() == 2);

    auto right_first_pos = target_lane_infos[0].info_date[0];
    auto right_last_pos = target_lane_infos[0].info_date[target_lane_infos[0].info_date.size() - 1];
    auto left_first_pos = target_lane_infos[1].info_date[0];
    auto left_last_pos = target_lane_infos[1].info_date[target_lane_infos[1].info_date.size() - 1];

    auto first_dis = glm::length(left_first_pos - right_first_pos);
    auto last_dis = glm::length(left_last_pos - right_last_pos);

    if (first_dis < last_dis) {
        return;
    }

    for (uint16_t i = 0; i < target_lane_infos.size(); i++) {
        HdMapMessage::TargetLaneInfo& target_lane_info = target_lane_infos[i];
        std::vector<glm::vec2> tmp;
        for (uint16_t i = 0; i < target_lane_info.info_date.size(); i++) {
            tmp.emplace_back(target_lane_info.info_date[target_lane_info.info_date.size() - i - 1]);
        }
        target_lane_info.info_date = tmp;
    }
}

PolygonNode::PolygonNode(const std::string& name,std::vector<HdMapMessage::TargetLaneInfo>  test_info,std::shared_ptr<zr::sg::Texture> tex) :
        Node(name){

    format_target_lane_infos(test_info);

    std::shared_ptr<zr::sg::DynamicMesh> mesh = this->add_component<zr::sg::DynamicMesh>();
    std::vector<std::vector<zr::sg::VertexAttribute>> vtx_attrs(1, std::vector<zr::sg::VertexAttribute>());
    vtx_attrs[0].emplace_back(zr::sg::VertexAttribute(5 * sizeof(float), 0, zr::sg::VERTEX_ATTRI_POS));
    vtx_attrs[0].emplace_back(zr::sg::VertexAttribute{5 * sizeof(float), sizeof(float) * 3, zr::sg::VERTEX_ATTRI_UV});
    mesh->set_vtx_attrs(vtx_attrs);
    std::vector<zr::sg::DynamicMesh::Vertex>& vertices = mesh->get_vertices();
    float data_buf[5] = {0.0f};
    // Mesh::Vertex vtx;
    memset(data_buf, 0, sizeof(float) * 5);
   // data_buf[0] = -half_width;
    //data_buf[1] = -half_height;

    uint16_t max_right_index = 0;
    float right_distance = 0.0;


    int Lenght=test_info[0].info_date.size()/2;

    for (int i = 0; i < test_info[0].info_date.size(); ++i) {
            memset(data_buf, 0, sizeof(float) * 5);
            data_buf[0]=test_info[0].info_date[i].x;
            data_buf[1]=test_info[0].info_date[i].y;
            data_buf[3]=0;
            data_buf[4]=i/Lenght ;
        vertices.emplace_back(zr::sg::DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

        if (i == 0) {
            continue;
        }
        auto tmp = test_info[0].info_date[i] - test_info[0].info_date[i - 1];
        float distance = glm::sqrt(glm::dot(tmp, tmp));
        right_distance += distance;
        if (right_distance >= 60) {
            max_right_index = i + 1;
            // break;
        }
    }
    int Lenght_temp=test_info[1].info_date.size()/2;
    uint16_t max_left_index = test_info[1].info_date.size();
    float left_distance = 0.0;
    for (int i = 0; i < test_info[1].info_date.size(); ++i) {
        memset(data_buf, 0, sizeof(float) * 5);
        data_buf[0]=test_info[1].info_date[i].x;
        data_buf[1]=test_info[1].info_date[i].y;
        data_buf[3]=1;
        data_buf[4]=i/Lenght_temp;


        vertices.emplace_back(zr::sg::DynamicMesh::Vertex(data_buf, 5 * sizeof(float)));

        if (i == 0) {
            continue;
        }
        auto tmp = test_info[1].info_date[i] - test_info[1].info_date[i - 1];
        float distance = glm::sqrt(glm::dot(tmp, tmp));
        left_distance += distance;
        if (left_distance >= 60) {
            max_left_index = i + 1;
            // break;
        }
    }




    std::vector<zr::sg::DynamicMesh::Triangle>& triangles = mesh->get_triangles();
    uint16_t info_date_size=test_info[0].info_date.size();

    uint16_t left_size = test_info[1].info_date.size();
    uint16_t right_size = test_info[0].info_date.size();
    // uint16_t left_size = max_left_index;
    // uint16_t right_size = max_right_index;
    uint16_t left_idx = 0;
    uint16_t right_idx = 0;





    auto add_triangle = [&triangles](const std::vector<uint16_t>& indices) {
        triangles.emplace_back(zr::sg::DynamicMesh::Triangle{indices[0], indices[1], indices[2]});
    };

    bool use_left = true;

    auto use_nxt_left_idx = [&]() {
        add_triangle({(uint16_t)(left_idx + right_size), right_idx, (uint16_t)(left_idx + 1 + right_size)});
        printf("%d %d %d\n", -left_idx, right_idx, -left_idx - 1);
        use_left = true;
        ++left_idx;
    };

    auto use_nxt_right_idx = [&]() {
        add_triangle({(uint16_t)(left_idx + right_size), right_idx, (uint16_t)(right_idx + 1)});
        printf("%d %d %d\n", -left_idx, right_idx, right_idx + 1);
        use_left = false;
        ++right_idx;


    };
    while(true) {
        if (left_idx + 1 == left_size && right_idx + 1 == right_size) {
            break;
        }

        if (left_idx + 1 == left_size) {
            use_nxt_right_idx();
            auto tmp_left_pos = test_info[1].info_date[left_idx];
            auto tmp_right_pos = test_info[0].info_date[right_idx];
            auto offset = tmp_right_pos - tmp_left_pos;
            auto len = glm::length(offset);
            if (len > 10) {
                break;
            }
            continue;
        }

        if (right_idx + 1 == right_size) {

            use_nxt_left_idx();
            auto tmp_left_pos = test_info[1].info_date[left_idx];
            auto tmp_right_pos = test_info[0].info_date[right_idx];
            auto offset = tmp_right_pos - tmp_left_pos;
            auto len = glm::length(offset);
            if (len > 10) {
                break;
            }
            continue;
        }

        uint16_t nxt_left_idx = left_idx + 1;
        uint16_t nxt_right_idx = right_idx + 1;

        // for init
        if (left_idx == 0 && right_idx == 0) {
            use_nxt_left_idx();
            continue;
        }

        auto pre_pos = use_left ? test_info[1].info_date[left_idx - 1] : test_info[0].info_date[right_idx - 1];
        auto left_pos = test_info[1].info_date[left_idx];
        auto right_pos = test_info[0].info_date[right_idx];
        auto nxt_left_pos = test_info[1].info_date[nxt_left_idx];
        auto nxt_right_pos = test_info[0].info_date[nxt_right_idx];

        auto lv1 = glm::normalize(nxt_left_pos - left_pos);
        auto lv2 = glm::normalize(right_pos - left_pos);
        auto ld = glm::dot(lv1, lv2);

        auto rv1 = glm::normalize(nxt_right_pos - right_pos);
        auto rv2 = glm::normalize(left_pos - right_pos);
        auto rd = glm::dot(rv1, rv2);

        if (ld > rd) {
            use_nxt_left_idx();
        }
        else {
            use_nxt_right_idx();
        }

        /*float param_a = left_pos.y - right_pos.y;
        float param_b = right_pos.x - left_pos.x;
        float param_c = left_pos.x * right_pos.y - left_pos.y * right_pos.x;

        float tmp_val1 = param_a * pre_pos.x + param_b * pre_pos.y + param_c;
        float tmp_val2 = param_a * nxt_left_pos.x + param_b * nxt_left_pos.y + param_c;
        float tmp_val3 = param_a * nxt_right_pos.x + param_b * nxt_right_pos.y + param_c;

        // use right
        if (tmp_val1 * tmp_val2 > 0) {

            use_nxt_right_idx();
            continue;
        }

        // use left
        if (tmp_val1 * tmp_val3 > 0) {

            use_nxt_left_idx();
            continue;
        }

        // use right
        if (abs(tmp_val2) > abs(tmp_val3)) {

            use_nxt_right_idx();
            use_nxt_left_idx();
            continue;
        }
        // use left
        else {

            use_nxt_left_idx();
            use_nxt_right_idx();
            continue;
        }*/
    }

    std::shared_ptr<zr::sg::Material> mat = this->add_component<zr::sg::Material>();
    mat->set_light_enabled(false);
    mat->update_texture(zr::sg::TEXTURE_TYPE_DIFFUSE, tex);
}

