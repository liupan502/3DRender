
#pragma once
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollArea>
#include "editor_windows.h"

class NodeTreeView : public QTreeView
{
    enum ValueType
    {
        Float,
        FloatSet
    };

public:
    NodeTreeView(QWidget *parent = nullptr) : QTreeView(parent) {}

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void show_input_window(QString name, QString title, QString &input);
    void show_create_geometry_node_window(editor::GeometryNodeInfo &info);

    void show_add_rect_window(editor::GeometryNodeInfo &info, std::shared_ptr<sg::GeometryNode> node);

    void show_add_line_path_window(editor::GeometryNodeInfo &info, std::shared_ptr<sg::GeometryNode> node);

    void move_node(std::shared_ptr<sg::Node> node);
};

class ComponentScrollArea : public QScrollArea
{
public:
    ComponentScrollArea(QWidget *parent = nullptr) : QScrollArea(parent) {}

protected:
    void mousePressEvent(QMouseEvent *event);
};
