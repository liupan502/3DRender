
#include "editor_windows.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    auto &inst = editor::EditorWindows::getInstance();
    inst.init();
    inst.bing_treeView();
    auto loop= inst.Show();
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&](){loop();});
    timer.start(16); // 每16毫秒更新一次

    return app.exec();
}
