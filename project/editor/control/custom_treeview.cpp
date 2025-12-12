#include "custom_treeview.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QColorDialog>

void NodeTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {

        auto window_editor = &editor::EditorWindows::getInstance();
        auto node = window_editor->get_select_node();
        if (!node)
        {
            return;
        }
        auto node_type = window_editor->get_node_type(node);
        // 创建右键菜单
        QMenu menu(this);

        // 创建菜单项
        QAction action1("添加节点", this);
        QAction action2("添加gltf节点", this);
        QAction action3("添加几何节点", this);
        QAction action4("删除节点", this);
        QAction action5("添加Rect", this);
        QAction action6("添加LinePath", this);
        QAction action7("清空Mash", this);
        QAction action8("更换车漆颜色", this);
        QAction action9("保存材质参数", this);
        QAction action10("替换材质参数", this);
        QAction action11("移动到", this);
        switch (node_type)
        {
        case editor::NodeType::None:
            menu.addAction(&action1);
            menu.addAction(&action2);
            menu.addAction(&action3);
            menu.addAction(&action4);
            menu.addAction(&action11);
            break;
        case editor::NodeType::Geometry:
            menu.addAction(&action5);
            menu.addAction(&action6);
            menu.addAction(&action7);
            menu.addAction(&action4);
            menu.addAction(&action11);
            break;
        case editor::NodeType::Gltf:
            menu.addAction(&action4);
            if (node->find_node("cheqi"))
                menu.addAction(&action8);
            menu.addAction(&action11);
            break;
        case editor::NodeType::SubGltf:
            break;
        case editor::NodeType::Root:
            menu.addAction(&action1);
            menu.addAction(&action2);
            menu.addAction(&action3);
            break;

        default:
            break;
        }
        if (node->get_component<sg::Material>() && node->get_name() == "cheqi")
        {
            menu.addAction(&action9);
            // menu.addAction(&action10);
        }
        // 获取鼠标点击的位置
        QPoint globalPos = event->globalPos();

        // 在指定位置显示菜单
        QAction *selectedAction = menu.exec(globalPos);
        static int curr_skin_index = 0;
        auto get_skin_path = [](int index) -> std::string
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
        };
        // 根据选择的菜单项执行操作
        if (selectedAction == &action1)
        {
            QString name = "name";
            QString title = "请输入...";
            QString input;
            show_input_window(name, title, input);
            if (input.size() > 0)
            {
                std::shared_ptr<sg::Node> node_t = std::make_shared<sg::Node>(input.toStdString());
                node->add_child(node_t);
                window_editor->bing_treeView();
            }
        }
        else if (selectedAction == &action2)
        {
            auto add_gltf_func = window_editor->get_add_gltf_func();
            window_editor->_is_editor = true;
            add_gltf_func();
        }
        else if (selectedAction == &action3)
        {
            editor::GeometryNodeInfo info;
            show_create_geometry_node_window(info);
            if (info.node_name.empty())
            {
                return;
            }
            std::shared_ptr<zr::sg::Texture> texture = std::make_shared<zr::sg::SingleLayerTexture>(info.textrue_path);
            std::shared_ptr<sg::GeometryNode> node_t = std::make_shared<sg::GeometryNode>(info.node_name, texture);
            node->add_child(node_t);
            window_editor->add_geometry_node(node_t, info);
            window_editor->bing_treeView();
        }
        else if (selectedAction == &action4)
        {
            int result = QMessageBox::question(this, "询问", "确定要执行删除操作吗", QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
            {
                if (node->parent())
                {
                    node->parent()->remove_child(node);
                    window_editor->bing_treeView();
                    window_editor->reset_elect_node();
                }
            }
        }
        else if (selectedAction == &action5)
        {
            auto &info = window_editor->get_geometry_node_info();
            auto node_t = std::dynamic_pointer_cast<sg::GeometryNode>(node);
            show_add_rect_window(info, node_t);
            node_t->set_is_visible(true);
        }
        else if (selectedAction == &action6)
        {
            auto &info = window_editor->get_geometry_node_info();
            auto node_t = std::dynamic_pointer_cast<sg::GeometryNode>(node);
            show_add_line_path_window(info, node_t);
            node_t->set_is_visible(true);
        }
        else if (selectedAction == &action7)
        {
            auto &info = window_editor->get_geometry_node_info();
            auto node_t = std::dynamic_pointer_cast<sg::GeometryNode>(node);
            node_t->reset();
            // node_t->set_is_visible(false);
            info.rects.clear();
            info.linepaths.clear();
        }
        else if (selectedAction == &action8)
        {

            auto node_cq = node->find_node("cheqi");

            std::vector<std::string> mpaths{
                "images/car_paint/black.png",
                "images/car_paint/electric_blue.png",
                "images/car_paint/gray.png",
                "images/car_paint/extreme_blue.png",
                "images/car_paint/orange.png",
                "images/car_paint/white.png"};

            std::shared_ptr<zr::sg::Texture> texture = std::make_shared<zr::sg::SingleLayerTexture>(mpaths[curr_skin_index]);
            node_cq->get_component<sg::Material>()->update_texture(zr::sg::TextureType::TEXTURE_TYPE_DIFFUSE, texture);

            curr_skin_index++;
            if (curr_skin_index > mpaths.size() - 1)
            {
                curr_skin_index = 0;
            }
            auto mat = node_cq->get_component<sg::Material>();
            mat->load(get_skin_path(curr_skin_index));
        }
        else if (selectedAction == &action9)
        {
            node->get_component<sg::Material>()->save(get_skin_path(curr_skin_index));
        }
        else if (selectedAction == &action10)
        {
            node->get_component<sg::Material>()->load(get_skin_path(curr_skin_index));
        }
        else if (selectedAction == &action11)
        {
            move_node(node);
        }
    }
    else
    {
        QTreeView::mousePressEvent(event);
    }
}

void NodeTreeView::show_input_window(QString name, QString title, QString &input)
{
    // 创建对话框
    QDialog dialog(this->window());
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
    dialog.setParent(this->window());
    dialog.setWindowTitle(title);
    dialog.setModal(true); // 设置对话框为模态
    // 显示对话框并进入事件循环
    if (dialog.exec() == QDialog::Accepted)
    {
        input = lineEdit.text();
    }
}

void NodeTreeView::show_create_geometry_node_window(editor::GeometryNodeInfo &info)
{

    // 创建对话框
    QDialog dialog(this->window());
    QVBoxLayout dialogLayout(&dialog);
    // 创建水平布局
    QHBoxLayout inputLayout;
    dialogLayout.addLayout(&inputLayout);

    // 创建文本标签和输入框，并将它们添加到水平布局中
    QLabel name_l("名称");
    inputLayout.addWidget(&name_l);
    QLineEdit lineEdit;
    inputLayout.addWidget(&lineEdit);

    // 创建水平布局用于按钮
    QHBoxLayout textureLayout;
    dialogLayout.addLayout(&textureLayout);

    QLabel name_t("纹理");
    textureLayout.addWidget(&name_t);
    QLineEdit lineEdit_t;
    lineEdit_t.setReadOnly(true);
    textureLayout.addWidget(&lineEdit_t);
    QPushButton textrueButton("选择");
    textureLayout.addWidget(&textrueButton);

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
                         if (lineEdit.text().trimmed().isEmpty())
                         {
                             QMessageBox::information(this->window(), "创建GeometryNode", ("创建GeometryNode失败，节点名称不能为空！！！"));
                             lineEdit.setFocus();
                             return;
                         }
                         if (lineEdit_t.text().trimmed().isEmpty())
                         {
                             QMessageBox::information(this->window(), "创建GeometryNode", ("创建GeometryNode失败，纹理不能为空！！！"));
                             textrueButton.setFocus();
                             return;
                         }
                         dialog.accept(); // 用户点击 "确定" 按钮，调用 accept() 函数
                     });

    QObject::connect(&rejectButton, &QPushButton::clicked, [&]()
                     {
                         dialog.reject(); // 用户点击 "取消" 按钮，调用 reject() 函数
                     });

    QObject::connect(&textrueButton, &QPushButton::clicked, [&]()
                     {
                        auto bass_dir=editor::EditorWindows::getInstance().get_assert_base_dir();
                        QString arg("img files (*.png)");
                        QString fileName = QFileDialog::getOpenFileName(
                                this->window(), "Open File",bass_dir.c_str() ,
                                "img files (*.png)", &arg);
                        QFileInfo fileInfo(fileName);
                        if(fileInfo.exists()){
                            if( fileName.startsWith(bass_dir.c_str())){
                                QString img_path=fileName.mid(bass_dir.size(),fileName.size()-bass_dir.size());
                                lineEdit_t.setText(img_path);
                            }else{
                                QMessageBox::information( this->window(), "打开文件", "不能选取不是资源路径的img文件 请先将文件拷贝到资源路径下再载入"); 
                            }
                        } });

    // 设置对话框的父窗口和标题
    dialog.setParent(this->window());
    dialog.setWindowTitle("创建GeometryNode");
    dialog.setModal(true); // 设置对话框为模态
    // 显示对话框并进入事件循环
    if (dialog.exec() == QDialog::Accepted)
    {
        info.node_name = lineEdit.text().toStdString();
        info.textrue_path = lineEdit_t.text().toStdString();
    }
}

void NodeTreeView::show_add_rect_window(editor::GeometryNodeInfo &info, std::shared_ptr<sg::GeometryNode> node)
{

    // 创建对话框
    QDialog dialog(this->window());
    QVBoxLayout dialogLayout(&dialog);
    QString win_title = "添加Rect";

    auto add_h_layout_item = [&](QVBoxLayout &dialog_l, QHBoxLayout &inputLayout, QLabel &label, QString title, ValueType type, std::function<bool()> &check, QLineEdit &line_edit)
    {
        dialog_l.addLayout(&inputLayout);
        inputLayout.addWidget(&label);
        if (type == ValueType::Float)
        {
            line_edit.setValidator(new QDoubleValidator(this));
        }
        line_edit.setReadOnly(false);
        inputLayout.addWidget(&line_edit);
        auto win = this->window();
        check = [&line_edit, title, type, win_title, win]() -> bool
        {
            if (line_edit.text().trimmed().isEmpty())
            {
                QMessageBox::information(win, win_title, (win_title + "失败" + title + "不能为空"));
                line_edit.setFocus();
                return false;
            }
            else if (type == ValueType::FloatSet)
            {
                QRegExp regex("^[-+]?[0-9]*\\.?[0-9]+([,\\s]+[-+]?[0-9]*\\.?[0-9]+)*$");
                if (!regex.exactMatch(line_edit.text().trimmed()))
                {
                    QMessageBox::information(win, win_title, (win_title + "失败" + title + "数据有误"));
                    line_edit.setFocus();
                    return false;
                }
            }
            return true;
        };
    };

    std::function<bool()> check_width;
    QLineEdit le_width;
    QHBoxLayout layout_w;
    QLabel label_w("width");
    add_h_layout_item(dialogLayout, layout_w, label_w, "width", ValueType::Float, check_width, le_width);

    std::function<bool()> check_height;
    QLineEdit le_height;
    QHBoxLayout layout_h;
    QLabel label_h("height");
    add_h_layout_item(dialogLayout, layout_h, label_h, "height", ValueType::Float, check_height, le_height);

    std::function<bool()> check_uv_offset_x;
    QLineEdit le_uv_offset_x;
    QHBoxLayout layout_uv_s_x;
    QLabel label_uv_s_x("uv_offset_x");
    add_h_layout_item(dialogLayout, layout_uv_s_x, label_uv_s_x, "uv_offset_x", ValueType::Float, check_uv_offset_x, le_uv_offset_x);

    std::function<bool()> check_uv_offset_y;
    QLineEdit le_uv_offset_y;
    QHBoxLayout layout_uv_s_y;
    QLabel label_uv_s_y("uv_offset_y");
    add_h_layout_item(dialogLayout, layout_uv_s_y, label_uv_s_y, "uv_offset_y", ValueType::Float, check_uv_offset_y, le_uv_offset_y);

    std::function<bool()> check_uv_range_x;
    QLineEdit le_uv_range_x;
    QHBoxLayout layout_uv_r_x;
    QLabel label_uv_r_x("uv_range_x");
    add_h_layout_item(dialogLayout, layout_uv_r_x, label_uv_r_x, "uv_range_x", ValueType::Float, check_uv_range_x, le_uv_range_x);

    std::function<bool()> check_uv_range_y;
    QLineEdit le_uv_range_y;
    QHBoxLayout layout_uv_r_y;
    QLabel label_uv_r_y("uv_range_y");
    add_h_layout_item(dialogLayout, layout_uv_r_y, label_uv_r_y, "uv_range_y", ValueType::Float, check_uv_range_y, le_uv_range_y);

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
                         if (check_width() && check_height() && check_uv_offset_x() && check_uv_offset_y() && check_uv_range_x() && check_uv_range_y())
                             dialog.accept(); // 用户点击 "确定" 按钮，调用 accept() 函数
                     });

    QObject::connect(&rejectButton, &QPushButton::clicked, [&]()
                     {
                         dialog.reject(); // 用户点击 "取消" 按钮，调用 reject() 函数
                     });

    // 设置对话框的父窗口和标题
    dialog.setParent(this->window());
    dialog.setWindowTitle(win_title);
    dialog.setModal(true); // 设置对话框为模态
    // 显示对话框并进入事件循环
    if (dialog.exec() == QDialog::Accepted)
    {
        auto w = le_width.text().toFloat();
        auto h = le_height.text().toFloat();
        auto off_x = le_uv_offset_x.text().toFloat();
        auto off_y = le_uv_offset_y.text().toFloat();
        auto range_x = le_uv_range_x.text().toFloat();
        auto range_y = le_uv_range_y.text().toFloat();
        editor::Geometry_Rect rect;
        rect.height = h;
        rect.width = w;
        rect.transform = glm::mat4(1.0);
        rect.uv_offset = glm::vec2(off_x, off_y);
        rect.uv_range = glm::vec2(range_x, range_y);
        info.add_node_rect(node, rect);
        info.rects.emplace_back(rect);
    }
}

void NodeTreeView::show_add_line_path_window(editor::GeometryNodeInfo &info, std::shared_ptr<sg::GeometryNode> node)
{

    // 创建对话框
    QDialog dialog(this->window());
    QVBoxLayout dialogLayout(&dialog);
    QString win_title = "添加LinePath";

    auto add_h_layout_item = [&](QVBoxLayout &dialog_l, QHBoxLayout &inputLayout, QLabel &label, QString title, ValueType type, std::function<bool()> &check, QLineEdit &line_edit)
    {
        dialog_l.addLayout(&inputLayout);
        inputLayout.addWidget(&label);
        if (type == ValueType::Float)
        {
            line_edit.setValidator(new QDoubleValidator(this));
        }
        inputLayout.addWidget(&line_edit);
        auto win = this->window();
        check = [&line_edit, title, type, win_title, win]() -> bool
        {
            if (line_edit.text().trimmed().isEmpty())
            {
                QMessageBox::information(win, win_title, (win_title + "失败" + title + "不能为空"));
                line_edit.setFocus();
                return false;
            }
            else if (type == ValueType::FloatSet)
            {
                QRegExp regex("^[-+]?[0-9]*\\.?[0-9]+([,\\s]+[-+]?[0-9]*\\.?[0-9]+)*$");
                if (!regex.exactMatch(line_edit.text().trimmed()))
                {
                    QMessageBox::information(win, win_title, (win_title + "失败" + title + "数据有误"));
                    line_edit.setFocus();
                    return false;
                }
            }
            return true;
        };
    };

    std::function<bool()> check_width;
    QLineEdit le_width;
    QHBoxLayout layout_w;
    QLabel label_w("width");
    add_h_layout_item(dialogLayout, layout_w, label_w, "width", ValueType::Float, check_width, le_width);

    std::function<bool()> check_path;
    QLineEdit le_path;
    QHBoxLayout layout_p;
    QLabel label_p("path");
    add_h_layout_item(dialogLayout, layout_p, label_p, "path", ValueType::FloatSet, check_path, le_path);

    std::function<bool()> check_uv_offset_x;
    QLineEdit le_uv_offset_x;
    QHBoxLayout layout_uv_s_x;
    QLabel label_uv_s_x("uv_offset_x");
    add_h_layout_item(dialogLayout, layout_uv_s_x, label_uv_s_x, "uv_offset_x", ValueType::Float, check_uv_offset_x, le_uv_offset_x);

    std::function<bool()> check_uv_offset_y;
    QLineEdit le_uv_offset_y;
    QHBoxLayout layout_uv_s_y;
    QLabel label_uv_s_y("uv_offset_y");
    add_h_layout_item(dialogLayout, layout_uv_s_y, label_uv_s_y, "uv_offset_y", ValueType::Float, check_uv_offset_y, le_uv_offset_y);

    std::function<bool()> check_uv_range_x;
    QLineEdit le_uv_range_x;
    QHBoxLayout layout_uv_r_x;
    QLabel label_uv_r_x("uv_range_x");
    add_h_layout_item(dialogLayout, layout_uv_r_x, label_uv_r_x, "uv_range_x", ValueType::Float, check_uv_range_x, le_uv_range_x);

    std::function<bool()> check_uv_range_y;
    QLineEdit le_uv_range_y;
    QHBoxLayout layout_uv_r_y;
    QLabel label_uv_r_y("uv_range_y");
    add_h_layout_item(dialogLayout, layout_uv_r_y, label_uv_r_y, "uv_range_y", ValueType::Float, check_uv_range_y, le_uv_range_y);

    std::function<bool()> check_tex_len;
    QLineEdit le_tex_len;
    QHBoxLayout layout_tex_len;
    QLabel label_tex_len("tex_len");
    le_tex_len.setEnabled(false);

    QHBoxLayout layout_use_tex;
    QCheckBox check_use_tex("use_tex");
    dialogLayout.addLayout(&layout_use_tex);
    layout_use_tex.addWidget(&check_use_tex);
    QObject::connect(&check_use_tex, &QCheckBox::stateChanged, [&](int state)
                     {
                        if (state == Qt::Checked)
                            {
                               le_tex_len.setEnabled(true);
                            }
                            else if (state == Qt::Unchecked)
                            {
                               le_tex_len.setEnabled(false);
                            } });

    add_h_layout_item(dialogLayout, layout_tex_len, label_tex_len, "tex_len", ValueType::Float, check_tex_len, le_tex_len);

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
                         if (check_width() && check_path() && check_uv_offset_x() && check_uv_offset_y() && check_uv_range_x() && check_uv_range_y())
                         {
                             if (check_use_tex.checkState() == Qt::Checked && !check_tex_len())
                             {
                                 return;
                             }
                             dialog.accept(); // 用户点击 "确定" 按钮，调用 accept() 函数
                         } });

    QObject::connect(&rejectButton, &QPushButton::clicked, [&]()
                     {
                         dialog.reject(); // 用户点击 "取消" 按钮，调用 reject() 函数
                     });

    // 设置对话框的父窗口和标题
    dialog.setParent(this->window());
    dialog.setWindowTitle(win_title);
    dialog.setModal(true); // 设置对话框为模态
    // 显示对话框并进入事件循环
    if (dialog.exec() == QDialog::Accepted)
    {
        auto w = le_width.text().toFloat();
        auto path = le_path.text();
        auto off_x = le_uv_offset_x.text().toFloat();
        auto off_y = le_uv_offset_y.text().toFloat();
        auto range_x = le_uv_range_x.text().toFloat();
        auto range_y = le_uv_range_y.text().toFloat();
        auto len = le_tex_len.text().toFloat();
        editor::Geometry_LinePath linepath;

        QStringList ps = path.split(",");
        // 遍历分割后的子字符串
        for (const QString &p : ps)
        {
            linepath.line_path_data.emplace_back(p.toFloat());
        }

        linepath.width = w;
        linepath.transform = glm::mat4(1.0);
        linepath.uv_offset = glm::vec2(off_x, off_y);
        linepath.uv_range = glm::vec2(range_x, range_y);
        linepath.use_tex_len = check_use_tex.checkState() == Qt::Checked;
        linepath.tex_len = len;
        info.add_node_line_pat(node, linepath);
        info.linepaths.emplace_back(linepath);
    }
}

void NodeTreeView::move_node(std::shared_ptr<sg::Node> node)
{
    auto p = node->parent();
    while (p->parent())
    {
        p = p->parent();
    }
    if (p->get_name() == "root")
    {
        // 创建对话框
        QDialog dialog(this->window());
        QVBoxLayout dialogLayout(&dialog);
        // 创建水平布局
        QHBoxLayout inputLayout;
        dialogLayout.addLayout(&inputLayout);

        QTreeView tree;
        inputLayout.addWidget(&tree);

        std::map<QStandardItem *, sg::Node *> node_map;
        QStandardItem *select_item;
        // 初始化modeltree
        QStandardItemModel *model = new QStandardItemModel(0, 1);
        auto item_root = new QStandardItem("root");
        model->appendRow(item_root);
        node_map.insert({item_root, p});
        std::function<void(sg::Node *, QStandardItem *, int)> pass_node = [&](sg::Node *node_t, QStandardItem *item, int id)
        {
            for (size_t i = 0; i < node_t->children().size(); i++)
            {
                auto sub_node = node_t->children()[i];
                if (editor::EditorWindows::getInstance().get_node_type(sub_node) != editor::NodeType::None)
                {
                    continue;
                }
                if (sub_node->id() == id)
                {
                    continue;
                }
                auto item_sub = new QStandardItem(sub_node->get_name().c_str());

                pass_node(sub_node.get(), item_sub, id);
                item->appendRow(item_sub);
                node_map.insert({item_sub, sub_node.get()});
            }
        };
        pass_node(p, item_root, node->parent()->id());

        tree.setModel(model);
        tree.setHeaderHidden(true);
        tree.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        tree.setEditTriggers(QAbstractItemView::NoEditTriggers);

        // 连接 selectionChanged 信号到自定义的槽函数
        QObject::connect(tree.selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected)
                         {
            // 获取被选中的索引
            QModelIndexList selectedIndexes = selected.indexes();
            // 处理选中行的逻辑
            for (const QModelIndex& index : selectedIndexes) {
                select_item = model->itemFromIndex(index);
              break;
            } });

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
                             auto des_node = node_map[select_item];
                             auto parent = node->parent();
                             parent->remove_child(node);
                             des_node->add_child(node);
                             editor::EditorWindows::getInstance().bing_treeView();
                             dialog.accept(); // 用户点击 "确定" 按钮，调用 accept() 函数
                         });

        QObject::connect(&rejectButton, &QPushButton::clicked, [&]()
                         {
                             dialog.reject(); // 用户点击 "取消" 按钮，调用 reject() 函数
                         });

        // 设置对话框的父窗口和标题
        dialog.setParent(this->window());
        dialog.setWindowTitle("select node");
        dialog.setModal(true); // 设置对话框为模态
        // 显示对话框并进入事件循环
        if (dialog.exec() == QDialog::Accepted)
        {
        }
    }
}

void ComponentScrollArea::mousePressEvent(QMouseEvent *event)
{
    return;
    // 创建右键菜单
    QMenu menu(this);

    // 创建菜单项
    QAction action1("Add_Material", this);
    QAction action2("Add_Rect", this);
    QAction action3("Add_Line_Path", this);

    // 将菜单项添加到菜单中
    menu.addAction(&action1);
    menu.addAction(&action2);
    menu.addAction(&action3);

    // 获取鼠标点击的位置
    QPoint globalPos = event->globalPos();
    auto window_editor = &editor::EditorWindows::getInstance();
    // 在指定位置显示菜单
    QAction *selectedAction = menu.exec(globalPos);
    if (selectedAction == &action1)
    {
    }
    else if (selectedAction == &action2)
    {
    }
    else if (selectedAction == &action3)
    {
    }
}
