/********************************************************************************
** Form generated from reading UI file 'directional_light.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DIRECTIONAL_LIGHT_UI_H
#define DIRECTIONAL_LIGHT_UI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Directional_Light_UI
{
public:
    QLineEdit *lb_d_x;
    QLabel *label_10;
    QLabel *label_5;
    QLabel *label_2;
    QLineEdit *lb_d_z;
    QLineEdit *lb_d_y;
    QLabel *label_9;
    QLineEdit *lb_c_g;
    QPushButton *bt_color;
    QLineEdit *lb_c_r;
    QLabel *label_8;
    QLabel *label_11;
    QLabel *label_6;
    QLineEdit *lb_c_b;
    QLabel *label_12;
    QLabel *label_7;
    QLineEdit *lb_c_w;

    void setupUi(QGroupBox *Directional_Light_UI)
    {
        if (Directional_Light_UI->objectName().isEmpty())
            Directional_Light_UI->setObjectName(QString::fromUtf8("Directional_Light_UI"));
        Directional_Light_UI->resize(447, 105);
        lb_d_x = new QLineEdit(Directional_Light_UI);
        lb_d_x->setObjectName(QString::fromUtf8("lb_d_x"));
        lb_d_x->setGeometry(QRect(60, 30, 111, 21));
        label_10 = new QLabel(Directional_Light_UI);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(310, 30, 20, 20));
        label_5 = new QLabel(Directional_Light_UI);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(50, 30, 20, 20));
        label_2 = new QLabel(Directional_Light_UI);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 30, 31, 17));
        lb_d_z = new QLineEdit(Directional_Light_UI);
        lb_d_z->setObjectName(QString::fromUtf8("lb_d_z"));
        lb_d_z->setGeometry(QRect(320, 30, 111, 21));
        lb_d_y = new QLineEdit(Directional_Light_UI);
        lb_d_y->setObjectName(QString::fromUtf8("lb_d_y"));
        lb_d_y->setGeometry(QRect(190, 30, 111, 21));
        label_9 = new QLabel(Directional_Light_UI);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(180, 30, 20, 20));
        lb_c_g = new QLineEdit(Directional_Light_UI);
        lb_c_g->setObjectName(QString::fromUtf8("lb_c_g"));
        lb_c_g->setGeometry(QRect(140, 60, 51, 21));
        bt_color = new QPushButton(Directional_Light_UI);
        bt_color->setObjectName(QString::fromUtf8("bt_color"));
        bt_color->setGeometry(QRect(380, 60, 51, 25));
        lb_c_r = new QLineEdit(Directional_Light_UI);
        lb_c_r->setObjectName(QString::fromUtf8("lb_c_r"));
        lb_c_r->setGeometry(QRect(60, 60, 51, 21));
        label_8 = new QLabel(Directional_Light_UI);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(120, 60, 20, 20));
        label_11 = new QLabel(Directional_Light_UI);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(200, 60, 20, 20));
        label_6 = new QLabel(Directional_Light_UI);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 60, 41, 17));
        lb_c_b = new QLineEdit(Directional_Light_UI);
        lb_c_b->setObjectName(QString::fromUtf8("lb_c_b"));
        lb_c_b->setGeometry(QRect(220, 60, 51, 21));
        label_12 = new QLabel(Directional_Light_UI);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(280, 60, 20, 20));
        label_7 = new QLabel(Directional_Light_UI);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(50, 60, 20, 20));
        lb_c_w = new QLineEdit(Directional_Light_UI);
        lb_c_w->setObjectName(QString::fromUtf8("lb_c_w"));
        lb_c_w->setGeometry(QRect(300, 60, 51, 21));

        retranslateUi(Directional_Light_UI);

        QMetaObject::connectSlotsByName(Directional_Light_UI);
    } // setupUi

    void retranslateUi(QGroupBox *Directional_Light_UI)
    {
        Directional_Light_UI->setWindowTitle(QApplication::translate("Directional_Light_UI", "GroupBox", nullptr));
        Directional_Light_UI->setTitle(QApplication::translate("Directional_Light_UI", "DirectionalLight", nullptr));
        label_10->setText(QApplication::translate("Directional_Light_UI", "Z", nullptr));
        label_5->setText(QApplication::translate("Directional_Light_UI", "X", nullptr));
        label_2->setText(QApplication::translate("Directional_Light_UI", "\346\226\271\345\220\221", nullptr));
        label_9->setText(QApplication::translate("Directional_Light_UI", "Y", nullptr));
        bt_color->setText(QApplication::translate("Directional_Light_UI", "\351\200\211\346\213\251", nullptr));
        label_8->setText(QApplication::translate("Directional_Light_UI", "G", nullptr));
        label_11->setText(QApplication::translate("Directional_Light_UI", "B", nullptr));
        label_6->setText(QApplication::translate("Directional_Light_UI", "\351\242\234\350\211\262", nullptr));
        label_12->setText(QApplication::translate("Directional_Light_UI", "W", nullptr));
        label_7->setText(QApplication::translate("Directional_Light_UI", "R", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Directional_Light_UI: public Ui_Directional_Light_UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DIRECTIONAL_LIGHT_UI_H
