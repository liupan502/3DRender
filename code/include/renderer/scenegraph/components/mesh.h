//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <scenegraph/node.h>
#include <scenegraph/component.h>

namespace rhi{
    class Buffer;
}
namespace zr {

    namespace sg {

        enum VertexAttributeType {
            VERTEX_ATTRI_NONE = 0,
            VERTEX_ATTRI_POS = 1,
            VERTEX_ATTRI_NORMAL = 1 << 1,
            VERTEX_ATTRI_UV = 1 << 2,
            VERTEX_ATTRI_COLOR = 1 << 3,
            VERTEX_ATTRI_JOINT_8 = 1 << 4,
            VERTEX_ATTRI_JOINT_16 = 1 << 5,
            VERTEX_ATTRI_WEIGHT = 1 << 6,
            VERTEX_ATTRI_MAX = 1 << 31
        };

        class VertexAttribute {
        public:
            VertexAttribute(uint32_t stride = 0, uint32_t offset = 0, VertexAttributeType type = VERTEX_ATTRI_POS, uint32_t buf_idx = 0);
            uint32_t stride = 0;
            uint32_t offset = 0;
            uint32_t location = 0;
            uint32_t buf_idx = 0;
            VertexAttributeType type;

            bool operator < (const VertexAttribute& other) const;
        };

        class Mesh : public Component{

        public:
            virtual std::type_index get_type() const override;
            virtual ~Mesh() = default;

            virtual void upload_data() = 0;

            inline std::vector<std::shared_ptr<rhi::Buffer>> get_vtx_buffers() { return _vtx_buffers;} ;
            inline std::shared_ptr<rhi::Buffer> get_index_buffer() { return _index_buffer;};
            inline uint32_t  get_indice_count(){ return _indice_count;};
            inline void set_indice_count(uint32_t indice_count) { _indice_count = indice_count;};

            Mesh(Node* node) : Component(node) {};
            inline void set_vtx_attrs(std::vector<std::vector<VertexAttribute>> vtx_attrs) { _vtx_attrs = vtx_attrs;};
            std::vector<std::vector<VertexAttribute>>  get_vtx_attrs() { return _vtx_attrs;};
        
            rhi::RenderPrimitive render_primitive() const;
        protected:
            std::vector<std::shared_ptr<rhi::Buffer>> _vtx_buffers;
            std::shared_ptr<rhi::Buffer> _index_buffer;
            uint32_t _indice_count{0};

            std::vector<std::weak_ptr<Node>> _nodes;

            std::vector<std::vector<VertexAttribute>> _vtx_attrs;
        };

        class StaticMesh : public Mesh {
        public:
            struct MeshBuffer{
                const char* ptr;
                uint32_t  len;
            };

        public:
            StaticMesh(Node* node) : Mesh(node) {};
            virtual void upload_data() override;
            inline void add_vtx_mesh_buffer(MeshBuffer vtx_mesh_buffer) { _vtx_mesh_buffers.emplace_back(vtx_mesh_buffer);};
            inline void set_indice_mesh_buffer(MeshBuffer indice_mesh_buffer) { _indice_mesh_buffer = indice_mesh_buffer;};
        private:
            std::vector<MeshBuffer> _vtx_mesh_buffers;
            MeshBuffer _indice_mesh_buffer;
        };

        class DynamicMesh : public Mesh{
        public:
            class Vertex{
            public:
                Vertex() {};
                Vertex(const float* data, uint32_t data_len);
                Vertex(Vertex&& vtx);
                void set_data(const float* data, uint32_t vertex_attrs);
                int get_size() const { return _size;};
                const char* get_data() {return (const char*) buf;};
                ~Vertex() {
                    if (buf) {
                        delete [] buf;
                        buf = nullptr;
                    }
                }
            private:
                float* buf{nullptr};
                uint32_t _size{0};
            };
            struct Triangle{
                uint16_t indices[3];
            };

        public:
            DynamicMesh(Node* node) : Mesh(node), _tri_size(2), 
                    _is_dirty(true), _buf(nullptr) {};
            inline std::vector<Vertex>& get_vertices() { return _vertices;};
            inline std::vector<Triangle>& get_triangles() { return _triangles;};
            virtual void upload_data() override;
            virtual ~DynamicMesh();
            void reset();
            void set_tri_size(uint16_t tri_size);
            inline uint32_t get_tri_size() const { return _tri_size; };
        protected:
            bool need_upload();  
            void recreate_bufs();
            void update_data();  
        private:
            std::vector<Vertex> _vertices;
            std::vector<Triangle> _triangles;
            uint32_t _tri_size;
            bool _is_dirty;
            uint8_t* _buf;
        };
    }
}

