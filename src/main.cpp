#include "ui/MainWindow.hpp"
#include <QApplication>
#include <QTranslator>
#include <qlogging.h>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setDesktopFileName("io.github.monsler.Gemspace");

    QTranslator translator;
    QLocale systemLocale = QLocale::system();

    if(translator.load(systemLocale, "gemspace", "_", ":/translations")) {
        app.installTranslator(&translator);

        qDebug() << "Setup translation for locale" << systemLocale.name();
    } else {
        auto _ = translator.load(":/translations/gemspace_en.qm");
    }

    Gemspace::MainWindow window;
    window.show();

    return app.exec();
}
