#include "editor_windows.h"
#include <utils/file_helper.h>
#include <scenegraph/geometry/gltf_node.h>

#include <fstream>
#include <QKeyEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <iostream>
#include <QDebug>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>

#define WIDTH 1024
#define HEIGHT 720
namespace editor
{

    class KeyEventFilter : public QObject
    {
    private:
        int curr_skin_index = 0;
        std::map<std::string, std::shared_ptr<zr::sg::Texture>> _tex_map;
        std::vector<std::string> mpaths{
            "images/car_paint/black.png",
            "images/car_paint/electric_blue.png",
            "images/car_paint/gray.png",
            "images/car_paint/extreme_blue.png",
            "images/car_paint/orange.png",
            "images/car_paint/white.png"};

        std::string get_skin_path(int index)
        {
            std::string path = "";
            switch (index)
            {
            case 0:
                path = "images/car_paint/white.mat";
                break;
            case 1:
                path = "images/car_paint/black.mat";
                break;
            case 2:
                path = "images/car_paint/electric_blue.mat";
                break;
            case 3:
                path = "images/car_paint/gray.mat";
                break;
            case 4:
                path = "images/car_paint/extreme_blue.mat";
                break;
            case 5:
                path = "images/car_paint/orange.mat";
                break;
            }
            return path;
        }
        std::shared_ptr<zr::sg::Texture> get_texture(std::string tex_name)
        {

            if (_tex_map.find(tex_name) == _tex_map.end())
            {
                _tex_map[tex_name] = std::make_shared<zr::sg::SingleLayerTexture>(tex_name);
            }
            return _tex_map[tex_name];
        }

    public:
        bool eventFilter(QObject *obj, QEvent *event) override
        {

            if (event->type() == QEvent::KeyPress)
            {

                QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
                auto add_to_animation_emissive_color_intensity = [](
                                                                     std::shared_ptr<zr::sg::Node> node_t, const std::string &name, float start, float end, float time, bool is_play = true, std::function<void(const std::vector<float> &vals)> func = nullptr)
                {
                    std::shared_ptr<zr::sg::Animation> animation = std::make_shared<zr::sg::Animation>(name);
                    auto channel = std::make_shared<sg::AnimationChannel>();
                    channel->set_interpolation_type(sg::ANIMATION_INTERPOLATION_TYPE_LINEAR);
                    int frame_num = 2;
                    std::vector<float> input_data{0, time};
                    std::vector<float> output_data{start, end};

                    channel->set_input_buf((uint8_t *)input_data.data(), sizeof(float) * frame_num,
                                           sizeof(float), frame_num);
                    channel->set_output_buf((uint8_t *)output_data.data(), sizeof(float) * frame_num,
                                            sizeof(float), frame_num);
                    channel->set_type(sg::ANIMATION_CHANNEL_TYPE_CUSTOM);
                    channel->set_custom_func([func](sg::Node *node, const std::vector<float> &vals)
                                             {
                                                 auto color = node->get_component<sg::Material>()->get_emissive_color();
                                                 color.w = vals[0];
                                                 qDebug() << "name->"<<node->get_name().c_str();
                                                 node->get_component<sg::Material>()->set_emissive_color(color);
                                                 if(func){
                                                     qDebug()<<"func";
                                                    func(vals); 
                                                 } });
                    animation->add_channel(channel);
                    node_t->add_component<sg::AnimationManager>()->add_animation(name, animation);
                    if (is_play)
                        animation->play();
                };
                auto node = EditorWindows::getInstance().get_select_node();
                // 根据按下的键进行相应操作
                if (key == Qt::Key_C) // 更换车漆
                {
                    if (node && node->get_name() == "ziche")
                    {
                        auto node_t = node->find_node("cheqi");
                        std::shared_ptr<zr::sg::Texture> texture = get_texture(mpaths[curr_skin_index]);
                        node_t->get_component<sg::Material>()->update_texture(zr::sg::TextureType::TEXTURE_TYPE_DIFFUSE, texture);

                        curr_skin_index++;
                        if (curr_skin_index > mpaths.size() - 1)
                        {
                            curr_skin_index = 0;
                        }
                        auto mat = node_t->get_component<sg::Material>();
                        mat->load(get_skin_path(curr_skin_index));
                        // mat->set_emissive_color
                    }
                }
                else if (key == Qt::Key_D) // 更换车灯
                {

                    if (node && node->get_name() == "ziche")
                    {
                        auto node_t = node->find_node("cheweihongdeng");
                        std::string animation_name = "cheweihongdeng";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            add_to_animation_emissive_color_intensity(node_t, animation_name, 0, 3, 1);
                        }

                        node_t = node->find_node("yuanguang_deng");
                        animation_name = "yuanguang_deng";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            auto node_j = node->find_node("jinguang_deng");
                            auto animation_name_j = "jinguang_deng";
                            add_to_animation_emissive_color_intensity(node_j, animation_name_j, 0, 20, 1, false);

                            add_to_animation_emissive_color_intensity(node_t, animation_name, 0, 3, 1, true, [node_j, animation_name_j](const std::vector<float> &vals)
                                                                      {

                                                                          if (vals[0] > 1.5)
                                                                          {
                                                                              auto anim = node_j->get_animation(animation_name_j)[0];
                                                                              if (anim->get_state() != sg::Animation::AnimationState::ANIMATION_STATE_PLAYING)
                                                                              {
                                                                                  anim->stop();
                                                                                  anim->play();
                                                                              }
                                                                          } });
                        }

                        node_t = node->find_node("faguanglogo");
                        animation_name = "faguanglogo";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            add_to_animation_emissive_color_intensity(node_t, animation_name, 0, 2, 1);
                        }
                    }
                }
                else if (key == Qt::Key_F) // 更换车灯
                {

                    if (node && node->get_name() == "ziche")
                    {
                        auto node_t = node->find_node("cheweihongdeng");
                        std::string animation_name = "cheweihongdeng2";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            add_to_animation_emissive_color_intensity(node_t, animation_name, 3, 0, 1);
                        }

                        node_t = node->find_node("jinguang_deng");
                        animation_name = "jinguang_deng2";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            auto node_y = node->find_node("yuanguang_deng");
                            auto animation_name_y = "yuanguang_deng2";
                            add_to_animation_emissive_color_intensity(node_y, animation_name_y, 3, 0, 1, false);

                            add_to_animation_emissive_color_intensity(node_t, animation_name, 20, 0, 1, true, [node_y, animation_name_y](const std::vector<float> &vals)
                                                                      {
                                                                        qDebug()<<"yuanguang_deng2";
                                                                          if (vals[0] <10)
                                                                          {
                                                                              auto anim = node_y->get_animation(animation_name_y)[0];
                                                                              if (anim->get_state() != sg::Animation::AnimationState::ANIMATION_STATE_PLAYING)
                                                                              {
                                                                                  anim->stop();
                                                                                  anim->play();
                                                                              }
                                                                          } });
                        }

                        node_t = node->find_node("faguanglogo");
                        animation_name = "faguanglogo2";
                        if (node_t->get_animation(animation_name).size() > 0)
                        {
                            node_t->get_animation(animation_name)[0]->stop();
                            node_t->get_animation(animation_name)[0]->play();
                        }
                        else
                        {
                            add_to_animation_emissive_color_intensity(node_t, animation_name, 2, 0, 1);
                        }
                    }
                }
            }
            return QObject::eventFilter(obj, event);
        }
    };
    void EditorWindows::init()
    {
        // 获取当前工作目录
        // QString currentPath = QDir::currentPath();
        // 构建绝对路径
        //_assert_base_dir = QDir(currentPath).filePath(_assert_base_dir.c_str()).toStdString();
        init_gltf_window();
        init_valukan();
        init_qt_func_window();
        init_scene();
    }

    std::function<void()> EditorWindows::Show()
    {

        // 初始化帧率时器
        double lastTime = glfwGetTime();
        int frameCount = 0;
        std::function<void()> renderloop = [&]()
        {
            // 更新帧率计数器
            double currentTime = glfwGetTime();
            frameCount++;
            static float angle = 0.0;
            // 每秒更新一次帧率信息
            if (currentTime - lastTime >= 1.0)
            {
                // 计算帧率
                double fps = static_cast<double>(frameCount) / (currentTime - lastTime);

                // 在窗口的标题栏上显示帧率信息
                std::string title = "FPS: " + std::to_string(fps);
                glfwSetWindowTitle(glfwWindow, title.c_str());

                // 重置计时帧率计数器
                lastTime = currentTime;
                frameCount = 0;
            }
            glfwPollEvents(); // 处理GLFW事件
            // 在每一帧渲染之前将当前上下文设置为GLFW窗口上下文
            glfwMakeContextCurrent(glfwWindow);
            if (_isroate)
            {
                angle += 0.001;
                auto mat_z = glm::angleAxis(angle, glm::vec3(0.0, 0.0, 1.0));

                auto mat = mat_z;
                auto move = mat * glm::vec4(-1.5f, 0.0, 0.0, 1.0);
                for (auto node : scene->get_nodes())
                {
                    std::shared_ptr<sg::GltfNode> g_node = std::dynamic_pointer_cast<sg::GltfNode>(node);
                    if (g_node && g_node->get_name() == "ziche")
                    {
                        std::shared_ptr<sg::Transform> transform = g_node->get_component<sg::Transform>();
                        transform->set_rotation(mat);
                        transform->set_translation(move);
                    }
                    if (node->get_name() == "shadow")
                    {

                        std::shared_ptr<sg::Transform> transform = node->get_component<sg::Transform>();
                        transform->set_rotation(mat);
                        transform->set_translation(move);
                    }
                }
            }
            renderer.render_scene(scene);
            // 更新GLFW窗口的绘图缓冲区
            glfwSwapBuffers(glfwWindow);
            qtWindow.repaint(); // 强制重绘Qt窗口
        };

        qtWindow.show();
        return renderloop;
    }
    void EditorWindows::bing_treeView()
    {
        static std::map<QStandardItem *, std::shared_ptr<sg::Node>> map;
        map.clear();
        // 初始化modeltree
        QStandardItemModel *m_standardItemModel = new QStandardItemModel();

        // 使用model设置QTreeView表头
        m_standardItemModel->setHorizontalHeaderLabels(QStringList(QStringLiteral("scene")));

        // 创建根节点，抽象Item，并没有实际数据
        QStandardItem *itemRoot = m_standardItemModel->invisibleRootItem();

        std::function<void(std::shared_ptr<sg::Node>, QStandardItem *)> pass_node = [&](std::shared_ptr<sg::Node> node, QStandardItem *prent)
        {
            QStandardItem *item = new QStandardItem(node->get_name().c_str());
            if (!(node->get_component<sg::DirectionalLight>() || node->get_component<sg::PerspectiveCamera>() || node->get_component<sg::EnvironmentLight>())) //(node->get_mesh())
            {
                item->setCheckable(true);
                if (node->is_visible())
                {
                    item->setCheckState(Qt::CheckState::Checked);
                }
                else
                {
                    item->setCheckState(Qt::CheckState::Unchecked);
                }
            }
            map.insert({item, node});
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 禁止编辑文本
            prent->appendRow(item);
            for (auto sub_node : node->children())
            {
                pass_node(sub_node, item);
            }
        };

        pass_node(scene->get_root(), itemRoot);
        // 将数据模型设置给QTreeView
        _treeView->setModel(m_standardItemModel);

        // 响应复选框状态改变事件
        QObject::connect(m_standardItemModel, &QStandardItemModel::itemChanged, [=](QStandardItem *changedItem)
                         {
                        auto node = map[changedItem];
                        if (changedItem->isCheckable())
                        {
                            Qt::CheckState state = changedItem->checkState();
                
                            node->set_is_visible(state);
                        } });
        // 创建选择模型并与模型关联
        QItemSelectionModel *selectionModel = new QItemSelectionModel(m_standardItemModel);
        _treeView->setSelectionModel(selectionModel);
        _treeView->expandAll();
        // 连接selectionChanged信号，并处理选中项改变事件
        QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected)
                         {
        Q_UNUSED(deselected); // 忽略取消选中的项

        // 获取当前选中项的索引
        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        for (const QModelIndex &index : selectedIndexes) {
            QStandardItem *item = m_standardItemModel->itemFromIndex(index);
            auto node = map[item];
            if(node)
            bing_listView(node);
            break;
        } });
    }
    void EditorWindows::init_components()
    {
        QScrollArea *scroll = qtWindow.findChild<QScrollArea *>("scrollArea");
        QWidget *viewport = scroll->findChild<QWidget *>("scrollAreaWidgetContents");
        QVBoxLayout *layout = viewport->findChild<QVBoxLayout *>("verticalLayout");
        layout->setAlignment(Qt::AlignTop);

        while (QLayoutItem *item = layout->takeAt(0))
        {
            layout->removeItem(item);
            item->widget()->setVisible(false);
        }

        if (!_materal_ui)
        {
            _materal_ui = new Ui::Materal(viewport); // 使用指针进行实例化
            _materal_ui->setFixedHeight(270);
        }
        if (!_transform_ui)
        {
            _transform_ui = new Ui::Transform(viewport);
            _transform_ui->setFixedHeight(120);
        }
        if (!_camera_ui)
        {
            _camera_ui = new Ui::Camera(viewport);
            _camera_ui->setFixedHeight(120);
        }
        if (!_directional_light_ui)
        {
            _directional_light_ui = new Ui::Directional_Light(viewport);
            _directional_light_ui->setFixedHeight(100);
        }
        if (!_animation_manager_ui)
        {
            _animation_manager_ui = new Ui::Animation_Manager(viewport);
            _animation_manager_ui->setFixedHeight(155);
        }

        if (_select_node)
        {
            if (get_node_type(_select_node) != NodeType::Root)
            {
                if (_select_node->get_component<sg::Transform>())
                {
                    _transform_ui->binding_node();
                    layout->addWidget(_transform_ui);
                }
                if (_select_node->get_component<sg::PerspectiveCamera>())
                {
                    _camera_ui->binding_node();
                    layout->addWidget(_camera_ui);
                }
                if (_select_node->get_component<sg::DirectionalLight>())
                {
                    _directional_light_ui->binding_node();
                    layout->addWidget(_directional_light_ui);
                }
                if (_select_node->get_component<sg::Material>())
                {
                    _materal_ui->binding_node();
                    layout->addWidget(_materal_ui);
                }
                if (_select_node->get_component<sg::AnimationManager>())
                {
                    _animation_manager_ui->binding_node();
                    layout->addWidget(_animation_manager_ui);
                }
            }
        }
        scroll->setWidget(viewport);
        scroll->setWidgetResizable(true);
        scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置 QScrollArea 的大小策略
        viewport->setLayout(layout);
        viewport->adjustSize();
    }
    NodeType EditorWindows::get_node_type(std::shared_ptr<sg::Node> node)
    {
        if (node->get_component<sg::DirectionalLight>() || node->get_component<sg::PerspectiveCamera>() || node->get_component<sg::EnvironmentLight>())
        {
            return NodeType::ReadOnly;
        }
        if (std::dynamic_pointer_cast<sg::GeometryNode>(node))
        {
            return NodeType::Geometry;
        }
        else if (std::dynamic_pointer_cast<sg::GltfNode>(node))
        {
            return NodeType::Gltf;
        }
        else
        {
            auto pre = node->parent();
            if (pre)
            {
                do
                {
                    if (dynamic_cast<sg::GltfNode *>(pre))
                    {

                        return NodeType::SubGltf;
                    }
                } while (pre = pre->parent());
            }
            else
            {
                return NodeType::Root;
            }

            return NodeType::None;
        }
    }
    void EditorWindows::init_qt_func_window()
    {
        KeyEventFilter *eventFilter = new KeyEventFilter;
        qtWindow.installEventFilter(eventFilter);

        QAction *import_gltf = qtWindow.findChild<QAction *>("import_gltf");
        if (import_gltf)
        {
            _add_gltf_func = [&]()
            {
                    QString arg("gltf files (*.gltf)");
                    QString fileName = QFileDialog::getOpenFileName(
                            &qtWindow, "Open File", _assert_base_dir.c_str(),
                            "gltf files (*.gltf)", &arg);
                    QFileInfo fileInfo(fileName);
                    QString path = fileInfo.absolutePath();
                    QString name = fileInfo.baseName();
                    if(fileInfo.exists()){
                        auto cccc=fileName.toStdString();
                        if( fileName.startsWith(_assert_base_dir.c_str())){
                              
                            QFileInfo matInfo(path+"/"+name+".mat");
                            QString gltf_path=fileName.mid(_assert_base_dir.size(),fileName.size()-_assert_base_dir.size());
                            QString material_file_path="";
                            if(matInfo.exists()){
                                material_file_path=name+".mat";
                            }
                        // QMessageBox::information(&qtWindow, "打开文件", "gltf_path->"+gltf_path+"   material_file_path->"+material_file_path);
#ifdef WIN32
                            auto glft_node = std::make_shared<sg::GltfNode>(gltf_path.toUtf8().constData(), material_file_path.toUtf8().constData(), name.toUtf8().constData());
#else
                            auto glft_node = std::make_shared<sg::GltfNode>(gltf_path.toStdString(), material_file_path.toStdString(), name.toStdString());
#endif // WIN32                          
                     
                            if(_is_editor){
                                _select_node->add_child(glft_node);
                                _is_editor=false;
                            }
                            else{
                                scene->add_node(glft_node);
                            }
                              bing_treeView();
                      
                        }else{
                             QMessageBox::information(&qtWindow, "打开文件", "不能选取不是资源路径的gltf文件,请先将文件拷贝到资源路径下再载入."); 
                        }
                    } };
            QObject::connect(import_gltf, &QAction::triggered, _add_gltf_func);
        }
        QAction *new_scene = qtWindow.findChild<QAction *>("new_scene");
        if (new_scene)
        {
            QObject::connect(new_scene, &QAction::triggered, [&]()
                             { init_scene(); });
        }
        QAction *save_param = qtWindow.findChild<QAction *>("save_param");
        if (save_param)
        {
            QObject::connect(save_param, &QAction::triggered, [&]()
                             {
                                QString save_name;
                                show_input_window("名称","保存场景",save_name);
                                 // 保存场景参数
                                auto add_vec3 = [](glm::vec3 p) -> Json::Value
                                {
                                    Json::Value arr(Json::arrayValue); // 创建一个空的 JSON 对象
                                    arr.append(p.x);
                                    arr.append(p.y);
                                    arr.append(p.z);
                                    return arr;
                                };
                                auto add_vec4 = [](glm::vec4 p) -> Json::Value
                                {
                                    Json::Value arr(Json::arrayValue); // 创建一个空的 JSON 对象
                                    arr.append(p.x);
                                    arr.append(p.y);
                                    arr.append(p.z);
                                    arr.append(p.w);
                                    return arr;
                                };
                      
                                std::function<void(std::shared_ptr<sg::Node> node,Json::Value&  parent)> pass_node=[&]( std::shared_ptr<sg::Node> node,  Json::Value&  parent)
                                {
                                    auto type= get_node_type(node);
                                    Json::Value json_n(Json::objectValue); // 创建一个空的 JSON 对象
                                    json_n["name"]=node->get_name();
                                    if(type==NodeType::Gltf){
                                        auto note_t=std::dynamic_pointer_cast<sg::GltfNode>(node);
                                        note_t->save_external_material();
                                        json_n["type"]="Gltf";
                                        json_n["gltf_path"]=note_t->get_gltf_path();
                                        json_n["material_path"]=note_t->get_material_path();
                                    }else if(type==NodeType::Geometry){
                                        auto note_t=std::dynamic_pointer_cast<sg::GeometryNode>(node);
                                        auto info=_geom_map[note_t];
                                        json_n["type"]="Geometry";
                                        json_n["info"]=info.tojson();
                                    }else if(type==NodeType::None){
                                        json_n["type"]="Node";
                                        Json::Value children(Json::arrayValue); 
                                        for (auto sub_node : node->children())
                                        {
                                            pass_node(sub_node,children);
                                        }
                                        json_n["children"]=children;
                                    }else{
                                        return;
                                    }
                                    json_n["translation"]= add_vec3(node->get_component<sg::Transform>()->get_translation());
                                    auto r=node->get_component<sg::Transform>()->get_rotation();
                                    json_n["rotation"]= add_vec4(glm::vec4(r.x,r.y,r.z,r.w));
                                    json_n["scale"]= add_vec3(node->get_component<sg::Transform>()->get_scale());
                                    parent.append(json_n);
                                 };
                                 Json::Value root(Json::objectValue); // 创建一个空的 JSON 对象

                                 auto camera = scene->get_active_camera();
                                 if (camera)
                                 {
                                     Json::Value camera_j(Json::objectValue); // 创建一个空的 JSON 对象
                                     camera_j["pos"] = add_vec3(camera->get_pos());
                                     camera_j["target"] = add_vec3(camera->get_target());
                                     camera_j["up"] = add_vec3(camera->get_up());
                                     root["camera"] = camera_j;
                                 }
                                 
                                 if (_directional_light)
                                 {
                                     Json::Value light_j(Json::objectValue); // 创建一个空的 JSON 对象
                                     light_j["direction"] = add_vec3(_directional_light->get_light_info().direction);
                                     light_j["color"] = add_vec4(_directional_light->get_light_info().color);
                                     root["light"] = light_j;
                                 }
                                if (_directional_light_2)
                                 {
                                     Json::Value light_j(Json::objectValue); // 创建一个空的 JSON 对象
                                     light_j["direction"] = add_vec3(_directional_light_2->get_light_info().direction);
                                     light_j["color"] = add_vec4(_directional_light_2->get_light_info().color);
                                     root["light2"] = light_j;
                                 }


                                 Json::Value scenes(Json::arrayValue); 
                                 for(auto node :scene->get_root()->children()){
                                    pass_node(node,scenes);
                                 }
                                root["scene"] = scenes;

                                 // 输出生成的 JSON 字符串
                                 Json::StreamWriterBuilder writer;
                                 std::string jsonString = Json::writeString(writer, root);

#ifdef WIN32
                                 auto path = _assert_base_dir;
                                 // 将斜杠替换为反斜杠
                                 for (char& c : path) {
                                     if (c == '/') {
                                         c = '\\';
                                     }
                                 }
                                 std::ofstream outputFile(path + save_name.toUtf8().constData() + ".json"); // 打开输出文件
#else
                                 std::ofstream outputFile(_assert_base_dir + save_name.toStdString() + ".json"); // 打开输出文件
#endif // WIN32

                             
                                 if (outputFile.is_open())
                                 {                             // 检查文件是否成功打开
                                     outputFile << jsonString; // 将字符串写入文件
                                     outputFile.close();       // 关闭文件
                                     return true;
                                 }
                                 else
                                 {
                                     return false;
                                 } });
        }
        QAction *load_param = qtWindow.findChild<QAction *>("load_param");
        if (load_param)
        {
            QObject::connect(load_param, &QAction::triggered, [&]()
                             {
                                 QString arg("json files (*.json)");
                                 QString fileName = QFileDialog::getOpenFileName(
                                     &qtWindow, "Open File", _assert_base_dir.c_str(),
                                     "json files (*.json)", &arg);
                                 QFileInfo fileInfo(fileName);
                                 QString path = fileInfo.absolutePath();
                                 QString name = fileInfo.baseName();
                                 if (fileInfo.exists())
                                 {
                                     if (fileName.startsWith(_assert_base_dir.c_str()))
                                     {

                                         QString json_path = fileName.mid(_assert_base_dir.size(), fileName.size() - _assert_base_dir.size());
#ifdef WIN32
                                         std::string content = utils::FileHelper().load_content(json_path.toUtf8().constData());
#else
                                         std::string content = utils::FileHelper().load_content(json_path.toStdString().c_str());
#endif // WIN32
                                         Json::Value root;
                                         Json::Reader jr;
                                         if (!jr.parse(content, root))
                                         {
                                             return false;
                                         }
                                         auto get_vec3 = [](Json::Value vaule, const char *name, glm::vec3 &out_p) -> bool
                                         {
                                             auto json_params = vaule.get(name, Json::Value::null);
                                             if (json_params != Json::Value::null && json_params.size() == 3)
                                             {
                                                 out_p.x = json_params[0].asFloat();
                                                 out_p.y = json_params[1].asFloat();
                                                 out_p.z = json_params[2].asFloat();
                                                 return true;
                                             }
                                             return false;
                                         };
                                         auto get_vec4 = [](Json::Value vaule, const char *name, glm::vec4 &out_p) -> bool
                                         {
                                             auto json_params = vaule.get(name, Json::Value::null);
                                             if (json_params != Json::Value::null && json_params.size() == 4)
                                             {
                                                 out_p.x = json_params[0].asFloat();
                                                 out_p.y = json_params[1].asFloat();
                                                 out_p.z = json_params[2].asFloat();
                                                 out_p.w = json_params[3].asFloat();
                                                 return true;
                                             }
                                             return false;
                                         };

                                         std::function<std::shared_ptr<sg::Node>(Json::Value & value)> pass_node = [&](Json::Value &value) -> std::shared_ptr<sg::Node>
                                         {
                                            std::shared_ptr<sg::Node> node;
                                            auto name = value["name"].asCString();
                                            auto type = std::string(value["type"].asCString());
                                            if (type == "Gltf")
                                            {
                                                auto material_path = value["material_path"].asCString(); 
                                                auto gltf_path = value["gltf_path"].asCString();
                                                node = std::make_shared<sg::GltfNode>(gltf_path, material_path, name);
                                            }
                                            else if (type == "Geometry")
                                            {
                                                GeometryNodeInfo info;
                                                info.loadjson(value["info"]);
                                                auto t=info.to_node();
                                                _geom_map.insert({t,info});
                                                node=t;
                                            }
                                            else if (type == "Node")
                                            {
                                                node=std::make_shared<sg::Node>(name);
                                                auto children = value.get("children", Json::Value::null);
                                                if (children != Json::Value::null && children.size() > 0){
                                                    for (int i = 0; i < children.size(); i++){
                                                        node->add_child(pass_node(children[i]));
                                                    }
                                                }
                                            }else{

                                                return nullptr;
                                            }
                                            glm::vec3 translation, scale;
                                            glm::vec4 rotation;
                                            if(get_vec3(value,"translation",translation) && get_vec4(value,"rotation",rotation) && get_vec3(value,"scale",scale)){
                                                auto tf= node->get_component<sg::Transform>();
                                                if(tf){
                                                    tf->set_translation(translation);
                                                    tf->set_rotation(glm::quat(rotation.w,rotation.x,rotation.y,rotation.z));  
                                                    tf->set_scale(scale);   
                                                }
                                                                   
                                            }
                                            return node;
                                         };

                                         auto camera = root["camera"];
                                         glm::vec3 pos, target, up;
                                         if (get_vec3(camera, "pos", pos) && get_vec3(camera, "target", target) && get_vec3(camera, "up", up))
                                         {
                                             auto camera_n = scene->get_active_camera();
                                             camera_n->look_at(pos, target, up);
                                         }
                                         auto light = root["light"];
                                         glm::vec3 direction;
                                         glm::vec4 color;
                                         if (get_vec3(light, "direction", direction) && get_vec4(light, "color", color))
                                         {
                                             _directional_light->set_params(direction, color);
                                         }
                                        auto light2 = root.get("light2", Json::Value::null);
                                         if (light2 != Json::Value::null )
                                         {
                                            if (get_vec3(light2, "direction", direction) && get_vec4(light2, "color", color))
                                            {
                                                _directional_light_2->set_params(direction, color);
                                            }
                                         }
                                         auto scenes = root.get("scene", Json::Value::null);
                                         if (scenes != Json::Value::null && scenes.size() > 0)
                                         {
                                            for (int i = 0; i < scenes.size(); i++)
                                            {
                                                auto node = pass_node(scenes[i]);
                                                if(node)
                                                    scene->add_node(node);
                                            }
                                         }

                                         bing_treeView();
                                     }
                                     else
                                     {
                                         //QMessageBox::information(&qtWindow, "打开文件", "不能选取不是资源路径的json文件，请先将文件拷贝到资源路径下再载入。");
                                     }
                                 } });
        }

        _treeView = qtWindow.findChild<QTreeView *>("treeView");
        _listView = qtWindow.findChild<QTreeView *>("propView");
        _applyButton = qtWindow.findChild<QPushButton *>("btnApply");
        _resetButton = qtWindow.findChild<QPushButton *>("btnReset");
        _treeView->installEventFilter(eventFilter);
    }

    void EditorWindows::init_gltf_window()
    {

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        glfwWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

        auto mouse_callback = [](GLFWwindow *window, double xpos, double ypos)
        {
            static double lastX = xpos;
            static double lastY = ypos;
            static bool isfrist = true;
            auto _camera = EditorWindows::getInstance()._camera;
            float sensitivity = 0.001f; // 鼠标灵敏度

            // 检查鼠标左键是否按下
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                // 计算鼠标的移动增量
                float deltaX = xpos - lastX;
                float deltaY = lastY - ypos; // 注意此处的方向，因为窗口的坐标系的原点在左上角
                lastX = xpos;
                lastY = ypos;
                if (isfrist)
                {
                    isfrist = false;
                    return;
                }
                auto pos = _camera->get_pos();
                auto target = _camera->get_target();
                auto up = _camera->get_up();
                glm::vec3 axis_x(glm::normalize(target - pos));
                glm::vec3 axis_y(glm::cross(axis_x, up));
                glm::vec3 axis_z(glm::cross(axis_y, axis_x));
                // 根据鼠标移动增量更新相机的旋转角度
                if (glm::abs(deltaX) > glm::abs(deltaY))
                {
                    auto angle = deltaX * sensitivity;
                    auto rotate_z = glm::angleAxis(angle, axis_z);
                    auto offset_n = rotate_z * axis_x;
                    target = offset_n * glm::length(target - pos) + pos;

                    up = glm::cross(axis_y, glm::normalize(target - pos));
                }
                else
                {
                    auto angle = deltaY * sensitivity;
                    auto rotate_y = glm::angleAxis(-angle, axis_y);
                    auto offset_n = rotate_y * axis_x;
                    target = offset_n * glm::length(target - pos) + pos;
                }
                _camera->look_at(pos, target, up);
                EditorWindows::getInstance().bing_treeView();
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {

                // 计算鼠标的移动增量
                float deltaX = xpos - lastX;
                float deltaY = lastY - ypos; // 注意此处的方向，因为窗口的坐标系的原点在左上角
                lastX = xpos;
                lastY = ypos;
                if (isfrist)
                {
                    isfrist = false;
                    return;
                }
                auto pos = _camera->get_pos();
                auto target = _camera->get_target();
                auto up = _camera->get_up();
                glm::vec3 axis_x(glm::normalize(target - pos));
                glm::vec3 axis_y(glm::cross(axis_x, up));
                glm::vec3 axis_z(glm::cross(axis_y, axis_x));
                float len = glm::length(target - pos);
                if (glm::abs(deltaX) > glm::abs(deltaY))
                {
                    auto angle = deltaX * sensitivity;
                    auto rotate_z = glm::angleAxis(-angle, axis_z);
                    auto offset_n = rotate_z * axis_x;
                    pos = -offset_n * len + target;

                    up = glm::cross(axis_y, glm::normalize(target - pos));
                }
                else
                {
                    auto angle = deltaY * sensitivity;
                    auto rotate_y = glm::angleAxis(angle, axis_y);
                    auto offset_n = rotate_y * axis_x;
                    pos = -offset_n * len + target;
                }
                _camera->look_at(pos, target, up);
                EditorWindows::getInstance().bing_treeView();
            }
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {

                // 计算鼠标的移动增量
                float deltaX = xpos - lastX;
                float deltaY = lastY - ypos; // 注意此处的方向，因为窗口的坐标系的原点在左上角
                sensitivity = 0.01f;         // 鼠标灵敏度
                lastX = xpos;
                lastY = ypos;
                if (isfrist)
                {
                    isfrist = false;
                    return;
                }
                auto pos = _camera->get_pos();
                auto target = _camera->get_target();
                auto up = _camera->get_up();
                glm::vec3 axis_x(glm::normalize(target - pos));
                glm::vec3 axis_y(glm::cross(axis_x, up));
                pos -= axis_y * deltaX * sensitivity + up * deltaY * sensitivity;
                target -= axis_y * deltaX * sensitivity + up * deltaY * sensitivity;

                _camera->look_at(pos, target, up);
                EditorWindows::getInstance().bing_treeView();
            }
            else
            {
                isfrist = true;
            }
        };

        auto scroll_callback = [](GLFWwindow *window, double xoffset, double yoffset)
        {
            auto _camera = EditorWindows::getInstance()._camera;
            auto pos = _camera->get_pos();
            auto target = _camera->get_target();
            auto up = _camera->get_up();
            pos += glm::vec3(-yoffset * 0.1) * glm::normalize(pos - target);
            _camera->look_at(pos, target, up);
            EditorWindows::getInstance().bing_treeView();
        };

        auto key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            auto _camera = EditorWindows::getInstance()._camera;
            auto pos = _camera->get_pos();
            auto target = _camera->get_target();
            auto up = _camera->get_up();
            glm::vec3 axis_x(glm::normalize(target - pos));
            glm::vec3 axis_y(glm::cross(axis_x, up));
            glm::vec3 axis_z(glm::cross(axis_y, axis_x));
            glm::vec3 move(0.0);
            const float cameraSpeed = 0.1f; // 相机的移动速度
            // 根据按键调整相机位置
            if (action == GLFW_PRESS)
            {
                switch (key)
                {
                case GLFW_KEY_W:
                    move += axis_x * cameraSpeed;
                    break;
                case GLFW_KEY_S:
                    move -= axis_x * cameraSpeed;
                    break;
                case GLFW_KEY_A:
                    move += axis_y * cameraSpeed;
                    break;
                case GLFW_KEY_D:
                    move -= axis_y * cameraSpeed;
                    break;
                case GLFW_KEY_Q:
                    move += axis_z * cameraSpeed;
                case GLFW_KEY_E:
                    move -= axis_z * cameraSpeed;
                    break;
                case GLFW_KEY_M:
                    up = up * glm::vec3(-1.0f);
                case GLFW_KEY_R:
                    EditorWindows::getInstance()._isroate = !EditorWindows::getInstance()._isroate;
                    break;
                default:
                    break;
                }
            }

            _camera->look_at(pos + move, target + move, up);
            EditorWindows::getInstance().bing_treeView();
        };

        // 设置回调函数
        glfwSetCursorPosCallback(glfwWindow, mouse_callback);
        glfwSetKeyCallback(glfwWindow, key_callback);
        // 注册滚轮滚动的回调函数
        glfwSetScrollCallback(glfwWindow, scroll_callback);
    }

    void EditorWindows::update_parameter(QStandardItem *item, const std::shared_ptr<sg::Node> node, std::vector<std::function<void()>> &update_datas)
    {
        auto findChildRow_xyz = [](QStandardItem *parentItem) -> glm::vec3
        {
            float x = parentItem->child(0, 1)->text().toFloat();
            float y = parentItem->child(1, 1)->text().toFloat();
            float z = parentItem->child(2, 1)->text().toFloat();
            return glm::vec3(x, y, z);
        };
        auto findChildRow_xyzw = [](QStandardItem *parentItem) -> glm::vec4
        {
            float x = parentItem->child(0, 1)->text().toFloat();
            float y = parentItem->child(1, 1)->text().toFloat();
            float z = parentItem->child(2, 1)->text().toFloat();
            float w = parentItem->child(3, 1)->text().toFloat();
            return glm::vec4(x, y, z, w);
        };
        auto parent = item->parent();
        if (parent)
        {
            auto parent_t = parent->text();
            auto ccc = parent_t.toStdString();
            if (parent_t.compare("Material") == 0)
            {
                // 修改pbr
                std::function<void()> func = [=]
                {
                    auto mat = node->get_component<sg::Material>();
                    sg::Material::PbrParams params;
                    params.metallic = parent->child(2, 1)->text().toFloat();
                    params.roughness = parent->child(3, 1)->text().toFloat();
                    params.ior = parent->child(4, 1)->text().toFloat();
                    params.clear_coat = parent->child(5, 1)->text().toFloat();
                    params.clear_coat_roughness = parent->child(6, 1)->text().toFloat();
                    mat->set_pbr_params(params);
                };
                update_datas.emplace_back(func);
            }
            else if (parent_t.compare("Direction") == 0 || parent_t.compare("Color") == 0)
            {
                // 修改平行光
                std::function<void()> func = [=]
                {
                    auto d_light = node->get_component<sg::DirectionalLight>();
                    glm::vec3 dir = d_light->get_light_info().direction;
                    glm::vec4 color = d_light->get_light_info().color;
                    if (parent_t.compare("Direction") == 0)
                    {
                        dir = findChildRow_xyz(parent);
                    }
                    else
                    {
                        color = findChildRow_xyzw(parent);
                    }
                    d_light->set_params(dir, color);
                };
                update_datas.emplace_back(func);
            }
            else if (parent_t.compare("Transform") == 0)
            {
                std::function<void()> func = [=]
                {
                    auto tf = node->get_component<sg::Transform>();
                    glm::vec3 translation = findChildRow_xyz(parent);
                    tf->set_translation(translation);
                };
                update_datas.emplace_back(func);
            }
            else if (parent_t.compare("camera_pos") == 0 || parent_t.compare("camera_target") == 0)
            {
                // 修改相机
                std::function<void()> func = [=]
                {
                    auto camera = node->get_component<sg::PerspectiveCamera>();
                    glm::vec3 pos = camera->get_pos();
                    glm::vec3 target = camera->get_target();
                    glm::vec3 up;
                    if (parent_t.compare("camera_pos") == 0)
                    {
                        pos = findChildRow_xyz(parent);
                    }
                    else
                    {
                        target = findChildRow_xyz(parent);
                    }
                    auto dir_x = glm::normalize(glm::vec3(pos - target));
                    auto dir_z = glm::vec3(0.0, 0.0, 1.0);
                    if (dir_x.z > 0.9)
                    {
                        dir_z = glm::vec3(-1.0, 0.0, 0.0);
                    }
                    auto dir_y = glm::cross(dir_z, dir_x);
                    up = glm::cross(dir_x, dir_y);
                    camera->look_at(pos, target, up);
                };
                update_datas.emplace_back(func);
            }
            else if (parent_t.compare("Emissive") == 0)
            {
                auto mat = node->get_component<sg::Material>();
                std::function<void()> func = [=]
                {
                    auto mat = node->get_component<sg::Material>();
                    mat->set_emissive_color(findChildRow_xyzw(parent));
                };
                update_datas.emplace_back(func);
            }
        }
    }

    void EditorWindows::bing_listView(const std::shared_ptr<sg::Node> node)
    {
        if (!node)
            return;
        _select_node = node;
        qtWindow.findChild<QLabel *>("lb_node_name")->setText(_select_node->get_name().c_str());
        init_components();
    }

    void EditorWindows::create_env_light()
    {
        auto env_light_node = std::make_shared<sg::Node>("env_light");
        auto env_light = env_light_node->add_component<sg::EnvironmentLight>();
        std::vector<float> sh_params = {
            5.007108, 5.526969, 6.651827,
            -0.459191, -0.528810, -0.596514,
            0.610822, 0.646088, 0.579100,
            -0.342131, -0.355901, -0.313266,
            -0.001960, -0.002796, -0.009978,
            -0.016324, -0.013947, 0.005466,
            0.020849, 0.031139, 0.039364,
            -0.086265, -0.082241, -0.071213,
            0.073472, 0.074529, 0.06595};
        std::vector<std::string> pre_filtered_img_paths = {
            "images/environment/prefiltered_1.png",
            "images/environment/prefiltered_2.png",
            "images/environment/prefiltered_4.png",
            "images/environment/prefiltered_8.png",
            "images/environment/prefiltered_16.png",
        };
        std::string dfg_img_path = "images/environment/dfg.png";
        env_light->set_params(sh_params, pre_filtered_img_paths, dfg_img_path);
        scene->add_node(env_light_node);

        // 对齐环境光贴图，修改默认坐标系
        glm::mat4 tmp_mat2 = glm::mat4(0);
        tmp_mat2[0][2] = -1;
        tmp_mat2[1][0] = -1;
        tmp_mat2[2][1] = 1;
        tmp_mat2[3][3] = 1;
        scene->get_root()->get_component<zr::sg::Transform>()->set_matrix(tmp_mat2);
    }

    void EditorWindows::draw_background()
    {

        // auto back = std::make_shared<sg::GltfNode>("");

        // for (auto child: back->children()) {
        //     if(child->get_component<zr::sg::Material>()){
        //         child->get_component<zr::sg::Material>()->set_light_enabled(false);
        //     }
        // }
        // auto asix_x=glm::normalize(_camera_pos-_camera_target);
        // glm::mat4 mat(1);
        // glm::vec3 asix_y=glm::cross(asix_x,-_camera_up);
        // glm::vec3 asix_z=glm::cross(asix_x,asix_y);
        // mat[0]=glm::vec4 (asix_x,1.0);
        // mat[1]=glm::vec4 (-asix_y,1.0);
        // mat[2]=glm::vec4 (asix_z,1.0);

        // mat[3]=glm::vec4 (asix_x*glm::vec3 (-43.0)+_camera_pos-glm::vec3 (0.0,0.0,0.70),1.0);
        // back->get_component<sg::Transform>()->set_matrix(mat);
        // scene->add_node(back);
    }

    void EditorWindows::show_input_window(QString name, QString title, QString &input)
    {
        // 创建对话框
        QDialog dialog(&qtWindow);
        QVBoxLayout dialogLayout(&dialog);
        // 创建水平布局
        QHBoxLayout inputLayout;
        dialogLayout.addLayout(&inputLayout);

        // 创建文本标签和输入框，并将它们添加到水平布局中
        QLabel name_l(name);
        inputLayout.addWidget(&name_l);
        QLineEdit lineEdit;
        inputLayout.addWidget(&lineEdit);

        // 创建水平布局用于按钮
        QHBoxLayout buttonLayout;
        dialogLayout.addLayout(&buttonLayout);

        // 创建确定和取消按钮，并将它们添加到按钮布局中
        QPushButton rejectButton("取消");
        QPushButton acceptButton("确定");
        buttonLayout.addWidget(&rejectButton);
        buttonLayout.addWidget(&acceptButton);
        rejectButton.setAutoDefault(false);
        acceptButton.setAutoDefault(true); // 设置按钮为默认按钮，以便在对话框中响应回车键

        // 连接按钮的点击事件到槽函数
        QObject::connect(&acceptButton, &QPushButton::clicked, [&]()
                         {
                             dialog.accept(); // 用户点击 "确定" 按钮，调用 accept() 函数
                         });

        QObject::connect(&rejectButton, &QPushButton::clicked, [&]()
                         {
                             dialog.reject(); // 用户点击 "取消" 按钮，调用 reject() 函数
                         });

        // 设置对话框的父窗口和标题
        dialog.setParent(&qtWindow);
        dialog.setWindowTitle(title);
        dialog.setModal(true); // 设置对话框为模态
        // 显示对话框并进入事件循环
        if (dialog.exec() == QDialog::Accepted)
        {
            input = lineEdit.text();
        }
    }

    void EditorWindows::init_valukan()
    {
        zr::utils::FileHelper::assert_base_dir = _assert_base_dir;
        renderer.init(glfwWindow);
        renderer.set_viewport(0, 0, WIDTH, HEIGHT);
    }
    void EditorWindows::init_scene()
    {

        if (scene)
        {
            scene.reset();
            scene = nullptr;
        }
        scene = std::make_shared<sg::Scene>();
        // add light
        auto d_light_node = std::make_shared<sg::Node>("d_light");
        _directional_light = d_light_node->add_component<sg::DirectionalLight>();
        _directional_light->set_params(glm::vec3(1.0, 1.0, 1.0), glm::vec4(1.0, 1.0, 1.0, 5.0));
        scene->add_node(d_light_node);

        std::shared_ptr<sg::Node> directionl_node_2 = std::make_shared<sg::Node>(
            "d_light_2");
        _directional_light_2 = directionl_node_2->add_component<sg::DirectionalLight>();
        _directional_light_2->set_params(glm::vec3(0.0, -1.0, 0.5), glm::vec4(0.8, 0.8, 1.0, 1.1));
        scene->add_node(directionl_node_2);
        //_scene_

        create_env_light();
        auto camera_node = scene->add_node("camera");
        std::shared_ptr<sg::PerspectiveCamera> camera = camera_node->add_component<sg::PerspectiveCamera>();
        camera->perspective(glm::radians(40.0f), 0.6125f, 0.1f, 500.0f);
        scene->set_active_camera(camera);
        // create_complex_car_node();
        _camera = scene->get_active_camera();
        _camera->look_at(glm::vec3(0, 0, 5.0f),
                         glm::vec3(0, 0, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f));
        bing_treeView();

        // std::string tex_path= "images/line/road_boundary_2.png";;
        // _line_node = std::make_shared<zr::sg::GeometryNode>("line_node", std::make_shared<zr::sg::SingleLayerTexture>(tex_path), 100);
        // _line_node->translate(0, 0, 0.02f);
        // _line_node->set_is_transparent(true);
        // _line_node->set_is_visible(true);
        // _line_node->add_line_path(glm::mat4(1.0),{0.0,0.0,20.0,0},1.0,glm::vec2(0.0),glm::vec2(54.0/162.0-0.01,1.0),true,4);
        // scene->add_node(_line_node);
    }


}
