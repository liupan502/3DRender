#include "toolwindow.h"
#include "ui_toolwindow.h"

ToolWindow::ToolWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ToolWindow)
{
    ui->setupUi(this);
}

ToolWindow::~ToolWindow()
{
    delete ui;
}
