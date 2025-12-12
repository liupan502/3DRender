//
// Created by zhida.jiq on 2022/9/6.
//

#pragma once
#include <scenegraph/scene_graph.h>
#include <common/car.h>
#include <json/json.h>

using namespace zr;
namespace hmi_app {
struct  ParkingSpaceInfo{
    ParkingSpaceInfo(const Json::Value& val);
    int id;
    int status;
    float yaw;
    float c_x;
    float c_y;
    float width;
    float height;
};
class ParkingSpace: public sg::RectNode {
public:
    ParkingSpace(const ParkingSpaceInfo& info );
    void update(const ParkingSpaceInfo& info);
    inline bool is_dirty() const { return _is_dirty;};
    inline void set_is_dirty(bool is_dirty) {_is_dirty = is_dirty;};
private:
    bool _is_dirty{false};
    std::shared_ptr<Car> _car;
};
}
