/********************************************************************************
** Form generated from reading UI file 'animation_manager.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeView>

QT_BEGIN_NAMESPACE

class Ui_AnimationManager_UI
{
public:
    QTreeView *tvAnimation;

    void setupUi(QGroupBox *AnimationManager_UI)
    {
        if (AnimationManager_UI->objectName().isEmpty())
            AnimationManager_UI->setObjectName(QString::fromUtf8("AnimationManager_UI"));
        AnimationManager_UI->resize(442, 160);
        tvAnimation = new QTreeView(AnimationManager_UI);
        tvAnimation->setObjectName(QString::fromUtf8("tvAnimation"));
        tvAnimation->setGeometry(QRect(0, 22, 441, 131));

        retranslateUi(AnimationManager_UI);

        QMetaObject::connectSlotsByName(AnimationManager_UI);
    } // setupUi

    void retranslateUi(QGroupBox *AnimationManager_UI)
    {
        AnimationManager_UI->setWindowTitle(QApplication::translate("AnimationManager_UI", "GroupBox", nullptr));
        AnimationManager_UI->setTitle(QApplication::translate("AnimationManager_UI", "AnimationManager", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AnimationManager_UI: public Ui_AnimationManager_UI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // ANIMATION_MANAGER_H
