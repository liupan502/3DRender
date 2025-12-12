//
// Created by zhida.ji1 on 2022/10/18.
//

#include "message.h"
using namespace hmi_app;

Message::Message(const std::string content) {
    Json::Reader json_reader;
    _parse_success = json_reader.parse(content, _root);
}
RelativeLocalizationMessage::RelativeLocalizationMessage(const std::string& content) : Message(content) {
    if (_parse_success) {
        init();
    }
}

void RelativeLocalizationMessage::init() {
    if (_root["List"].size()<1)
    {
        return;
    }
    Json::Value val = _root["List"][0];
    _yaw = val["yaw"].asFloat();
    _pos.x = val["x"].asFloat();
    _pos.y = val["y"].asFloat();
    _pos.z = 0.0f;
    _pos.w = 1.0f;
}

FusionObjectsMessage::FusionObjectsMessage(const std::string& content) : Message(content) {
    if (_parse_success) {
        init();
    }
}

void FusionObjectsMessage::init(){
    uint32_t num=_root["List"].size();
    for (int i = 0; i < num; i++) {
        Json::Value val = _root["List"][i];
        FusionObjectInfo info;

        info.pos.x = val["p"][0].asFloat();
        info.pos.y = val["p"][1].asFloat();

        info.col=val["col"].asInt();
        info.id= val["id"].asInt();
        info.yaw = val["yaw"].asFloat();
        info.v=val["v"].asFloat();

        info.size.x=val["s"][0].asFloat();
        info.size.y=val["s"][1].asFloat();
        info.size.z=val["s"][2].asFloat();

        info.type=val["t"].asInt();

        _infos.emplace_back(info);
    }
}

TrafficLightsMessage::TrafficLightsMessage(const std::string& content) : Message(content) {
    if (_parse_success) {
        init();
    }
}

void TrafficLightsMessage::init() {
    for (int i = 0; i < _root["List"]["geo"].size(); i++) {
        TrafficLightInfo info;
        Json::Value val = _root["List"]["geo"];
        info.pos.x=val[i][0].asFloat();
        info.pos.y=val[i][1].asFloat();
        _infos.emplace_back(info);
    }
}


PlanningTrajectoryMessage::PlanningTrajectoryMessage(const std::string& content) : Message(content) {
    if (_parse_success) {
        init();
    }
    else {
        _geo.clear();
    }
}

void PlanningTrajectoryMessage::init() {
    if (_root["List"].size()<1)
    {
        return;
    }
    Json::Value geo = _root["List"][0]["geo"];
    for (uint32_t i = 0; i < geo.size(); i++) {
        glm::vec2 point;
        if (geo[i].size()<2)
        {
            return;
        }
        point.x = geo[i][0].asFloat();
        point.y = geo[i][1].asFloat();

        if(i==0){
            _temp_point=point.x;
            _geo.emplace_back(point);
            continue;
        }
        if(abs(_temp_point-geo[i][0].asFloat())<0.1&&i!=geo.size()){
            continue;
        }
        _geo.emplace_back(point);
        _temp_point=point.x;
    }
}

std::map<std::string, HdMapMessage::HdMapMessageType> HdMapMessage::type_map = {
        {"map.stop_line" , HdMapMessage::HdMapMessageType::HDMAP_MESSAGE_TYPE_STOP_LINE},
        {"map.road_high_light", HDMAP_MESSAGE_TYPE_TARGET_LANE},
        { "map.cross_walk", HDMAP_MESSAGE_TYPE_CROSS_WALK},
        {"map.boundary", HDMAP_MESSAGE_TYPE_BOUNDARY},
        {"map.idm_boundary", HDMAP_MESSAGE_TYPE_IDM_BOUNDARY},
        {"map.road_mark", HDMAP_MESSAGE_TYPE_ROAD_MARK},
};



HdMapMessage::HdMapMessage(const std::string& content_key, const std::string& content) : Message(content) {
    HdMapMessageType type = HdMapMessage::type_map[content_key];
    switch(type) {
        case HDMAP_MESSAGE_TYPE_STOP_LINE:

            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_STOP_LINE;
            //process_stop_line();
            break;
        case HDMAP_MESSAGE_TYPE_TARGET_LANE:
            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_TARGET_LANE;
            process_target_lane();
            break;
        case HDMAP_MESSAGE_TYPE_CROSS_WALK:
            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_CROSS_WALK;
          //  process_cross_walk();
            break;
        case HDMAP_MESSAGE_TYPE_BOUNDARY:

            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_BOUNDARY;
            process_boundary();
            break;
        case HDMAP_MESSAGE_TYPE_IDM_BOUNDARY:
             hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_IDM_BOUNDARY;
          //  process_idm_boundary();
            break;
        case HDMAP_MESSAGE_TYPE_ROAD_MARK:
            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_ROAD_MARK;
          //  process_road_mark();
            break;
        case HDMAP_MESSAGE_TYPE_CAR_BOX:
            hdmap_message_node_type=HDMAP_NODE_MESSAGE_TYPE_ROAD_MARK;
            //  process_road_mark();
            break;

        default:
            break;
    }
}
void HdMapMessage::process_stop_line() {
    for (int i = 0; i < _root["List"].size(); i++) {
        StopLineInfo stop_line_info;
        Json::Value pos  = _root["List"][i]["geo"];
        for (int j = 0; j < pos.size(); j++) {
            glm::vec2 point;
            point.x = pos[i][0].asFloat();
            point.y = pos[i][1].asFloat();
            stop_line_info.info_date.emplace_back(point);
        }
        _stop_line_infos.emplace_back(stop_line_info);
    }
}

void HdMapMessage::process_boundary() {
    _boundary_infos.clear();
    for (uint32_t k = 0; k <_root["List"].size(); ++k) {
        BoundaryInfo boundary_info;
        Json::Value mark  = _root["List"][k]["mark"];
        Json::Value type  = _root["List"][k]["t"];
        Json::Value pos  = _root["List"][k]["geo"];
        boundary_info.info_mark= mark.asInt();
        boundary_info.info_type= type.asInt();
        for (uint32_t i = 0; i < pos.size(); i++) {
            glm::vec2 point;
            point.x = pos[i][0].asFloat();
            point.y = pos[i][1].asFloat();
            boundary_info.info_date.emplace_back(point);
        }
        _boundary_infos.emplace_back(boundary_info);
    }
}

void HdMapMessage::process_target_lane()
{
    _target_lane_infos.clear();
    int col=_root["col"].asInt();
    if (col==0)
    {
        return;
    };
    //Target_line_info.info_col=col
    for (uint32_t k = 0; k <_root["List"].size(); ++k) {
        TargetLaneInfo Target_line_info;
        Json::Value pos  = _root["List"][k]["geo"];
        Target_line_info.info_col=col;
        if (k == 0) {
            for (uint32_t i = 0; i < pos.size(); i++) {
                glm::vec2 point;
                point.x = pos[pos.size() - i - 1][0].asFloat();
                point.y = pos[pos.size() - i - 1][1].asFloat();

                if (Target_line_info.info_date.size()) {
                    auto last_point = Target_line_info.info_date[Target_line_info.info_date.size() - 1];
                    if (last_point.x == point.x && last_point.y == point.y) {
                        continue;
                    }
                }

                Target_line_info.info_date.emplace_back(point);
            }
        }
        else {
            for (uint32_t i = 0; i < pos.size(); i++) {
                glm::vec2 point;
                point.x = pos[i][0].asFloat();
                point.y = pos[i][1].asFloat();

                if (Target_line_info.info_date.size()) {
                    auto last_point = Target_line_info.info_date[Target_line_info.info_date.size() - 1];
                    if (last_point.x == point.x && last_point.y == point.y) {
                        continue;
                    }
                }

                Target_line_info.info_date.emplace_back(point);

            }
        }

//        std::vector<glm::vec2> tmp;
//        for (uint16_t i = 0; i < Target_line_info.info_date.size(); i++) {
//            tmp.emplace_back(Target_line_info.info_date[Target_line_info.info_date.size() - i - 1]);
//        }
//        Target_line_info.info_date = tmp;
        _target_lane_infos.emplace_back(Target_line_info);
    }


}

void HdMapMessage::process_road_mark()
{}

CarFrameMessage::CarFrameMessage(const std::string& content_key, const std::string& content) : Message(content) {
    Car_Framep_Position();
}

void CarFrameMessage::Car_Framep_Position(){
     Json::Value v = _root["List"][0]["v"];
    Json::Value yaw = _root["List"][0]["yaw"];
    Json::Value geo = _root["List"][0]["p"];
    glm::vec2 v2;
    Json::Value pos  = _root["List"][0]["p"];
    v2[0]=pos[0].asFloat();
    v2[1]=pos[1].asFloat();
    target_pos_Info.pos_date=v2;
    target_pos_Info.yaw=yaw.asFloat();
}




PerceptionApaSlotsMessage::PerceptionApaSlotsMessage(const std::string& content) : Message(content) {
    if (_parse_success) {
        init();
    }
}

void PerceptionApaSlotsMessage::init() {
    for (int i = 0; i < _root["List"].size(); i++) {
        Json::Value val = _root["List"][i];
        PerceptionApaInfo info;
        info.id=val["id"].asInt();
        if(val["recommendID"]==" "){
            info.recommendID=0;
        }
        else{
            info.recommendID=atoi(val["recommendID"].asString().c_str());
        }
        info.pos.x=val["p"][0].asFloat();
        info.pos.y=val["p"][1].asFloat();
        info.size.x=val["s"][0].asFloat();
        info.size.y=val["s"][1].asFloat();
        info.size.z=val["s"][2].asFloat();
        info.yaw=val["yaw"].asFloat();
        info.status=val["status"].asInt();
        info.type=val["t"].asInt();
        info.pos_z=0;
        _infos.emplace_back(info);
    }
    float pos_z=0.001f;
    for (int i = 0; i < _infos.size(); i++) {
        _infos[i].pos_z=pos_z;
        pos_z+=0.002f;
        if(pos_z>0.02)
            pos_z=0.001f;
    }
}

std::map<std::string, SceneStatusMessage::SceneType> SceneStatusMessage::type_map = {
        {"apa_state" , SceneStatusMessage::SceneType::SCENE_TYPE_APA},
        {"nzp_state" , SceneStatusMessage::SceneType::SCENE_TYPE_NZP},
        {"cruise_speed" , SceneStatusMessage::SceneType::SCENE_TYPE_SPEED},
};
SceneStatusMessage::SceneStatusMessage(const std::string &content_key, const std::string &content):Message(content)  {
    _scene_type = SceneStatusMessage::type_map[content_key];
    _status_val = (uint32_t)atoi(content.c_str());
    if(_scene_type!=SCENE_TYPE_SPEED)
        ++_status_val;
}

SceneStatusMessage::NzpState SceneStatusMessage::get_nzp_status() const {
    if (_scene_type != SCENE_TYPE_NZP) {
        return NZP_STATE_NONE;
    }
    return (NzpState)(_status_val);
}

SceneStatusMessage::ApaState SceneStatusMessage::get_apa_status() const {
    if (_scene_type != SCENE_TYPE_APA) {
        return APA_STATE_NONE;
    }
    return (ApaState)(_status_val);
}

DistancePercentageMessage::DistancePercentageMessage(const std::string &content_key,const std::string &content):Message(content) {
    if(content_key=="DistancePercentage"){
        _distance=(float)atof(content.c_str());
    }

}

ApaObstacleMessage::ApaObstacleMessage(const std::string &content_key, const std::string &content): Message(content) {
    if (_parse_success) {
        init();
    }
}

void ApaObstacleMessage::init() {
    for (int i = 0; i < _root["List"].size(); i++) {
        Json::Value val = _root["List"][i];
        ApaObstacleInfo info;

        info.pos.x = val["p"][0].asFloat();
        info.pos.y = val["p"][1].asFloat();

        info.id= val["id"].asInt();
        info.yaw = val["yaw"].asFloat();
        info.v=val["v"].asFloat();

        info.size.x=val["s"][0].asFloat();
        info.size.y=val["s"][1].asFloat();
        info.size.z=val["s"][2].asFloat();

        info.t=val["t"].asInt();

        _infos.emplace_back(info);
    }
}


