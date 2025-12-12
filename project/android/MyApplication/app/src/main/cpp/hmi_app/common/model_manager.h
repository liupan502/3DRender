//
// Created by zhida.ji1 on 2022/9/8.
//

#pragma once
#include <memory>
#include <map>
namespace zr::sg{
    class Node;
}

namespace hmi_app{
    enum ModelName{
        MODEL_NAME_NONE,
        MODEL_NAME_OTHER_CAR,
        MODEL_NAME_MAX
    };
    class ModelManager{

    public:
        static ModelManager* get_instance();

    public:
        class Destructor {
        public:
            ~Destructor();
        };

    public:
        std::shared_ptr<zr::sg::Node> get_model(ModelName type);
    private:
        ModelManager() = default;
        ~ModelManager() = default;
        void add_obj_model(ModelName type);

    private:
        std::map<ModelName, std::shared_ptr<zr::sg::Node>> _model_map;
    };

    static ModelManager* _model_mgr;
    static ModelManager::Destructor _model_mgr_destructor;
}


