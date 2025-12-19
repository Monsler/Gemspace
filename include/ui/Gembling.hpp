#pragma once

#include <QWidget>
#include <qframe.h>
#include <qmainwindow.h>
#include <vector>
#include <QVBoxLayout>

namespace Gemspace {
    class MainWindow;

    class Gembling : public QFrame {
    public:
        Gembling(MainWindow* parent = nullptr);
        ~Gembling();
        void parseSite(const QString& site);
        void clearObjects();
        void drawImage(const QByteArray& imageData);

        std::string currentUrl;

    private:

        std::vector<QObject*> objects;
        QVBoxLayout* layout;

        MainWindow* mainWindow;
    };
}
