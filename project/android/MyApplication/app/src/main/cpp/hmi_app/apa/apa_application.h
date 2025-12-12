//
// Created by zhida.ji1 on 2022/9/6.
//

#pragma once
#include <common/application.h>
#include "parking_space.h"
#include <scenegraph/scene_graph.h>
#include <vector>
#include <map>
namespace hmi_app{
    class ApaApplication: public Application {
    public:
        ApaApplication(const std::string& name) : Application(name){};
        virtual void update(const std::string& json_content) override;
        virtual bool init(AAssetManager* asset_mgr, ANativeWindow* window) override;
        void update_geo(const std::string& json_content);
        void update_self_car(const std::string& json_content);
        void map_pt_from_space_to_screen(float space_pt[3], float screen_pt[2]);

        // event
        void on_clicked(float x, float y);
    private:
        void dirty_all_elements();

        void init_self_car();

    private:
        std::map<int, std::shared_ptr<ParkingSpace>> _parking_spaces;
        std::shared_ptr<sg::LinePathNode> _geo;
        std::shared_ptr<Car> _self_car;
    };
}
