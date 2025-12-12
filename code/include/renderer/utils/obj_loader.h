//
// Created by zhida.ji1 on 2022/8/22.
//

#pragma once
#include <memory>
#include <scenegraph/scene.h>
#include "file_helper.h"
// #include <scenegraph/geometry/obj_node.h>
#include <string>

namespace tinyobj{
    class ObjReader;
    struct shape_t;
}

namespace zr{
    namespace sg{
        class ObjNode;
    }
    namespace utils{

        class ObjLoader{
#ifdef WIN32
            friend class zr::sg::ObjNode;
#else
            friend class __attribute__((unused)) zr::sg::ObjNode;
#endif 

          
        public:
            std::shared_ptr<sg::Scene> create_scene(const char* obj_dir, const char *obj_name, const char *mtl_name);

        protected:
            std::string load_content(const char* file_path);

            void create_node(const tinyobj::ObjReader& reader,
                             const tinyobj::shape_t& shape,
                             std::shared_ptr<sg::Scene> scene,
                             const char* obj_dir);
            void create_mesh(const tinyobj::ObjReader& reader,
                             const tinyobj::shape_t& shape,
                             std::shared_ptr<sg::Node> node);
            void create_material(const tinyobj::ObjReader& reader,
                                 const tinyobj::shape_t& shape,
                                 std::shared_ptr<sg::Node> node,
                                 const char* obj_dir);
        };
    }
}


