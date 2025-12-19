#pragma once
#include <QMainWindow>
#include <QVBoxLayout>
#include <qboxlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qstringview.h>
#include <qwidget.h>
#include <QLineEdit>
#include <QPushButton>
#include "browser/Requester.hpp"
#include "ui/Gembling.hpp"
#include <QScrollArea>
#include <vector>

namespace Gemspace {
    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();
        QLineEdit* searchInput;
        std::string currentUrl;
        void addToBackwardStack(const QString& url);
        void addToForwardStack(const QString& url);

    public slots:
        void onUrlEntered();
        void onConnected(const QByteArray& data);
        void onRefreshClicked();
        void onBackClicked();
        void onForwardClicked();
        void updateNavigationButtons();



    private:
        std::vector<QString> forwardStack;
        std::vector<QString> backwardStack;

        bool isNavigating;

        QVBoxLayout* layout;
        QWidget* centralWidget;
        QHBoxLayout* searchPanel;

        QPushButton* backButton;
        QPushButton* forwardButton;
        QPushButton* refreshButton;

        QPushButton* menuButton;
        QMenuBar* menuBar;
        QMenu* fileMenu;
        QScrollArea* scrollArea;
        Gembling* gembling;

        Requester* requester;
    };
}
