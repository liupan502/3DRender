//
// Created by zhida.ji1 on 2022/10/9.
//

#pragma once
#include "scenegraph/node.h"
#include "scenegraph/components/mesh.h"
#include <string>
#include <map>
#include "cgltf/cgltf.h"



namespace zr{
    namespace sg{

        class AnimationChannel;
        class Skin;
        class Texture;

        class GltfNode : public Node{
        public:
            GltfNode(const std::string& path, const std::string& material_file_path = "",const std::string& name = "");
            virtual ~GltfNode();
            inline bool is_valid() const { return _is_valid; };
            // virtual
            bool save_external_material();

            inline std::string get_gltf_path() const { return _gltf_path; };

            inline std::string get_material_path() const { return _material_path; };
        protected:
            bool parse_gltf(const std::string& path);
        private:
            bool parse_scene(const cgltf_scene* scene);
            bool parse_node(const cgltf_node* gltf_node, std::shared_ptr<Node> sg_node);
            bool try_parse_mesh(const cgltf_node *gltf_node, std::shared_ptr<Node> sg_node);

            bool try_parse_transform(const cgltf_node* gltf_node, std::shared_ptr<Node> sg_node);
            bool try_parse_weight(const cgltf_node* gltf_node, std::shared_ptr<Node> sg_node);
            bool try_parse_skins();
            void parse_skin(const cgltf_skin& skin, std::shared_ptr<sg::Skin> sg_skin);
            bool try_parse_material(const cgltf_material *gltf_material, std::shared_ptr<Node> sg_node);
            bool parse_attribute(const cgltf_attribute& attribute, std::shared_ptr<Mesh> sg_mesh);
            StaticMesh::MeshBuffer get_mesh_buffer(const cgltf_buffer_view* buf_view);
            void add_vtx_bufs(std::shared_ptr<StaticMesh> sg_mesh);


            void try_parse_animations(const cgltf_node* node, std::shared_ptr<sg::Node> sg_node);
            void parse_animation_channel(const cgltf_animation_channel& channel,
                                         std::shared_ptr<AnimationChannel> sg_channel);
            const std::string& get_uri_data(const std::string& uri);
            std::string create_sub_node_name(uint32_t idx);
            uint32_t get_node_idx(const cgltf_node* node);
            std::shared_ptr<sg::Texture> get_texture(cgltf_texture* gltf_tex);

            bool apply_external_material(const std::string& material_path);




        private:
            bool _is_valid{false};
            cgltf_data* _gltf_data{nullptr};
            std::map<cgltf_buffer_view*, uint32_t> _buf_view_idx_map;
            std::vector<std::vector<VertexAttribute>> _vertex_attrs;
            std::vector<cgltf_accessor*> _cgltf_indice;
            std::string _base_dir;
            std::string _material_path;
            std::string _gltf_path;
            std::map<std::string, std::string> _external_data_map;
            std::vector<std::vector<std::shared_ptr<sg::Node>>> _skin_nodes_arr;
            std::map<std::string, std::shared_ptr<sg::Texture>> _tex_map;
        };
    }
}

