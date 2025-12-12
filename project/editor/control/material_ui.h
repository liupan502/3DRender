/********************************************************************************
** Form generated from reading UI file 'material.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MATERIAL_UI_H
#define MATERIAL_UI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_Material_UI
{
public:
    QCheckBox *ck_transparent;
    QCheckBox *ck_environment;
    QCheckBox *ck_light;
    QLabel *label;
    QSlider *hs_metallic;
    QLabel *label_2;
    QSlider *hs_roughness;
    QLabel *label_3;
    QSlider *hs_ior;
    QLabel *label_4;
    QSlider *hs_clear_coat;
    QLabel *label_5;
    QSlider *hs_clear_coat_roughness;
    QLabel *label_6;
    QLabel *label_7;
    QLineEdit *le_emissive_r;
    QLabel *label_8;
    QLineEdit *le_emissive_g;
    QLabel *label_9;
    QLineEdit *le_emissive_b;
    QLabel *label_10;
    QLineEdit *le_emissive_w;
    QPushButton *bt_emissive;
    QDoubleSpinBox *sb_metallic;
    QDoubleSpinBox *sb_clear_coat;
    QDoubleSpinBox *sb_clear_coat_roughness;
    QDoubleSpinBox *sb_roughness;
    QDoubleSpinBox *sb_ior;
    QLabel *label_11;
    QLineEdit *le_diffuse;
    QPushButton *bt_diffuse;

    void setupUi(QGroupBox *Material_UI)
    {
        if (Material_UI->objectName().isEmpty())
            Material_UI->setObjectName(QString::fromUtf8("Material_UI"));
        Material_UI->resize(440, 279);
        Material_UI->setFlat(false);
        Material_UI->setCheckable(false);
        ck_transparent = new QCheckBox(Material_UI);
        ck_transparent->setObjectName(QString::fromUtf8("ck_transparent"));
        ck_transparent->setGeometry(QRect(10, 30, 111, 23));
        ck_environment = new QCheckBox(Material_UI);
        ck_environment->setObjectName(QString::fromUtf8("ck_environment"));
        ck_environment->setGeometry(QRect(150, 30, 111, 23));
        ck_light = new QCheckBox(Material_UI);
        ck_light->setObjectName(QString::fromUtf8("ck_light"));
        ck_light->setGeometry(QRect(320, 30, 61, 23));
        label = new QLabel(Material_UI);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 60, 67, 17));
        hs_metallic = new QSlider(Material_UI);
        hs_metallic->setObjectName(QString::fromUtf8("hs_metallic"));
        hs_metallic->setGeometry(QRect(170, 60, 160, 16));
        hs_metallic->setOrientation(Qt::Horizontal);
        label_2 = new QLabel(Material_UI);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 90, 91, 17));
        hs_roughness = new QSlider(Material_UI);
        hs_roughness->setObjectName(QString::fromUtf8("hs_roughness"));
        hs_roughness->setGeometry(QRect(170, 90, 160, 16));
        hs_roughness->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(Material_UI);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 120, 67, 17));
        hs_ior = new QSlider(Material_UI);
        hs_ior->setObjectName(QString::fromUtf8("hs_ior"));
        hs_ior->setGeometry(QRect(170, 120, 160, 16));
        hs_ior->setOrientation(Qt::Horizontal);
        label_4 = new QLabel(Material_UI);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 150, 91, 17));
        hs_clear_coat = new QSlider(Material_UI);
        hs_clear_coat->setObjectName(QString::fromUtf8("hs_clear_coat"));
        hs_clear_coat->setGeometry(QRect(170, 150, 160, 16));
        hs_clear_coat->setOrientation(Qt::Horizontal);
        label_5 = new QLabel(Material_UI);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 180, 141, 17));
        hs_clear_coat_roughness = new QSlider(Material_UI);
        hs_clear_coat_roughness->setObjectName(QString::fromUtf8("hs_clear_coat_roughness"));
        hs_clear_coat_roughness->setGeometry(QRect(170, 180, 160, 16));
        hs_clear_coat_roughness->setOrientation(Qt::Horizontal);
        label_6 = new QLabel(Material_UI);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 210, 67, 17));
        label_7 = new QLabel(Material_UI);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(90, 210, 20, 20));
        le_emissive_r = new QLineEdit(Material_UI);
        le_emissive_r->setObjectName(QString::fromUtf8("le_emissive_r"));
        le_emissive_r->setGeometry(QRect(100, 210, 51, 21));
        label_8 = new QLabel(Material_UI);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(150, 210, 20, 20));
        le_emissive_g = new QLineEdit(Material_UI);
        le_emissive_g->setObjectName(QString::fromUtf8("le_emissive_g"));
        le_emissive_g->setGeometry(QRect(160, 210, 51, 21));
        label_9 = new QLabel(Material_UI);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(210, 210, 20, 20));
        le_emissive_b = new QLineEdit(Material_UI);
        le_emissive_b->setObjectName(QString::fromUtf8("le_emissive_b"));
        le_emissive_b->setGeometry(QRect(220, 210, 51, 21));
        label_10 = new QLabel(Material_UI);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(270, 210, 20, 20));
        le_emissive_w = new QLineEdit(Material_UI);
        le_emissive_w->setObjectName(QString::fromUtf8("le_emissive_w"));
        le_emissive_w->setGeometry(QRect(290, 210, 51, 21));
        bt_emissive = new QPushButton(Material_UI);
        bt_emissive->setObjectName(QString::fromUtf8("bt_emissive"));
        bt_emissive->setGeometry(QRect(360, 210, 51, 25));
        sb_metallic = new QDoubleSpinBox(Material_UI);
        sb_metallic->setObjectName(QString::fromUtf8("sb_metallic"));
        sb_metallic->setGeometry(QRect(360, 50, 71, 26));
        sb_metallic->setDecimals(4);
        sb_metallic->setMaximum(2.000000000000000);
        sb_metallic->setSingleStep(0.010000000000000);
        sb_clear_coat = new QDoubleSpinBox(Material_UI);
        sb_clear_coat->setObjectName(QString::fromUtf8("sb_clear_coat"));
        sb_clear_coat->setGeometry(QRect(360, 140, 71, 26));
        sb_clear_coat->setDecimals(4);
        sb_clear_coat->setMaximum(2.000000000000000);
        sb_clear_coat->setSingleStep(0.010000000000000);
        sb_clear_coat_roughness = new QDoubleSpinBox(Material_UI);
        sb_clear_coat_roughness->setObjectName(QString::fromUtf8("sb_clear_coat_roughness"));
        sb_clear_coat_roughness->setGeometry(QRect(360, 170, 71, 26));
        sb_clear_coat_roughness->setDecimals(4);
        sb_clear_coat_roughness->setMaximum(2.000000000000000);
        sb_clear_coat_roughness->setSingleStep(0.010000000000000);
        sb_roughness = new QDoubleSpinBox(Material_UI);
        sb_roughness->setObjectName(QString::fromUtf8("sb_roughness"));
        sb_roughness->setGeometry(QRect(360, 80, 71, 26));
        sb_roughness->setDecimals(4);
        sb_roughness->setMaximum(2.000000000000000);
        sb_roughness->setSingleStep(0.010000000000000);
        sb_ior = new QDoubleSpinBox(Material_UI);
        sb_ior->setObjectName(QString::fromUtf8("sb_ior"));
        sb_ior->setGeometry(QRect(360, 110, 71, 26));
        sb_ior->setDecimals(4);
        sb_ior->setMaximum(2.000000000000000);
        sb_ior->setSingleStep(0.010000000000000);
        label_11 = new QLabel(Material_UI);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(10, 240, 67, 17));
        le_diffuse = new QLineEdit(Material_UI);
        le_diffuse->setObjectName(QString::fromUtf8("le_diffuse"));
        le_diffuse->setGeometry(QRect(90, 240, 251, 25));
        le_diffuse->setReadOnly(true);
        bt_diffuse = new QPushButton(Material_UI);
        bt_diffuse->setObjectName(QString::fromUtf8("bt_diffuse"));
        bt_diffuse->setGeometry(QRect(360, 240, 51, 25));

        retranslateUi(Material_UI);

        QMetaObject::connectSlotsByName(Material_UI);
    } // setupUi

    void retranslateUi(QGroupBox *Material_UI)
    {
        Material_UI->setWindowTitle(QApplication::translate("Material_UI", "GroupBox", nullptr));
        Material_UI->setTitle(QApplication::translate("Material_UI", "Material", nullptr));
        ck_transparent->setText(QApplication::translate("Material_UI", "transparent", nullptr));
        ck_environment->setText(QApplication::translate("Material_UI", "environment", nullptr));
        ck_light->setText(QApplication::translate("Material_UI", "light", nullptr));
        label->setText(QApplication::translate("Material_UI", "metallic", nullptr));
        label_2->setText(QApplication::translate("Material_UI", "roughness", nullptr));
        label_3->setText(QApplication::translate("Material_UI", "ior", nullptr));
        label_4->setText(QApplication::translate("Material_UI", "clear_coat", nullptr));
        label_5->setText(QApplication::translate("Material_UI", "clear_coat_roughness", nullptr));
        label_6->setText(QApplication::translate("Material_UI", "emissive", nullptr));
        label_7->setText(QApplication::translate("Material_UI", "R", nullptr));
        label_8->setText(QApplication::translate("Material_UI", "G", nullptr));
        label_9->setText(QApplication::translate("Material_UI", "B", nullptr));
        label_10->setText(QApplication::translate("Material_UI", "W", nullptr));
        bt_emissive->setText(QApplication::translate("Material_UI", "\351\200\211\346\213\251", nullptr));
        label_11->setText(QApplication::translate("Material_UI", "diffuse", nullptr));
        bt_diffuse->setText(QApplication::translate("Material_UI", "\351\200\211\346\213\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Material_UI: public Ui_Material_UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MATERIAL_UI_H
