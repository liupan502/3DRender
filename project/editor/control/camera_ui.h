/********************************************************************************
** Form generated from reading UI file 'camera.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CAMERA_UI_H
#define CAMERA_UI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_Camera_UI
{
public:
    QLabel *label_4;
    QLabel *label_2;
    QLineEdit *lb_r_x;
    QLabel *label_5;
    QLabel *label_11;
    QLabel *label_12;
    QLabel *label_9;
    QLabel *label_10;
    QLineEdit *lb_p_z;
    QLineEdit *lb_p_x;
    QLineEdit *lb_s_y;
    QLabel *label_3;
    QLabel *label_16;
    QLineEdit *lb_s_z;
    QLabel *label_15;
    QLineEdit *lb_r_y;
    QLineEdit *lb_r_z;
    QLabel *label_13;
    QLineEdit *lb_p_y;
    QLineEdit *lb_s_x;
    QLabel *label_14;

    void setupUi(QGroupBox *Camera_UI)
    {
        if (Camera_UI->objectName().isEmpty())
            Camera_UI->setObjectName(QString::fromUtf8("Camera_UI"));
        Camera_UI->resize(444, 121);
        label_4 = new QLabel(Camera_UI);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 90, 31, 17));
        label_2 = new QLabel(Camera_UI);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 30, 31, 17));
        lb_r_x = new QLineEdit(Camera_UI);
        lb_r_x->setObjectName(QString::fromUtf8("lb_r_x"));
        lb_r_x->setGeometry(QRect(60, 60, 111, 21));
        label_5 = new QLabel(Camera_UI);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(50, 30, 20, 20));
        label_11 = new QLabel(Camera_UI);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(50, 60, 20, 20));
        label_12 = new QLabel(Camera_UI);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(310, 60, 20, 20));
        label_9 = new QLabel(Camera_UI);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(180, 30, 20, 20));
        label_10 = new QLabel(Camera_UI);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(310, 30, 20, 20));
        lb_p_z = new QLineEdit(Camera_UI);
        lb_p_z->setObjectName(QString::fromUtf8("lb_p_z"));
        lb_p_z->setGeometry(QRect(320, 30, 111, 21));
        lb_p_x = new QLineEdit(Camera_UI);
        lb_p_x->setObjectName(QString::fromUtf8("lb_p_x"));
        lb_p_x->setGeometry(QRect(60, 30, 111, 21));
        lb_s_y = new QLineEdit(Camera_UI);
        lb_s_y->setObjectName(QString::fromUtf8("lb_s_y"));
        lb_s_y->setGeometry(QRect(190, 90, 111, 21));
        label_3 = new QLabel(Camera_UI);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 60, 31, 17));
        label_16 = new QLabel(Camera_UI);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(180, 90, 20, 20));
        lb_s_z = new QLineEdit(Camera_UI);
        lb_s_z->setObjectName(QString::fromUtf8("lb_s_z"));
        lb_s_z->setGeometry(QRect(320, 90, 111, 21));
        label_15 = new QLabel(Camera_UI);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(310, 90, 20, 20));
        lb_r_y = new QLineEdit(Camera_UI);
        lb_r_y->setObjectName(QString::fromUtf8("lb_r_y"));
        lb_r_y->setGeometry(QRect(190, 60, 111, 21));
        lb_r_z = new QLineEdit(Camera_UI);
        lb_r_z->setObjectName(QString::fromUtf8("lb_r_z"));
        lb_r_z->setGeometry(QRect(320, 60, 111, 21));
        label_13 = new QLabel(Camera_UI);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(180, 60, 20, 20));
        lb_p_y = new QLineEdit(Camera_UI);
        lb_p_y->setObjectName(QString::fromUtf8("lb_p_y"));
        lb_p_y->setGeometry(QRect(190, 30, 111, 21));
        lb_s_x = new QLineEdit(Camera_UI);
        lb_s_x->setObjectName(QString::fromUtf8("lb_s_x"));
        lb_s_x->setGeometry(QRect(60, 90, 111, 21));
        label_14 = new QLabel(Camera_UI);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(50, 90, 20, 20));

        retranslateUi(Camera_UI);

        QMetaObject::connectSlotsByName(Camera_UI);
    } // setupUi

    void retranslateUi(QGroupBox *Camera_UI)
    {
        Camera_UI->setWindowTitle(QApplication::translate("Camera_UI", "GroupBox", nullptr));
        Camera_UI->setTitle(QApplication::translate("Camera_UI", "Camera", nullptr));
        label_4->setText(QApplication::translate("Camera_UI", "\344\270\212\350\275\264", nullptr));
        label_2->setText(QApplication::translate("Camera_UI", "\344\275\215\347\275\256", nullptr));
        label_5->setText(QApplication::translate("Camera_UI", "X", nullptr));
        label_11->setText(QApplication::translate("Camera_UI", "X", nullptr));
        label_12->setText(QApplication::translate("Camera_UI", "Z", nullptr));
        label_9->setText(QApplication::translate("Camera_UI", "Y", nullptr));
        label_10->setText(QApplication::translate("Camera_UI", "Z", nullptr));
        label_3->setText(QApplication::translate("Camera_UI", "\347\233\256\346\240\207", nullptr));
        label_16->setText(QApplication::translate("Camera_UI", "Y", nullptr));
        label_15->setText(QApplication::translate("Camera_UI", "Z", nullptr));
        label_13->setText(QApplication::translate("Camera_UI", "Y", nullptr));
        label_14->setText(QApplication::translate("Camera_UI", "X", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Camera_UI: public Ui_Camera_UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // CAMERA_UI_H
