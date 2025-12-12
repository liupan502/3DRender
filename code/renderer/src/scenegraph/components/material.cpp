//
// Created by root on 2022/8/8.
//

#include <scenegraph/components/material.h>
#include <core/buffer.h>
#include <string.h>
#include <utils/file_helper.h>
#include <iostream>
#include <fstream>

using namespace zr::sg;

std::type_index Material::get_type() const
{
    return std::type_index(typeid(Material));
}

void Material::update_texture(TextureType type, std::shared_ptr<Texture> tex)
{
    if (_texs.find(type) == _texs.end())
    {
        _texs.insert(std::make_pair(type, tex));
    }
    else
    {
        _texs[type] = tex;
    }
}

void Material::upload_data(std::shared_ptr<core::Device> device)
{

    for (auto tex : _texs)
    {
        tex.second->upload_data(device);
    }

    if (_pbr_params_buf == nullptr)
    {
        _pbr_params_buf = std::make_shared<zr::core::UniformBuffer>(14, device, sizeof(PbrParams));
        _pbr_params_buf->update((uint8_t *)(&_pbr_params), sizeof(_pbr_params));
    }

    if (_material_frag_uniform_buf == nullptr)
    {
        _material_frag_uniform_buf = std::make_shared<core::UniformBuffer>(18,
                                                                           device, sizeof(FragmentUniformBufferObject));
        _material_frag_uniform_buf->update((uint8_t *)(&_frag_uniform_buf_obj), sizeof(_frag_uniform_buf_obj));
    }

    if (_has_uploaded)
    {
        return;
    }

    _has_uploaded = true;
}

void Material::update()
{

    if (_pbr_params_buf)
    {
        _pbr_params_buf->update((uint8_t *)(&_pbr_params), sizeof(_pbr_params));
    }

    if (_material_frag_uniform_buf)
    {
        _material_frag_uniform_buf->update((uint8_t *)(&_frag_uniform_buf_obj),
                                           sizeof(_frag_uniform_buf_obj));
    }
}

std::shared_ptr<Texture> Material::get_tex(TextureType type)
{
    if (_texs.find(type) != _texs.end())
    {
        return _texs[type];
    }
    return std::shared_ptr<Texture>();
}

Material::Material(Node *node) : Component(node)
{
    _ability = MaterialAbility();
    _ability.reset();
    _ability.enable_light = 1;
    _ability.enable_blend = 1;
    _ability.enable_depth = 1;
    _pbr_params = PbrParams();
    _pbr_params_buf = nullptr;

    memset(&_frag_uniform_buf_obj, 0, sizeof(_frag_uniform_buf_obj));
    _frag_uniform_buf_obj.jitter_info.x = 1.0f;
}

void Material::set_light_enabled(bool enabled)
{
    _ability.enable_light = enabled ? 1 : 0;
}

bool Material::get_light_enabled() const
{
    return _ability.enable_light ? true : false;
}

void Material::set_skin_enabled(bool enabled)
{
    _ability.enable_skin = enabled ? 1 : 0;
}

bool Material::get_skin_enabled() const
{
    return _ability.enable_skin ? true : false;
}

void Material::set_color_enabled(bool enabled)
{
    _ability.enable_color = enabled ? 1 : 0;
}

bool Material::get_color_enabled() const
{
    return _ability.enable_color ? true : false;
}

void Material::set_pbr_params(PbrParams params)
{
    _pbr_params = params;
}

void Material::set_environment_enabled(bool enabled)
{
    _ability.enable_environment = enabled ? 1 : 0;
}
bool Material::get_environment_enabled()
{
    return _ability.enable_environment;
}

bool Material::get_transparent_enabled() const
{
    return _ability.enable_transparent ? true : false;
}

void Material::set_transparent_enabled(bool enabled)
{
    _ability.enable_transparent = enabled ? 1 : 0;
}

bool Material::get_blend_enabled() const
{
    return _ability.enable_blend ? true : false;
}

void Material::set_blend_enabled(bool enabled)
{
    _ability.enable_blend = enabled ? 1 : 0;
}

int Material::get_cull_mode() const
{
    return _ability.cull_mode;
}

void Material::set_cull_mode(int cull_mode)
{
    _ability.cull_mode = cull_mode;
}

bool Material::get_depth_enabled() const
{
    return _ability.enable_depth ? true : false;
}

void Material::set_depth_enabled(bool enabled)
{
    _ability.enable_depth = enabled ? 1 : 0;
}

bool Material::save(const std::string &path)
{
    Json::Value mat = tojson();
    Json::StreamWriterBuilder writer;
    std::string jsonString = Json::writeString(writer, mat);
    std::stringstream ss;
    ss << utils::FileHelper().assert_base_dir << path ;
    // std::cout << ss.str()<< std::endl;
    std::ofstream outputFile(ss.str()); // 打开输出文件
    if (outputFile.is_open())
    {                             // 检查文件是否成功打开
        outputFile << jsonString; // 将字符串写入文件
        outputFile.close();       // 关闭文件
        return true;
    }
    else
    {
        return false;
    }
}

Json::Value Material::tojson()
{
    Json::Value mat(Json::objectValue); // 创建一个空的 JSON 对象
    auto pbr_params = get_pbr_params();
    Json::Value pbr(Json::arrayValue); // 创建一个空的 JSON 对象
    pbr.append(pbr_params.metallic);
    pbr.append(pbr_params.roughness);
    pbr.append(pbr_params.ior);
    pbr.append(pbr_params.clear_coat);
    pbr.append(pbr_params.clear_coat_roughness);
    mat["pbrParams"] = pbr;
    mat["isTransparent"] = get_transparent_enabled();
    mat["isEnvironmentEnabled"] = get_environment_enabled();
    auto emissive_color = get_emissive_color();
    Json::Value color(Json::arrayValue); // 创建一个空的 JSON 对象
    color.append(emissive_color.x);
    color.append(emissive_color.y);
    color.append(emissive_color.z);
    color.append(emissive_color.w);
    mat["emissiveColor"] = color;
    return mat;
}
void Material::loadjson(Json::Value &json_mat)
{
    auto json_pbr_params = json_mat.get("pbrParams", Json::Value::null);
    if (json_pbr_params != Json::Value::null)
    {
        sg::Material::PbrParams &pbr_params = get_pbr_params();
        pbr_params.metallic = json_pbr_params[0].asFloat();
        pbr_params.roughness = json_pbr_params[1].asFloat();
        pbr_params.ior = json_pbr_params[2].asFloat();
        if (json_pbr_params.size() == 5)
        {
            pbr_params.clear_coat = json_pbr_params[3].asFloat();
            pbr_params.clear_coat_roughness = json_pbr_params[4].asFloat();
        }
    }

    auto json_is_transparent = json_mat.get("isTransparent", Json::Value::null);
    if (json_is_transparent != Json::Value::null)
    {
        set_transparent_enabled(json_is_transparent.asBool());
    }
    auto json_is_env_enabled = json_mat.get("isEnvironmentEnabled", Json::Value::null);
    if (json_is_env_enabled != Json::Value::null)
    {
        set_environment_enabled(json_is_env_enabled.asBool());
    }
    else
    {
        set_environment_enabled(false);
    }
    auto json_emissive_color = json_mat.get("emissiveColor", Json::Value::null);
    if (json_emissive_color != Json::Value::null)
    {
        glm::vec4 color(json_emissive_color[0].asFloat(), json_emissive_color[1].asFloat(), json_emissive_color[2].asFloat(), json_emissive_color[3].asFloat());
        set_emissive_color(color);
    }
}
bool Material::load(const std::string &path)
{
    std::cout << path << std::endl;
    std::string content = utils::FileHelper().load_content(path.c_str());
    Json::Value json_mat;
    Json::Reader jr;
    if (jr.parse(content, json_mat))
    {
        loadjson(json_mat);
        return true;
    }
    return false;
}
