//
// Created by root on 2022/11/18.
//

#pragma once

#include <vector>
#include <scenegraph/node.h>
#include <scenegraph/geometry/line_path_node.h>
#include <scenegraph/geometry/rect_node.h>
#include "message.h"

namespace hmi_app {
    class PolygonNode : public zr::sg::Node {
        public:
        PolygonNode(const std::string& name,std::vector<HdMapMessage::TargetLaneInfo> test_info, std::shared_ptr<zr::sg::Texture> tex);
        float Dot( glm::vec2 point1, glm::vec2 point2);



        };


}


