#pragma once
#include <multipass_renderer.h>
#include <GLFW/glfw3.h>
#include <scenegraph/components/camera.h>
#include <scenegraph/geometry/geometry_node.h>
#include <scenegraph/geometry/gltf_node.h>

#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QMenuBar>
#include <toolwindow.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <functional>
#include <QVBoxLayout>
#include <control/materal.h>
#include <json/json.h>
#include <chrono>
#include <QDebug>
using namespace zr;
using namespace std::chrono;
namespace editor
{
    enum NodeType
    {
        None,
        Gltf,
        SubGltf,
        Geometry,
        ReadOnly,
        Root
    };
    struct Geometry_Rect
    {
        float width;
        float height;
        glm::mat4 transform = glm::mat4(1.0);
        glm::vec2 uv_offset;
        glm::vec2 uv_range;

        Json::Value tojson() const
        {
            Json::Value root;
            root["width"] = width;
            root["height"] = height;

            // Json::Value transformJson;
            // for (int i = 0; i < 4; ++i)
            // {
            //     Json::Value colJson;
            //     colJson.append(transform[0][i]);
            //     colJson.append(transform[1][i]);
            //     colJson.append(transform[2][i]);
            //     colJson.append(transform[3][i]);
            //     transformJson.append(colJson);
            // }
            // root["transform"] = transformJson;

            Json::Value uvOffsetJson;
            uvOffsetJson.append(uv_offset.x);
            uvOffsetJson.append(uv_offset.y);
            root["uv_offset"] = uvOffsetJson;

            Json::Value uvRangeJson;
            uvRangeJson.append(uv_range.x);
            uvRangeJson.append(uv_range.y);
            root["uv_range"] = uvRangeJson;

            return root;
        }
        void loadjson(const Json::Value &root)
        {
            width = root["width"].asInt();
            height = root["height"].asInt();

            // 解析并填充 transform 数组
            // Json::Value transformJson = root["transform"];
            // for (int i = 0; i < 4; ++i)
            // {
            //     Json::Value colJson = transformJson[i];
            //     transform[0][i] = colJson[0].asFloat();
            //     transform[1][i] = colJson[1].asFloat();
            //     transform[2][i] = colJson[2].asFloat();
            //     transform[3][i] = colJson[3].asFloat();
            // }

            // 解析并填充 uv_offset 数组
            Json::Value uvOffsetJson = root["uv_offset"];
            uv_offset.x = uvOffsetJson[0].asFloat();
            uv_offset.y = uvOffsetJson[1].asFloat();

            // 解析并填充 uv_range 数组
            Json::Value uvRangeJson = root["uv_range"];
            uv_range.x = uvRangeJson[0].asFloat();
            uv_range.y = uvRangeJson[1].asFloat();
        }
    };
    struct Geometry_LinePath
    {
        std::vector<float> line_path_data;
        glm::mat4 transform = glm::mat4(1.0);
        float width;
        glm::vec2 uv_offset;
        glm::vec2 uv_range;
        bool use_tex_len;
        float tex_len;
        Json::Value tojson() const
        {
            Json::Value root;

            Json::Value linePathDataJson;
            for (float data : line_path_data)
            {
                linePathDataJson.append(data);
            }
            root["line_path_data"] = linePathDataJson;

            // Json::Value transformJson;
            // for (int i = 0; i < 4; ++i)
            // {
            //     Json::Value colJson;
            //     colJson.append(transform[0][i]);
            //     colJson.append(transform[1][i]);
            //     colJson.append(transform[2][i]);
            //     colJson.append(transform[3][i]);
            //     transformJson.append(colJson);
            // }
            // root["transform"] = transformJson;

            root["width"] = width;

            Json::Value uvOffsetJson;
            uvOffsetJson.append(uv_offset.x);
            uvOffsetJson.append(uv_offset.y);
            root["uv_offset"] = uvOffsetJson;

            Json::Value uvRangeJson;
            uvRangeJson.append(uv_range.x);
            uvRangeJson.append(uv_range.y);
            root["uv_range"] = uvRangeJson;

            root["use_tex_len"] = use_tex_len;
            root["tex_len"] = tex_len;

            return root;
        }
        void loadjson(const Json::Value &root)
        {
            line_path_data.clear();
            const Json::Value linePathDataJson = root["line_path_data"];
            if (linePathDataJson.isArray())
            {
                for (Json::ArrayIndex i = 0; i < linePathDataJson.size(); ++i)
                {
                    float data = linePathDataJson[i].asFloat();
                    line_path_data.push_back(data);
                }
            }

            // 解析并填充 transform 数组
            // Json::Value transformJson = root["transform"];
            // for (int i = 0; i < 4; ++i)
            // {
            //     Json::Value colJson = transformJson[i];
            //     transform[0][i] = colJson[0].asFloat();
            //     transform[1][i] = colJson[1].asFloat();
            //     transform[2][i] = colJson[2].asFloat();
            //     transform[3][i] = colJson[3].asFloat();
            // }

            width = root["width"].asFloat();

            // 解析并填充 uv_offset 数组
            const Json::Value uvOffsetJson = root["uv_offset"];
            if (uvOffsetJson.isArray() && uvOffsetJson.size() >= 2)
            {
                uv_offset.x = uvOffsetJson[0].asFloat();
                uv_offset.y = uvOffsetJson[1].asFloat();
            }

            // 解析并填充 uv_range 数组
            const Json::Value uvRangeJson = root["uv_range"];
            if (uvRangeJson.isArray() && uvRangeJson.size() >= 2)
            {
                uv_range.x = uvRangeJson[0].asFloat();
                uv_range.y = uvRangeJson[1].asFloat();
            }

            use_tex_len = root["use_tex_len"].asBool();
            tex_len = root["tex_len"].asFloat();
        }
    };
    struct GeometryNodeInfo
    {
        std::string node_name;
        std::string textrue_path;
        std::vector<Geometry_Rect> rects;
        std::vector<Geometry_LinePath> linepaths;
        Json::Value tojson() const
        {
            Json::Value root;
            root["texture_path"] = textrue_path;

            root["node_name"] = node_name;

            Json::Value rectsJson;
            for (const auto &rect : rects)
            {
                rectsJson.append(rect.tojson());
            }
            root["rects"] = rectsJson;

            Json::Value linePathsJson;
            for (const auto &linepath : linepaths)
            {
                linePathsJson.append(linepath.tojson());
            }
            root["line_paths"] = linePathsJson;

            return root;
        }
        void loadjson(const Json::Value &root)
        {
            node_name = root.get("node_name", "").asString();
            textrue_path = root.get("texture_path", "").asString();

            rects.clear();
            const Json::Value rectsJson = root["rects"];
            if (rectsJson.isArray())
            {
                for (Json::ArrayIndex i = 0; i < rectsJson.size(); ++i)
                {
                    const Json::Value rectJson = rectsJson[i];
                    Geometry_Rect rect;
                    rect.loadjson(rectJson);
                    rects.push_back(rect);
                }
            }

            linepaths.clear();
            const Json::Value linePathsJson = root["line_paths"];
            if (linePathsJson.isArray())
            {
                for (Json::ArrayIndex i = 0; i < linePathsJson.size(); ++i)
                {
                    const Json::Value linepathJson = linePathsJson[i];
                    Geometry_LinePath linepath;
                    linepath.loadjson(linepathJson);
                    linepaths.push_back(linepath);
                }
            }
        }
        std::shared_ptr<sg::GeometryNode> to_node()
        {

            std::shared_ptr<zr::sg::Texture> texture = std::make_shared<zr::sg::SingleLayerTexture>(textrue_path);
            std::shared_ptr<sg::GeometryNode> node_t = std::make_shared<sg::GeometryNode>(node_name, texture);
            for (auto sub_rect : rects)
            {
                node_t->add_rect(sub_rect.transform, sub_rect.width, sub_rect.height, sub_rect.uv_offset, sub_rect.uv_range);
            }
            for (auto sub_path : linepaths)
            {
                node_t->add_line_path(sub_path.transform, sub_path.line_path_data, sub_path.width, sub_path.uv_offset, sub_path.uv_range, sub_path.use_tex_len, sub_path.tex_len);
            }
            return node_t;
        }
        void add_node_rect(std::shared_ptr<sg::GeometryNode> node, const Geometry_Rect &rect)
        {
            node->reset();
            for (auto sub_rect : rects)
            {
                node->add_rect(sub_rect.transform, sub_rect.width, sub_rect.height, sub_rect.uv_offset, sub_rect.uv_range);
            }
            for (auto sub_path : linepaths)
            {
                node->add_line_path(sub_path.transform, sub_path.line_path_data, sub_path.width, sub_path.uv_offset, sub_path.uv_range, sub_path.use_tex_len, sub_path.tex_len);
            }
            node->add_rect(rect.transform, rect.width, rect.height, rect.uv_offset, rect.uv_range);
        }
        void add_node_line_pat(std::shared_ptr<sg::GeometryNode> node, const Geometry_LinePath &line_path)
        {
            node->reset();
            for (auto sub_rect : rects)
            {
                node->add_rect(sub_rect.transform, sub_rect.width, sub_rect.height, sub_rect.uv_offset, sub_rect.uv_range);
            }
            for (auto sub_path : linepaths)
            {
                node->add_line_path(sub_path.transform, sub_path.line_path_data, sub_path.width, sub_path.uv_offset, sub_path.uv_range, sub_path.use_tex_len, sub_path.tex_len);
            }
            node->add_line_path(line_path.transform, line_path.line_path_data, line_path.width, line_path.uv_offset, line_path.uv_range, line_path.use_tex_len, line_path.tex_len);
        }
    };

    class EditorWindows
    {
    public:
        // 获取单例实例的静态方法
        static EditorWindows &getInstance()
        {
            static EditorWindows instance;
            return instance;
        }

        // 防止拷贝构造和拷贝赋值操作
        EditorWindows(const EditorWindows &) = delete;
        EditorWindows &operator=(const EditorWindows &) = delete;
        void init();

        std::function<void()> Show();
        void bing_treeView(); // 绑定场景节点
        void bing_listView()
        {
            bing_listView(_select_node);
        } // 绑定节点属性
        std::shared_ptr<sg::Camera> _camera;
        bool _isroate = false;
        void init_components();
        NodeType get_node_type(std::shared_ptr<sg::Node> node);

    private:
#ifdef WIN32
        std::string _assert_base_dir = "D:/assets/hmi_3d/";
#else

        std::string _assert_base_dir = "/home/zee001-w/Documents/code/3dR/3DRender/project/editor/assets/hmi_3d/";
#endif // DEBUG

        // std::string _assert_base_dir="assets/hmi_3d/";
        std::string _scene_config_file = _assert_base_dir + "scene.json";
        zr::MultiPassRenderer renderer;
        std::shared_ptr<sg::Scene> scene = nullptr;
        std::shared_ptr<zr::sg::GeometryNode> _tail_flame_node = nullptr;

        std::shared_ptr<zr::sg::GeometryNode> _line_node=nullptr;

        GLFWwindow *glfwWindow = nullptr;
        // 创建一个QWidget作为主窗口
        ToolWindow qtWindow;
        QVBoxLayout *_component_layout;

        QTimer timerLoop;
        void init_qt_func_window();
        void init_valukan();
        void init_scene();
        void init_gltf_window();

        void update_parameter(QStandardItem *item, const std::shared_ptr<sg::Node> node, std::vector<std::function<void()>> &update_datas);

        // 私有构造函数，防止直接实例化对象
        EditorWindows() {}
        // 私有析构函数，防止删除对象
        ~EditorWindows() {}

    private:
        void bing_listView(const std::shared_ptr<sg::Node> node); // 绑定节点属性
        void create_env_light();
        void draw_background();
        void show_input_window(QString name, QString title, QString &input);

    public:
        inline QPushButton *get_apply_button() const
        {
            return _applyButton;
        }
        inline std::shared_ptr<sg::Node> get_select_node() const
        {
            return _select_node;
        }
        inline std::function<void()> get_add_gltf_func() const
        {
            return _add_gltf_func;
        }
        inline void reset_elect_node()
        {
            _select_node.reset();
        }
        bool _is_editor = false;

        inline void add_geometry_node(std::shared_ptr<sg::GeometryNode> geom, GeometryNodeInfo info)
        {
            _geom_map.insert({geom, info});
        }
        inline std::string get_assert_base_dir()
        {
            return _assert_base_dir;
        }
        inline GeometryNodeInfo &get_geometry_node_info()
        {
            return _geom_map[std::dynamic_pointer_cast<sg::GeometryNode>(_select_node)];
        }

    private:
        QTreeView *_treeView = nullptr;      // 场景节点
        QTreeView *_listView = nullptr;      // 属性
        QPushButton *_applyButton = nullptr; // 应用
        QPushButton *_resetButton = nullptr; // 重置

        std::shared_ptr<sg::Node> _select_node = nullptr;
        std::shared_ptr<sg::DirectionalLight> _directional_light = nullptr;

        std::shared_ptr<sg::DirectionalLight> _directional_light_2 = nullptr;

        Ui::Materal *_materal_ui = nullptr;
        Ui::Transform *_transform_ui = nullptr;
        Ui::Camera *_camera_ui = nullptr;
        Ui::Directional_Light *_directional_light_ui = nullptr;
        Ui::Animation_Manager *_animation_manager_ui = nullptr;

        std::function<void()> _add_gltf_func;

        std::map<std::shared_ptr<sg::GeometryNode>, GeometryNodeInfo> _geom_map;
    };

}
