//
// Created by zhida.ji1 on 2022/8/8.
//

#pragma once

#include <string>
#include <typeindex>
#include <memory>
namespace zr{
    namespace core{
        class Device;
    }
    namespace sg{

        class Node;

        class Component {

        friend class Node;
        public:
            Component(Node* node, const std::string& name = "") : _name(name), _node(node) {};
            Component(Component&& other) = default;
            Component(const Component& other) = default;
            virtual std::type_index get_type() const = 0;
            virtual ~Component() = default;

            inline const std::string& name() const { return _name;};
            inline Node* get_node() {return _node;};
            inline void set_node(Node* node) { _node = node; };
            virtual void upload_data(std::shared_ptr<core::Device> device) {};
            virtual void update() {};
        protected:
            bool _has_uploaded{false};
        private:
            std::string _name;
            Node* _node{nullptr};

        };
    }
}

