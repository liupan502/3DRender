#pragma once

#include <scenegraph/node.h>
#include <glm/glm.hpp>

namespace zr {
    namespace sg {

        class Texture;
        class DynamicMesh;
        class GeometryNode : public Node {
            public:
            // tri_num 表示预期使用的三角片的最大数量，这不是硬约束，只是为了降低内存申请和释放的频率
            GeometryNode(const std::string& name, std::shared_ptr<Texture> tex, uint16_t tri_num = 1000);

            

            // 添加一个矩形shape，类似rectnode
            void add_rect(const glm::mat4& transform_mat, float width, float height, glm::vec2 uv_offset, glm::vec2 uv_range);

            // 添加一个line path shape, 类似linepathnode 
            void add_line_path(const glm::mat4& transform_mat, const std::vector<float> line_path_data, float width,
                        glm::vec2 uv_offset, glm::vec2 uv_range,
                        bool use_tex_len = false, float tex_len = 0.0f);

            // 以下为使用vertex color 的构造函数和相关方法
            GeometryNode(const std::string& name, uint16_t tri_num = 1000);

            // 添加一个矩形shape，类似rectnode, 使用vertex color
            void add_rect(const glm::mat4& transform_mat, float width, float height, const glm::vec4& color);

            // 添加一个line path shape, 类似linepathnode， 使用 vertex color  
            void add_line_path(const glm::mat4& transform_mat, 
                const std::vector<float> line_path_data, float width, const glm::vec4& color);

            // 清空shape 数据，可以重新添加shape 元素            
            void reset();            
            protected:
            std::shared_ptr<DynamicMesh> get_dynamic_mesh();


            void compute_data_buf(const glm::mat4& transform_buf, float data_buf[5], glm::vec4 pos, glm::vec2 uv) const;

            void compute_data_buf(const glm::mat4& transform_buf, float data_buf[7], glm::vec4 pos, glm::vec4 color) const;

            void compute_uv(glm::vec2& uv, glm::vec2 uv_offset, glm::vec2 uv_range) const;

            private:
            uint16_t _tri_num;
            bool _use_vertex_color;
        };
    }
}
