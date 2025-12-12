//
// Created by zhida.ji1 on 2022/10/18.
//

#pragma once
#include <string>
#include <vector>
#include <map>
#include <json/json.h>
#include <glm/glm.hpp>
namespace hmi_app{
    class Message{
    public:
        Message(const std::string content);

    protected:
        bool _parse_success;
        Json::Value _root;
    };

    class RelativeLocalizationMessage : public Message{
    public:
        RelativeLocalizationMessage(const std::string& content = "");
        inline float get_yaw() const { return _yaw;};
        inline glm::vec4 get_pos() const { return _pos;};
    private:
        void init();
    private:
        float _yaw;
        glm::vec4  _pos;
    };

    class FusionObjectsMessage : public Message {
        struct FusionObjectInfo{
            glm::vec2 pos;
            int  col;
            int id;
            float yaw;
            float v;
            glm::vec3 size;
            unsigned int type;
        };
    public:
        FusionObjectsMessage(const std::string& content = "");

        inline std::vector<FusionObjectInfo> get_fusion_object_infos() const { return _infos;};
    private:
        void init();
    private:
        std::vector<FusionObjectInfo> _infos;
    };

    class TrafficLightsMessage : public Message{
        struct TrafficLightInfo{
            glm::vec2 pos;
            glm::vec4 bbox;
            unsigned int color;
            unsigned int shape;
            unsigned int blink;
        };
    public:
        TrafficLightsMessage(const std::string& content = "");
        inline std::vector<TrafficLightInfo> get_traffic_light_infos() const { return _infos;};
    private:
        void init();
    private:
        std::vector<TrafficLightInfo> _infos;
    };

    class PlanningTrajectoryMessage : public Message {
    public:
        PlanningTrajectoryMessage(const std::string& content = "");
        inline const std::vector<glm::vec2>& get_geo() const { return _geo; };

    private:
        void init();
    private:
        std::vector<glm::vec2> _geo;
        float _temp_point=0;
    };

    class CarFrameMessage:public Message
    {
       public:
        CarFrameMessage(const std::string& content_key, const std::string& content = "");

      public:
        struct TargetposInfo
        {
            int id;
            float yaw;
            glm::vec2 pos_date;
        };
      public:
        inline const TargetposInfo get_target_pos_infos() const { return target_pos_Info; };
    private:
        TargetposInfo target_pos_Info;
    private:
        void Car_Framep_Position();

    };

    class HdMapMessage : public Message{
        static std::vector<std::string> high_light_messages;
    public:
        enum HdMapMessageNodeType {
            HDMAP_NODE_MESSAGE_TYPE_NONE,
            HDMAP_NODE_MESSAGE_TYPE_STOP_LINE,
            HDMAP_NODE_MESSAGE_TYPE_TARGET_LANE,
            HDMAP_NODE_MESSAGE_TYPE_CROSS_WALK,
            HDMAP_NODE_MESSAGE_TYPE_BOUNDARY,
            HDMAP_NODE_MESSAGE_TYPE_IDM_BOUNDARY,
            HDMAP_NODE_MESSAGE_TYPE_ROAD_MARK,
            HDMAP_NODE_MESSAGE_TYPE_MAX
        };
        enum HdMapBoundaryType
        {
            HDMAP_NODE_MESSAGE_BOUNDARY_FULL_LINE,
            HDMAP_NODE_MESSAGE_BOUNDARY_IMAGINARY_LINE,
            HDMAP_NODE_MESSAGE_BOUNDARY_LEFT_REAL_RIGHT,
        };
        HdMapMessageNodeType  hdmap_message_node_type;
        struct StopLineInfo{

            std::vector<glm::vec2> info_date;
        };

        struct TargetLaneInfo{
            int info_col;
            std::vector<glm::vec2>info_date;

        };
        struct CrossWalkInfo{
            int info_type;
            int info_mark;
            std::vector<glm::vec2>info_date;
        };

        struct BoundaryInfo{
            int info_type;
            int info_mark;
            std::vector<glm::vec2>info_date;
        };
        struct IdmBoundaryInfo{
            int info_type;
            int info_mark;
            std::vector<glm::vec2>info_date;
        };
        struct RoadMarkInfo{
            int info_type;
            int info_mark;
            std::vector<glm::vec2>info_date;
        };
    private:
        enum HdMapMessageType {
            HDMAP_MESSAGE_TYPE_NONE,
            HDMAP_MESSAGE_TYPE_STOP_LINE,
            HDMAP_MESSAGE_TYPE_TARGET_LANE,
            HDMAP_MESSAGE_TYPE_CROSS_WALK,
            HDMAP_MESSAGE_TYPE_BOUNDARY,
            HDMAP_MESSAGE_TYPE_IDM_BOUNDARY,
            HDMAP_MESSAGE_TYPE_ROAD_MARK,
            HDMAP_MESSAGE_TYPE_CAR_BOX,
            HDMAP_MESSAGE_TYPE_MAX
        };
    private:
        static std::map<std::string, HdMapMessageType> type_map;
    public:
        HdMapMessage(const std::string& content_key, const std::string& content = "");

        inline std::vector<StopLineInfo> get_stop_line_infos() { return _stop_line_infos; };
        inline const std::vector<TargetLaneInfo> get_target_lane_infos() const { return _target_lane_infos; };
        inline  std::vector<CrossWalkInfo> get_cross_walk_infos() { return _cross_walk_infos; };

        inline const std::vector<BoundaryInfo> get_boundary_infos()const { return _boundary_infos; };
        inline std::vector<IdmBoundaryInfo> get_idm_boundary_infos() { return _idm_boundary_infos; };
        inline std::vector<RoadMarkInfo> get_road_mark_infos() { return _road_mark_infos; };
    private:

        void process_stop_line();

        void process_target_lane();
        void process_cross_walk();
        void process_boundary();
        void process_idm_boundary();
        void process_road_mark();


    private:
        std::vector<StopLineInfo> _stop_line_infos;
        std::vector<TargetLaneInfo> _target_lane_infos;
        std::vector<CrossWalkInfo> _cross_walk_infos;
        std::vector<BoundaryInfo> _boundary_infos;
        std::vector<IdmBoundaryInfo> _idm_boundary_infos;
        std::vector<RoadMarkInfo> _road_mark_infos;

    private:
        void init();

    };

    class PerceptionApaSlotsMessage : public Message {
    public:
        struct PerceptionApaInfo{
            int32_t id;
            int32_t recommendID;
            glm::vec2 pos;
            glm::vec3 size;
            float yaw;
            uint32_t status;
            uint8_t type;
            float pos_z;
        };
    public:
        PerceptionApaSlotsMessage(const std::string& content = "");
        inline std::vector<PerceptionApaInfo> get_perception_apa_infos() const{return _infos;};
    private:
        void init();
    private:
        std::vector<PerceptionApaInfo> _infos;
    };

    class ApaObstacleMessage:public Message{
    public:
        struct ApaObstacleInfo{
            uint8_t id;
            glm::vec2 pos;
            glm::vec3 size;
            int t;
            int v;
            float yaw;
        };
    public:
        ApaObstacleMessage(const std::string& content_key="", const std::string& content = "");
        inline std::vector<ApaObstacleInfo> get_apa_obstacle_infos() const{return _infos;};
    private:
        void init();
    private:
        std::vector<ApaObstacleInfo> _infos;
    };

    class SceneStatusMessage:public Message{
    public:
        enum SceneType{
            SCENE_TYPE_NZP,
            SCENE_TYPE_APA,
            SCENE_TYPE_SPEED
    };
        enum NzpState{
            NZP_STATE_NONE = 0,
            NZP_STATE_INITIAL = 1,
            NZP_STATE_OFF = 2,
            NZP_STATE_STANDBY = 3,
            NZP_STATE_ACTIVE = 4,
            NZP_STATE_OVERRIDE_LON = 5,
            NZP_STATE_OVERRIDE_LAT = 6,
            NZP_STATE_STAND_WAIT = 7,
            NZP_STATE_MAX
        };
        enum ApaState{
            APA_STATE_NONE = 0,
            APA_STATE_INITIAL = 1,
            APA_STATE_OFF = 2,
            APA_STATE_STANDBY = 3,
            APA_STATE_SEARCHING = 4,
            APA_STATE_PRE_PARK = 5,
            APA_STATE_PARKING = 6,
            APA_STATE_PARK_COMPLETE = 7,
            APA_STATE_SUSPEND = 8,
            APA_STATE_MAX
        };
    public:
        SceneStatusMessage(const std::string& content_key="", const std::string& content = "");
    public:
        NzpState get_nzp_status() const;
        ApaState get_apa_status() const;

    private:
        static std::map<std::string, SceneType> type_map;
        uint32_t _status_val;
        SceneType _scene_type;
    };

    class DistancePercentageMessage:public Message{
    public:
        DistancePercentageMessage(const std::string& content_key="", const std::string& content = "");
        inline float get_distance() const{return _distance;};
    private:
          float _distance=0.0f;
    };

    template <class T>
    class MessageHandler {
    public:
        virtual void handle_message(const T & msg) {};
    };

    typedef MessageHandler<RelativeLocalizationMessage> RelativeLocalizationMessageHandler;
    typedef MessageHandler<FusionObjectsMessage> FusionObjectsMessageHandler;
    typedef MessageHandler<TrafficLightsMessage> TrafficLightsMessageHandler;
    typedef MessageHandler<PlanningTrajectoryMessage> PlanningTrajectoryMessageHandler;
    typedef MessageHandler<HdMapMessage> HdMapMessageHandler;
    typedef MessageHandler<PerceptionApaSlotsMessage> PerceptionApaSlotsMessageHandler;
    typedef MessageHandler<CarFrameMessage> CarFrameMessageHandler;
    typedef MessageHandler<SceneStatusMessage> SceneStatusMessageHandler;
    typedef MessageHandler<DistancePercentageMessage> DistancePercentageMessageHandler;
    typedef MessageHandler<ApaObstacleMessage> ApaObstacleMessageHandler;
}