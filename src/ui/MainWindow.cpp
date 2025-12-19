#include "ui/MainWindow.hpp"
#include <QMenuBar>
#include <qframe.h>
#include <qlogging.h>
#include "browser/Requester.hpp"
#include <QInputDialog>
#include <QTimer>
#include <qnamespace.h>
#include <qobjectdefs.h>

namespace Gemspace {
    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent) {
        setWindowTitle("Gemspace");
        resize(700, 500);

        requester = new Requester(this);

        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        menuBar = new QMenuBar(this);
        setMenuBar(menuBar);

        fileMenu = new QMenu(tr("&File"), menuBar);
        menuBar->addMenu(fileMenu);

        layout = new QVBoxLayout(centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        centralWidget->setLayout(layout);

        searchPanel = new QHBoxLayout(centralWidget);
        searchPanel->setContentsMargins(9, 9, 9, 3);
        layout->addLayout(searchPanel);

        searchInput = new QLineEdit(centralWidget);
        searchInput->setPlaceholderText(tr("&SearchCaptionPlaceholder"));

        backButton = new QPushButton(centralWidget);
        backButton->setIcon(QIcon::fromTheme("go-previous"));

        forwardButton = new QPushButton(centralWidget);
        forwardButton->setIcon(QIcon::fromTheme("go-next"));

        menuButton = new QPushButton(centralWidget);
        menuButton->setIcon(QIcon::fromTheme("application-menu"));

        refreshButton = new QPushButton(centralWidget);
        refreshButton->setIcon(QIcon::fromTheme("system-reboot"));

        searchPanel->addWidget(backButton);
        searchPanel->addWidget(forwardButton);
        searchPanel->addWidget(refreshButton);
        searchPanel->addWidget(searchInput);
        searchPanel->addWidget(menuButton);

        gembling = new Gembling(this);


        scrollArea = new QScrollArea(centralWidget);
        scrollArea->setWidget(gembling);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::StyledPanel);
        scrollArea->setFrameShadow(QFrame::Sunken);
        layout->addWidget(scrollArea);

        connect(searchInput, &QLineEdit::returnPressed, this, &MainWindow::onUrlEntered);
        connect(requester, &Requester::requestFinished, this, &MainWindow::onConnected);
        connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
        connect(backButton, &QPushButton::clicked, this, &MainWindow::onBackClicked);
        connect(forwardButton, &QPushButton::clicked, this, &MainWindow::onForwardClicked);

        searchInput->setText("home");

        show();
        onUrlEntered();
    }
    void MainWindow::onBackClicked() {
        if (!backwardStack.empty()) {

            QString url = backwardStack.back();
            backwardStack.pop_back();

            addToForwardStack(this->searchInput->text());

            isNavigating = true;
            searchInput->setText(url);
            updateNavigationButtons();
            onUrlEntered();
        }
    }

    void MainWindow::onForwardClicked() {
        if (!forwardStack.empty()) {
            QString url = forwardStack.back();
            forwardStack.pop_back();
            addToBackwardStack(this->searchInput->text());
            isNavigating = true;
            searchInput->setText(url);
            updateNavigationButtons();
            onUrlEntered();
        }
    }

    void MainWindow::onRefreshClicked() {
        onUrlEntered();
    }

    void MainWindow::addToBackwardStack(const QString& url) {
        backwardStack.push_back(url);
    }

    void MainWindow::addToForwardStack(const QString& url) {
        forwardStack.push_back(url);
    }

    void MainWindow::updateNavigationButtons() {
        backButton->setEnabled(!backwardStack.empty());
        forwardButton->setEnabled(!forwardStack.empty());
    }

    void MainWindow::onConnected(const QByteArray& data) {
        updateNavigationButtons();
        QString content = QString::fromUtf8(data);
        QString contentSample = QString::fromUtf8(data.left(100));
        qDebug() << "Received data: " << content;

        QUrl currentUrl(searchInput->text());

        if (content.startsWith("20")) {
            if (contentSample.contains("image/")) {
               //qDebug() << "Received image data";
                int headerEnd = content.indexOf("\r\n");
                if (headerEnd != -1) {
                    QByteArray imageData = data.mid(headerEnd + 2);

                    if (!imageData.isEmpty()) {
                        gembling->drawImage(imageData);
                    }
                }
                return;
            }

            //qDebug() << "Received data: " << content;
            refreshButton->setEnabled(true);
            int firstLineEnd = content.indexOf("\n");
            isNavigating = false;
            if (firstLineEnd != -1 && content.length() > firstLineEnd + 1) {
                gembling->parseSite(content);
            } else {
                qDebug() << "Header received, waiting for body...";
            }
        } else if (content.startsWith("30") || content.startsWith("31")) {
            QString nextUrlStr = content.mid(2).trimmed();
            isNavigating = true;

            QUrl baseUrl(searchInput->text());
            QUrl redirectUrl(nextUrlStr);

            QUrl finalUrl = baseUrl.resolved(redirectUrl);

            if (finalUrl.scheme().isEmpty()) finalUrl.setScheme("gemini");

            searchInput->clearFocus();
            searchInput->setText(finalUrl.toString());
            onUrlEntered();
        } else if (content.startsWith("10") || content.startsWith("11")) {
            QString currentUrl = this->searchInput->text();
            QString inputText = content.mid(3).trimmed();

            QTimer::singleShot(0, this, [this, inputText]() {
                    bool ok;
                    QString userInput = QInputDialog::getText(this, "Gemspace", inputText, QLineEdit::Normal, "", &ok);

                    if (ok && !userInput.isEmpty()) {
                        QUrl url(this->searchInput->text());
                        url.setQuery(QUrl::toPercentEncoding(userInput));

                        this->searchInput->setText(url.toString());
                        this->onUrlEntered();
                    } else {
                        this->onBackClicked();
                    }
                });
        } else {
            gembling->parseSite(content);
        }
    }

    void MainWindow::onUrlEntered() {
        refreshButton->setEnabled(false);
        QString newUrl = searchInput->text();
        QString oldUrl = QString::fromStdString(this->currentUrl);
        if (!isNavigating && !oldUrl.isEmpty() && newUrl != oldUrl && std::find(backwardStack.begin(), backwardStack.end(), oldUrl) == backwardStack.end()) {
            addToBackwardStack(oldUrl);
            forwardStack.clear();
        }

        currentUrl = this->searchInput->text().toStdString();
        gembling->clearObjects();
        const std::string url = searchInput->text().toStdString();

        requester->sendRequest(url);
    }

    MainWindow::~MainWindow()
    {

    }
}
