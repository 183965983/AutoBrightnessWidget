#include "UI/mainwindow.h"

#include <QApplication>
// Translation feature disabled to avoid deployment issues
// #include <QLocale>
// #include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Translation feature disabled to avoid deployment issues
    // QTranslator translator;
    // const QStringList uiLanguages = QLocale::system().uiLanguages();
    // for (const QString &locale : uiLanguages) {
    //     const QString baseName = "AutoBrightnessWidget_" + QLocale(locale).name();
    //     if (translator.load(":/i18n/" + baseName)) {
    //         a.installTranslator(&translator);
    //         break;
    //     }
    // }
    MainWindow w;
    w.show();
    return a.exec();
}
