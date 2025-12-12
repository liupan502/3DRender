#pragma once
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include <QTreeView>
namespace zr::sg
{

    class Node;
}

namespace Ui
{
    enum Pbr_Params
    {
        Roughness,
        Ior,
        Metallic,
        Clear_coat,
        Clear_coat_roughness

    };
    class Material_UI;
    class Materal : public QGroupBox
    {
    public:
        explicit Materal(QWidget *parent = nullptr);
        ~Materal();
        void binding_node();

    private:
        std::shared_ptr<zr::sg::Node> _node;
        Material_UI *ui;
        QCheckBox *_ck_transparent;
        QCheckBox *_ck_environment;
        QCheckBox *_ck_light;

        QSlider *_hs_metallic;
        QSlider *_hs_roughness;
        QSlider *_hs_ior;
        QSlider *_hs_clear_coat;
        QSlider *_hs_clear_coat_roughness;

        QDoubleSpinBox *_sb_metallic;
        QDoubleSpinBox *_sb_roughness;
        QDoubleSpinBox *_sb_ior;
        QDoubleSpinBox *_sb_clear_coat;
        QDoubleSpinBox *_sb_clear_coat_roughness;

        QLineEdit *_le_emissive_r;
        QLineEdit *_le_emissive_g;
        QLineEdit *_le_emissive_b;
        QLineEdit *_le_emissive_w;

        QPushButton *_bt_emissive;

        QLineEdit *_le_diffuse;
        QPushButton *_bt_diffuse;
    };

    class Transform_UI;
    class Transform : public QGroupBox
    {
    public:
        explicit Transform(QWidget *parent = nullptr);
        ~Transform();
        void binding_node();

    private:
        std::shared_ptr<zr::sg::Node> _node;

        Transform_UI *ui;
        QLineEdit *_lb_p_x;
        QLineEdit *_lb_p_y;
        QLineEdit *_lb_p_z;
        QLineEdit *_lb_r_x;
        QLineEdit *_lb_r_y;
        QLineEdit *_lb_r_z;
        QLineEdit *_lb_s_x;
        QLineEdit *_lb_s_y;
        QLineEdit *_lb_s_z;
    };

    class Camera_UI;
    class Camera : public QGroupBox
    {
    public:
        explicit Camera(QWidget *parent = nullptr);
        ~Camera();
        void binding_node();

    private:
        std::shared_ptr<zr::sg::Node> _node;

        Camera_UI *ui;
        QLineEdit *_lb_p_x;
        QLineEdit *_lb_p_y;
        QLineEdit *_lb_p_z;
        QLineEdit *_lb_r_x;
        QLineEdit *_lb_r_y;
        QLineEdit *_lb_r_z;
        QLineEdit *_lb_s_x;
        QLineEdit *_lb_s_y;
        QLineEdit *_lb_s_z;
    };

    class Directional_Light_UI;
    class Directional_Light : public QGroupBox
    {
    public:
        explicit Directional_Light(QWidget *parent = nullptr);
        ~Directional_Light();
        void binding_node();

    private:
        std::shared_ptr<zr::sg::Node> _node;

        Directional_Light_UI *ui;
        QLineEdit *_lb_d_x;
        QLineEdit *_lb_d_y;
        QLineEdit *_lb_d_z;
        QLineEdit *_lb_c_r;
        QLineEdit *_lb_c_g;
        QLineEdit *_lb_c_b;
        QLineEdit *_lb_c_w;
        QPushButton *_bt_color;
    };

    class AnimationManager_UI;
    class Animation_Manager : public QGroupBox
    {
    public:
        explicit Animation_Manager(QWidget *parent = nullptr);
        ~Animation_Manager();
        void binding_node();

    private:
        std::shared_ptr<zr::sg::Node> _node;
        AnimationManager_UI *ui;
        QTreeView * _view;
    };

}