#pragma execution_character_set("utf-8")
#include "controlwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    app.setApplicationVersion("1.0");
    app.setApplicationName("泊车库位标注");

    //load qss file
    QFile file(":/style/style/style.qss");
    file.open(QFile::ReadOnly);
    QString qss = QLatin1String(file.readAll());
    qApp->setStyleSheet(qss);

    ControlWindow w;
    w.show();
    return app.exec();
}
