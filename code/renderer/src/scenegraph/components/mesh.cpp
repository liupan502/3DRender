//
// Created by zhida.ji1 on 2022/8/8.
//
#include <string.h>
#include <scenegraph/components/mesh.h>
#include <core/core.h>
#include <utils/log.h>
using namespace zr::sg;

std::type_index Mesh::get_type() const {
    return std::type_index(typeid(Mesh));
}

DynamicMesh::~DynamicMesh() {
    if (_buf) {
        delete [] _buf;
        _buf = nullptr;
    }
}

void DynamicMesh::set_tri_size(uint16_t tri_size) {
    if (_has_uploaded) {
        return;
    }
    _tri_size = tri_size;
}

void DynamicMesh::reset() {
    _vertices.clear();
    _triangles.clear();
    _is_dirty = true;
}

bool DynamicMesh::need_upload() {
    if (_triangles.size() == 0) {
        return false;
    }

    if (_triangles.size() >= _tri_size) {
        _tri_size = _triangles.size() * 2;
        _has_uploaded = false;
        return true;
    }

    if (!_has_uploaded) {
        return true;
    }

    return false;
}

void DynamicMesh::recreate_bufs(std::shared_ptr<core::Device> device) {
    VkDeviceSize idx_device_size = sizeof(Triangle) * _tri_size;
    _index_buffer = std::make_shared<core::Buffer>(device, idx_device_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    uint32_t vertex_size = _vertices[0].get_size();
    VkDeviceSize vtx_device_size = vertex_size * _tri_size * 3;
    _vtx_buffers.clear();
    std::shared_ptr<core::Buffer> vtx_buffer = std::make_shared<core::Buffer>(device, vtx_device_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    _vtx_buffers.emplace_back(vtx_buffer);

    if (_buf) {
        delete [] _buf;
    }
    _buf = new uint8_t [_vertices[0].get_size() * _tri_size * 3];
}

void DynamicMesh::update_data(std::shared_ptr<core::Device> device) {
    VkDeviceSize idx_device_size = sizeof(Triangle) * _triangles.size();
    if(!_index_buffer) {
        LOGD("_index_buffer NULL");
    }
    _index_buffer->update((const uint8_t*)(_triangles.data()), idx_device_size);

    uint32_t vertex_size = _vertices[0].get_size();
    VkDeviceSize vtx_device_size = vertex_size * _vertices.size();
    for (int i = 0; i < _vertices.size(); i++) {
        memcpy(_buf + i * vertex_size, _vertices[i].get_data(), vertex_size);
    }
    _vtx_buffers[0]->update((const uint8_t*)(_buf), vtx_device_size);
}

void DynamicMesh::upload_data(std::shared_ptr<core::Device> device) {
    if (!_is_dirty) {
        return;
    }

    if (need_upload()) {
        recreate_bufs(device);
    }
    
    update_data(device);
    _indice_count = _triangles.size() * 3;
    _is_dirty = false;
    _has_uploaded = true;
}

/*void DynamicMesh::upload_data(std::shared_ptr<core::Device> device) {
    if (_has_uploaded) {
        return;
    }
    _vtx_buffers.clear();
    // upload indices data
    _indice_count = _triangles.size() * 3;
    VkDeviceSize device_size = sizeof(Triangle) * _triangles.size();
    _index_buffer = std::make_shared<core::Buffer>(device, device_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    _index_buffer->update((const uint8_t*)(_triangles.data()), device_size);

    // upload vertex data
    uint32_t vertex_size = _vertices[0].get_size();
    VkDeviceSize vtx_device_size = vertex_size * _vertices.size();
    std::shared_ptr<core::Buffer> vtx_buffer = std::make_shared<core::Buffer>(device, vtx_device_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    uint8_t* buf = new uint8_t [_vertices[0].get_size() * _vertices.size()];
    for (int i = 0; i < _vertices.size(); i++) {
        memcpy(buf + i * vertex_size, _vertices[i].get_data(), vertex_size);
    }
    vtx_buffer->update((const uint8_t*)(buf), vtx_device_size);
    _vtx_buffers.emplace_back(vtx_buffer);
    delete [] buf;
    _has_uploaded = true;
}*/

void DynamicMesh::Vertex::set_data(const float *data, uint32_t data_len) {
    if (buf) {
        delete [] buf;
    }

    _size = data_len;
    buf = new float[data_len];
    memcpy(buf, data, _size);
}

DynamicMesh::Vertex::Vertex(const float *data, uint32_t data_len) {
    set_data(data, data_len);
}

DynamicMesh::Vertex::Vertex(Vertex &&vtx) {
    buf = vtx.buf;
    vtx.buf = nullptr;
    _size = vtx.get_size();
    vtx._size = 0;
}

VertexAttribute::VertexAttribute(uint32_t stride, uint32_t offset, VertexAttributeType type, uint32_t buf_idx) {
    this->stride = stride;
    this->offset = offset;
    this->type = type;
    this->buf_idx = buf_idx;
    switch(type) {
        case VertexAttributeType::VERTEX_ATTRI_POS:
            this->location = 0;
            break;
        case VertexAttributeType::VERTEX_ATTRI_UV:
            this->location = 1;
            break;
        case VertexAttributeType::VERTEX_ATTRI_NORMAL:
            this->location = 2;
            break;
        case VertexAttributeType::VERTEX_ATTRI_WEIGHT:
            this->location = 3;
            break;
        case VertexAttributeType::VERTEX_ATTRI_JOINT_8:
        case VertexAttributeType::VERTEX_ATTRI_JOINT_16:
            this->location = 4;
            break;
        case VertexAttributeType::VERTEX_ATTRI_COLOR:
            this->location = 5;
            break;
        default:
            this->location = 0;
    }
}

bool VertexAttribute::operator<(const VertexAttribute &other) const {
    if (this->location < other.location) {
        return true;
    }
    else if (this->location > other.location) {
        return false;
    }

    if (this->type < other.type) {
        return true;
    }
    else if (this->type > other.type) {
        return false;
    }

    if (this->offset < other.offset) {
        return true;
    }
    else if (this->offset < other.offset) {
        return false;
    }

    if (this->stride < other.stride) {
        return true;
    }
    else if (this->stride > other.stride) {
        return false;
    }

    return false;
}

void StaticMesh::upload_data(std::shared_ptr<core::Device> device) {
    if (_has_uploaded) {
        return;
    }
    _vtx_buffers.clear();


    uint16_t * idx_ptr = (uint16_t*) (_indice_mesh_buffer.ptr);

    // upload indices data
    {
        VkDeviceSize device_size = _indice_mesh_buffer.len;
        _index_buffer = std::make_shared<core::Buffer>(device, device_size,
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        _index_buffer->update((const uint8_t *) (_indice_mesh_buffer.ptr), device_size);
    }

    for (uint32_t i = 0; i < _vtx_mesh_buffers.size(); i++) {
        VkDeviceSize device_size = _vtx_mesh_buffers[i].len;
        std::shared_ptr<core::Buffer> vtx_buf = std::make_shared<core::Buffer>(device, device_size,
                                                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        uint8_t  a = sizeof(char);
        vtx_buf->update((const uint8_t *) (_vtx_mesh_buffers[i].ptr), device_size);
        _vtx_buffers.emplace_back(vtx_buf);
    }
    _has_uploaded = true;
}
