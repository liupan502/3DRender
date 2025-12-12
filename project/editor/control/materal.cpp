#include "materal.h"
#include "material_ui.h"
#include "transform_ui.h"
#include "camera_ui.h"
#include "directional_light_ui.h"
#include "animation_manager.h"
#include "editor_windows.h"
#include <QColorDialog>

Ui::Materal::Materal(QWidget *parent) : QGroupBox(parent),
                                        ui(new Ui::Material_UI)
{
    ui->setupUi(this);
    _ck_transparent = this->findChild<QCheckBox *>("ck_transparent");
    QObject::connect(_ck_transparent, &QCheckBox::stateChanged, [&](int state)
                     {
                        if(_node){
                        auto mat = _node->get_component<zr::sg::Material>();
                        mat->set_transparent_enabled(state == Qt::Checked);
                        } });
    _ck_environment = this->findChild<QCheckBox *>("ck_environment");
    QObject::connect(_ck_environment, &QCheckBox::stateChanged, [&](int state)
                     {
                        if(_node){
                        auto mat = _node->get_component<zr::sg::Material>();
                        mat->set_environment_enabled(state == Qt::Checked);
                        } });
    _ck_light = this->findChild<QCheckBox *>("ck_light");
    QObject::connect(_ck_light, &QCheckBox::stateChanged, [&](int state)
                     {
                        if(_node){
                        auto mat = _node->get_component<zr::sg::Material>();
                        mat->set_light_enabled(state == Qt::Checked);
                        } });

    auto bingding_slider_spinBox = [&](QSlider *slider, QDoubleSpinBox *spinBox, Pbr_Params pbr_params)
    {
        // 通过信号和槽连接 QSlider 和 QDoubleSpinBox 的值
        QObject::connect(slider, &QSlider::valueChanged, spinBox, [=](int value)
                         {
                             double doubleValue = static_cast<double>(value) / 50.0; // 将 QSlider 的值映射到 QDoubleSpinBox 的范围
                              spinBox->setValue(doubleValue); });

        QObject::connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), slider, [=](double value)
                         {
                             int intValue = static_cast<int>(value * 50.0); // 将 QDoubleSpinBox 的值映射到 QSlider 的范围
                             slider->setValue(intValue);                     // 设置 QSlider 的值
                             auto node= editor::EditorWindows::getInstance().get_select_node();
                             if(node){
                                auto mat = node->get_component<zr::sg::Material>();
                                auto params = mat->get_pbr_params();
                                switch (pbr_params)
                                {
                                    case  Pbr_Params::Roughness:
                                        params.roughness=value;
                                        break;
                                    case   Pbr_Params::Ior:
                                        params.ior=value;
                                        break;
                                    case   Pbr_Params::Metallic:
                                        params.metallic=value;
                                        break;
                                    case   Pbr_Params::Clear_coat:
                                        params.clear_coat=value;
                                        break;
                                    case   Pbr_Params::Clear_coat_roughness:
                                        params.clear_coat_roughness=value;
                                        break;
                                    default:
                                        break;
                                }
                                mat->set_pbr_params(params);
                             } });
    };
    _hs_metallic = this->findChild<QSlider *>("hs_metallic");
    _hs_roughness = this->findChild<QSlider *>("hs_roughness");
    _hs_ior = this->findChild<QSlider *>("hs_ior");
    _hs_clear_coat = this->findChild<QSlider *>("hs_clear_coat");
    _hs_clear_coat_roughness = this->findChild<QSlider *>("hs_clear_coat_roughness");

    _sb_metallic = this->findChild<QDoubleSpinBox *>("sb_metallic");
    _sb_roughness = this->findChild<QDoubleSpinBox *>("sb_roughness");
    _sb_ior = this->findChild<QDoubleSpinBox *>("sb_ior");
    _sb_clear_coat = this->findChild<QDoubleSpinBox *>("sb_clear_coat");
    _sb_clear_coat_roughness = this->findChild<QDoubleSpinBox *>("sb_clear_coat_roughness");

    bingding_slider_spinBox(_hs_metallic, _sb_metallic, Pbr_Params::Metallic);
    bingding_slider_spinBox(_hs_roughness, _sb_roughness, Pbr_Params::Roughness);
    bingding_slider_spinBox(_hs_ior, _sb_ior, Pbr_Params::Ior);
    bingding_slider_spinBox(_hs_clear_coat, _sb_clear_coat, Pbr_Params::Clear_coat);
    bingding_slider_spinBox(_hs_clear_coat_roughness, _sb_clear_coat_roughness, Pbr_Params::Clear_coat_roughness);

    _le_emissive_r = this->findChild<QLineEdit *>("le_emissive_r");
    _le_emissive_r->setValidator(new QDoubleValidator(this));
    _le_emissive_g = this->findChild<QLineEdit *>("le_emissive_g");
    _le_emissive_g->setValidator(new QDoubleValidator(this));
    _le_emissive_b = this->findChild<QLineEdit *>("le_emissive_b");
    _le_emissive_b->setValidator(new QDoubleValidator(this));
    _le_emissive_w = this->findChild<QLineEdit *>("le_emissive_w");
    _le_emissive_w->setValidator(new QDoubleValidator(this));
    auto update_emissive = [&]()
    {
        auto r = _le_emissive_r->text().toFloat();
        auto g = _le_emissive_g->text().toFloat();
        auto b = _le_emissive_b->text().toFloat();
        auto w = _le_emissive_w->text().toFloat();
        _node->get_component<zr::sg::Material>()->set_emissive_color(glm::vec4(r, g, b, w));
    };
    connect(_le_emissive_r, &QLineEdit::textChanged, this, update_emissive);
    connect(_le_emissive_g, &QLineEdit::textChanged, this, update_emissive);
    connect(_le_emissive_b, &QLineEdit::textChanged, this, update_emissive);
    connect(_le_emissive_w, &QLineEdit::textChanged, this, update_emissive);

    _bt_emissive = this->findChild<QPushButton *>("bt_emissive");
    connect(_bt_emissive, &QPushButton::clicked, this, [&]()
            {
            // 创建一个 QColorDialog 对象
            QColorDialog dialog;

            // 设置对话框的标题
            dialog.setWindowTitle("Color Selector");

            // 设置初始颜色
            dialog.setCurrentColor(Qt::red);

            // 打开颜色选择器对话框
            if (dialog.exec() == QDialog::Accepted)
            {
                // 获取用户选择的颜色
                QColor color = dialog.currentColor();
                _le_emissive_r->setText(QString::number(color.red()/255.00, 'f', 2));
                _le_emissive_g->setText(QString::number(color.green()/255.00, 'f', 2));
                _le_emissive_b->setText(QString::number(color.blue()/255.00, 'f', 2));
                if(_node){
                    auto w= _le_emissive_w->text().toFloat();
                    _node->get_component<zr::sg::Material>()->set_emissive_color(glm::vec4(color.red()/255.00,color.green()/255.00,color.blue()/255.00,w));
                }
            } });

    _le_diffuse = this->findChild<QLineEdit *>("le_diffuse");
    _bt_diffuse = this->findChild<QPushButton *>("bt_diffuse");
    connect(_bt_diffuse, &QPushButton::clicked, this, [&]()
            {
                auto bass_dir = editor::EditorWindows::getInstance().get_assert_base_dir();
                QString arg("img files (*.png)");
                QString fileName = QFileDialog::getOpenFileName(
                    this->window(), "Open File", bass_dir.c_str(),
                    "img files (*.png)", &arg);
                QFileInfo fileInfo(fileName);
                if (fileInfo.exists())
                {
                    if (fileName.startsWith(bass_dir.c_str()))
                    {
                        QString img_path = fileName.mid(bass_dir.size(), fileName.size() - bass_dir.size());
                        _le_diffuse->setText(img_path);
                        std::shared_ptr<zr::sg::Texture> texture = std::make_shared<zr::sg::SingleLayerTexture>(img_path.toStdString());
                        _node->get_component<zr::sg::Material>()->update_texture(zr::sg::TextureType::TEXTURE_TYPE_DIFFUSE,texture);
                        auto &info = editor::EditorWindows::getInstance().get_geometry_node_info();
                        info.textrue_path=img_path.toStdString();
                    }
                    else
                    {
                        QMessageBox::information(this->window(), "打开文件", "不能选取不是资源路径的img文件 请先将文件拷贝到资源路径下再载入 ");
                    }
                } });
}

Ui::Materal::~Materal()
{
    delete ui;
}

void Ui::Materal::binding_node()
{
    this->setVisible(true);
    _node = editor::EditorWindows::getInstance().get_select_node();
    auto type = editor::EditorWindows::getInstance().get_node_type(_node);
    if (type == editor::NodeType::Geometry)
    {
        _bt_diffuse->setEnabled(true);
    }
    else
    {
        _bt_diffuse->setEnabled(false);
    }
    auto mat = _node->get_component<zr::sg::Material>();

    _ck_transparent->setChecked(mat->get_transparent_enabled());
    _ck_environment->setChecked(mat->get_environment_enabled());
    _ck_light->setChecked(mat->get_light_enabled());

    auto pbr_params = mat->get_pbr_params();

    _hs_metallic->setValue((int)(pbr_params.metallic * 50));
    _hs_roughness->setValue((int)(pbr_params.roughness * 50));
    _hs_ior->setValue((int)(pbr_params.ior * 50));
    _hs_clear_coat->setValue((int)(pbr_params.clear_coat * 50));
    _hs_clear_coat_roughness->setValue((int)(pbr_params.clear_coat_roughness * 50));

    _sb_metallic->setValue(pbr_params.metallic);
    _sb_roughness->setValue(pbr_params.roughness);
    _sb_ior->setValue(pbr_params.ior);
    _sb_clear_coat->setValue(pbr_params.clear_coat);
    _sb_clear_coat_roughness->setValue(pbr_params.clear_coat_roughness);

    auto color = mat->get_emissive_color();

    _le_emissive_r->setText(QString::number(color.r, 'f', 2));
    _le_emissive_g->setText(QString::number(color.g, 'f', 2));
    _le_emissive_b->setText(QString::number(color.b, 'f', 2));
    _le_emissive_w->setText(QString::number(color.a, 'f', 2));
    auto node_t = std::dynamic_pointer_cast<sg::GeometryNode>(_node);
    if (node_t)
    {

        auto &info = editor::EditorWindows::getInstance().get_geometry_node_info();
        _le_diffuse->setText(info.textrue_path.c_str());
    }
}

Ui::Transform::Transform(QWidget *parent) : QGroupBox(parent),
                                            ui(new Ui::Transform_UI)
{
    ui->setupUi(this);
    _lb_p_x = this->findChild<QLineEdit *>("lb_p_x");
    _lb_p_x->setValidator(new QDoubleValidator(this));
    _lb_p_y = this->findChild<QLineEdit *>("lb_p_y");
    _lb_p_y->setValidator(new QDoubleValidator(this));
    _lb_p_z = this->findChild<QLineEdit *>("lb_p_z");
    _lb_p_z->setValidator(new QDoubleValidator(this));
    _lb_r_x = this->findChild<QLineEdit *>("lb_r_x");
    _lb_r_x->setValidator(new QDoubleValidator(this));
    _lb_r_y = this->findChild<QLineEdit *>("lb_r_y");
    _lb_r_y->setValidator(new QDoubleValidator(this));
    _lb_r_z = this->findChild<QLineEdit *>("lb_r_z");
    _lb_r_z->setValidator(new QDoubleValidator(this));
    _lb_s_x = this->findChild<QLineEdit *>("lb_s_x");
    _lb_s_x->setValidator(new QDoubleValidator(this));
    _lb_s_y = this->findChild<QLineEdit *>("lb_s_y");
    _lb_s_y->setValidator(new QDoubleValidator(this));
    _lb_s_z = this->findChild<QLineEdit *>("lb_s_z");
    _lb_s_z->setValidator(new QDoubleValidator(this));

    // 欧拉角转换为旋转矩阵
    auto eulerToMatrix = [](glm::vec3 const &eulerAngles) -> glm::mat4
    {
        auto quat = glm::quat(glm::radians(eulerAngles));
        return glm::mat4_cast(quat);
        glm::mat4 rotationMatrix(1.0f);                                                                         // 初始化为单位矩阵
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(eulerAngles.z), glm::vec3(0.0f, 0.0f, 1.0f)); // 绕 Z 轴旋转
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(eulerAngles.x), glm::vec3(1.0f, 0.0f, 0.0f)); // 绕 X 轴旋转
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(eulerAngles.y), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕 Y 轴旋转

        return rotationMatrix;
    };

    auto update_transform = [&]()
    {
        glm::vec3 p{_lb_p_x->text().toFloat(), _lb_p_y->text().toFloat(), _lb_p_z->text().toFloat()};

        glm::vec3 r{_lb_r_x->text().toFloat(), _lb_r_y->text().toFloat(), _lb_r_z->text().toFloat()};

        glm::vec3 s{_lb_s_x->text().toFloat(), _lb_s_y->text().toFloat(), _lb_s_z->text().toFloat()};

        auto translate = glm::translate(glm::mat4(1.0), p);
        auto rotate = eulerToMatrix(r);
        auto scale = glm::scale(glm::mat4(1.0), s);

        _node->get_component<zr::sg::Transform>()->set_matrix(translate * rotate * scale);
    };
    connect(_lb_p_x, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_p_y, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_p_z, &QLineEdit::textChanged, this, update_transform);

    connect(_lb_r_x, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_r_y, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_r_z, &QLineEdit::textChanged, this, update_transform);

    connect(_lb_s_x, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_s_y, &QLineEdit::textChanged, this, update_transform);
    connect(_lb_s_z, &QLineEdit::textChanged, this, update_transform);
}

Ui::Transform::~Transform()
{
    delete ui;
}

void Ui::Transform::binding_node()
{
    this->setVisible(true);
    _node = editor::EditorWindows::getInstance().get_select_node();
    auto tf = _node->get_component<zr::sg::Transform>();
    auto translation = tf->get_translation();
    auto rotation = glm::eulerAngles(tf->get_rotation());
    auto scale = tf->get_scale();

    _lb_p_x->setText(QString::number(translation.x, 'f', 2));
    _lb_p_y->setText(QString::number(translation.y, 'f', 2));
    _lb_p_z->setText(QString::number(translation.z, 'f', 2));

    _lb_r_x->setText(QString::number(glm::degrees(rotation.x), 'f', 2));
    _lb_r_y->setText(QString::number(glm::degrees(rotation.y), 'f', 2));
    _lb_r_z->setText(QString::number(glm::degrees(rotation.z), 'f', 2));

    _lb_s_x->setText(QString::number(scale.x, 'f', 2));
    _lb_s_y->setText(QString::number(scale.y, 'f', 2));
    _lb_s_z->setText(QString::number(scale.z, 'f', 2));
}

Ui::Camera::Camera(QWidget *parent) : QGroupBox(parent),
                                      ui(new Ui::Camera_UI)
{
    ui->setupUi(this);

    _lb_p_x = this->findChild<QLineEdit *>("lb_p_x");
    _lb_p_x->setValidator(new QDoubleValidator(this));
    _lb_p_y = this->findChild<QLineEdit *>("lb_p_y");
    _lb_p_y->setValidator(new QDoubleValidator(this));
    _lb_p_z = this->findChild<QLineEdit *>("lb_p_z");
    _lb_p_z->setValidator(new QDoubleValidator(this));
    _lb_r_x = this->findChild<QLineEdit *>("lb_r_x");
    _lb_r_x->setValidator(new QDoubleValidator(this));
    _lb_r_y = this->findChild<QLineEdit *>("lb_r_y");
    _lb_r_y->setValidator(new QDoubleValidator(this));
    _lb_r_z = this->findChild<QLineEdit *>("lb_r_z");
    _lb_r_z->setValidator(new QDoubleValidator(this));
    _lb_s_x = this->findChild<QLineEdit *>("lb_s_x");
    _lb_s_x->setValidator(new QDoubleValidator(this));
    _lb_s_y = this->findChild<QLineEdit *>("lb_s_y");
    _lb_s_y->setValidator(new QDoubleValidator(this));
    _lb_s_z = this->findChild<QLineEdit *>("lb_s_z");
    _lb_s_z->setValidator(new QDoubleValidator(this));

    auto update_camera = [&]()
    {
        glm::vec3 p{_lb_p_x->text().toFloat(), _lb_p_y->text().toFloat(), _lb_p_z->text().toFloat()};

        glm::vec3 t{_lb_r_x->text().toFloat(), _lb_r_y->text().toFloat(), _lb_r_z->text().toFloat()};

        glm::vec3 u{_lb_s_x->text().toFloat(), _lb_s_y->text().toFloat(), _lb_s_z->text().toFloat()};

        _node->get_component<zr::sg::PerspectiveCamera>()->look_at(p, t, u);
    };
    connect(_lb_p_x, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_p_y, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_p_z, &QLineEdit::textChanged, this, update_camera);

    connect(_lb_r_x, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_r_y, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_r_z, &QLineEdit::textChanged, this, update_camera);

    connect(_lb_s_x, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_s_y, &QLineEdit::textChanged, this, update_camera);
    connect(_lb_s_z, &QLineEdit::textChanged, this, update_camera);
}

Ui::Camera::~Camera()
{
    delete ui;
}

void Ui::Camera::binding_node()
{
    this->setVisible(true);
    _node = editor::EditorWindows::getInstance().get_select_node();
    auto camera = _node->get_component<zr::sg::PerspectiveCamera>();
    auto pos = camera->get_pos();
    auto target = camera->get_target();
    auto up = camera->get_up();

    _lb_p_x->setText(QString::number(pos.x, 'f', 2));
    _lb_p_y->setText(QString::number(pos.y, 'f', 2));
    _lb_p_z->setText(QString::number(pos.z, 'f', 2));

    _lb_r_x->setText(QString::number(target.x, 'f', 2));
    _lb_r_y->setText(QString::number(target.y, 'f', 2));
    _lb_r_z->setText(QString::number(target.z, 'f', 2));

    _lb_s_x->setText(QString::number(up.x, 'f', 2));
    _lb_s_y->setText(QString::number(up.y, 'f', 2));
    _lb_s_z->setText(QString::number(up.z, 'f', 2));
}

Ui::Directional_Light::Directional_Light(QWidget *parent) : QGroupBox(parent),
                                                            ui(new Ui::Directional_Light_UI)
{
    ui->setupUi(this);

    _lb_d_x = this->findChild<QLineEdit *>("lb_d_x");
    _lb_d_x->setValidator(new QDoubleValidator(this));
    _lb_d_y = this->findChild<QLineEdit *>("lb_d_y");
    _lb_d_y->setValidator(new QDoubleValidator(this));
    _lb_d_z = this->findChild<QLineEdit *>("lb_d_z");
    _lb_d_z->setValidator(new QDoubleValidator(this));

    _lb_c_r = this->findChild<QLineEdit *>("lb_c_r");
    _lb_c_r->setValidator(new QDoubleValidator(this));
    _lb_c_g = this->findChild<QLineEdit *>("lb_c_g");
    _lb_c_g->setValidator(new QDoubleValidator(this));
    _lb_c_b = this->findChild<QLineEdit *>("lb_c_b");
    _lb_c_b->setValidator(new QDoubleValidator(this));
    _lb_c_w = this->findChild<QLineEdit *>("lb_c_w");
    _lb_c_w->setValidator(new QDoubleValidator(this));

    auto update_light = [&]()
    {
        glm::vec3 d{_lb_d_x->text().toFloat(), _lb_d_y->text().toFloat(), _lb_d_z->text().toFloat()};

        glm::vec4 c{_lb_c_r->text().toFloat(), _lb_c_g->text().toFloat(), _lb_c_b->text().toFloat(), _lb_c_w->text().toFloat()};

        _node->get_component<zr::sg::DirectionalLight>()->set_params(d, c);
    };
    connect(_lb_d_x, &QLineEdit::textChanged, this, update_light);
    connect(_lb_d_y, &QLineEdit::textChanged, this, update_light);
    connect(_lb_d_z, &QLineEdit::textChanged, this, update_light);

    connect(_lb_c_r, &QLineEdit::textChanged, this, update_light);
    connect(_lb_c_g, &QLineEdit::textChanged, this, update_light);
    connect(_lb_c_b, &QLineEdit::textChanged, this, update_light);
    connect(_lb_c_w, &QLineEdit::textChanged, this, update_light);

    _bt_color = this->findChild<QPushButton *>("bt_color");
    connect(_bt_color, &QPushButton::clicked, this, [&]()
            {
            // 创建一个 QColorDialog 对象
            QColorDialog dialog;

            // 设置对话框的标题
            dialog.setWindowTitle("Color Selector");

            // 设置初始颜色
            dialog.setCurrentColor(Qt::red);

            // 打开颜色选择器对话框
            if (dialog.exec() == QDialog::Accepted)
            {
                // 获取用户选择的颜色
                QColor color = dialog.currentColor();
                _lb_c_r->setText(QString::number(color.red()/255.00, 'f', 2));
                _lb_c_g->setText(QString::number(color.green()/255.00, 'f', 2));
                _lb_c_b->setText(QString::number(color.blue()/255.00, 'f', 2));
                if(_node){
                    auto d_light = _node->get_component<zr::sg::DirectionalLight>();
                    d_light->set_params(d_light->get_light_info().direction,glm::vec4(color.red()/255.00,color.green()/255.00,color.blue()/255.00,d_light->get_light_info().color.w));
                }
            } });
}

Ui::Directional_Light::~Directional_Light()
{
    delete ui;
}

void Ui::Directional_Light::binding_node()
{
    this->setVisible(true);
    _node = editor::EditorWindows::getInstance().get_select_node();
    auto d_light = _node->get_component<zr::sg::DirectionalLight>();
    auto dir = d_light->get_light_info().direction;
    auto clo = d_light->get_light_info().color;

    _lb_d_x->setText(QString::number(dir.x, 'f', 2));
    _lb_d_y->setText(QString::number(dir.y, 'f', 2));
    _lb_d_z->setText(QString::number(dir.z, 'f', 2));

    _lb_c_r->setText(QString::number(clo.r, 'f', 2));
    _lb_c_g->setText(QString::number(clo.g, 'f', 2));
    _lb_c_b->setText(QString::number(clo.b, 'f', 2));
    _lb_c_w->setText(QString::number(clo.w, 'f', 2));
}

Ui::Animation_Manager::Animation_Manager(QWidget *parent) : QGroupBox(parent),
                                                            ui(new Ui::AnimationManager_UI)
{

    ui->setupUi(this);
    _view = this->findChild<QTreeView *>("tvAnimation");
}

Ui::Animation_Manager::~Animation_Manager()
{
    delete ui;
}

void Ui::Animation_Manager::binding_node()
{

    auto set_check = [](QStandardItem *item, bool ischeck)
    {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable); // 禁止编辑文本
        item->setCheckable(true);
        if (ischeck)
        {
            item->setCheckState(Qt::CheckState::Checked);
        }
        else
        {
            item->setCheckState(Qt::CheckState::Unchecked);
        }
    };
    this->setVisible(true);
    _node = editor::EditorWindows::getInstance().get_select_node();
    auto a_manager = _node->get_component<zr::sg::AnimationManager>();

    // 初始化modeltree
    QStandardItemModel *model = new QStandardItemModel(0, 3);

    auto animations = a_manager->get_animations();
    for (auto animation : animations)
    {

        QList<QStandardItem *> rowItems;

        auto state = animation.second->get_state();
        auto animation_i = new QStandardItem(animation.first.c_str());

        auto play = new QStandardItem("play");
        set_check(play, state == sg::Animation::AnimationState::ANIMATION_STATE_PLAYING);

        auto loop = new QStandardItem("loop");
        set_check(loop, animation.second->get_is_looped());

        rowItems << animation_i << play << loop;
        // for (int i=0;i<10;i++)
        model->appendRow(rowItems);
    }

    QObject::connect(model, &QStandardItemModel::dataChanged,
                     [=](const QModelIndex &topLeft, const QModelIndex &bottomRight)
                     {
                         QStandardItem *item = model->itemFromIndex(topLeft);
                         Qt::CheckState state = item->checkState();
                          // 获取当前行的其他列的值
                         QStandardItem *name_item = model->item(topLeft.row(), 0);
                         auto animations = _node->get_animation(name_item->text().toStdString());
                         for (auto animation : animations)
                         {
                             if (item->text().compare("play") == 0)
                                 if (state == Qt::CheckState::Checked)
                                     animation->play();
                                 else if (animation->get_state() == sg::Animation::AnimationState::ANIMATION_STATE_PLAYING)
                                     animation->pause();
                                 else
                                     animation->stop();
                             else
                                 animation->set_is_looped(state == Qt::CheckState::Checked);
                         }
                     });
    _view->setModel(model);
    _view->setHeaderHidden(true);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}
