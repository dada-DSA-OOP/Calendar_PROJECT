#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile("resource/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    MainWindow w;
    w.setWindowTitle("dadaCalendar");
    w.setWindowIcon(QIcon("resource/icons/app_icon.png")); // Đặt icon cho cửa sổ
    w.show();
    return a.exec();
}
