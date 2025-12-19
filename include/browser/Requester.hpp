#pragma once
#include <QSslSocket>
#include <qstringview.h>
#include <QStandardPaths>
#include <QDir>

namespace Gemspace {
    class MainWindow;

    class Requester : public QObject {
        Q_OBJECT

        public:
        Requester(MainWindow *parent = nullptr);
        void sendRequest(const std::string &url);
        QSslSocket *currentSocket;
        MainWindow *parentWindow;
        const QDir certsPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("certs");
        void registerCert(const QString &siteHostName);

        signals:
            void requestFinished(const QByteArray& data);

        private:
            void setupKeyForSocket(QSslSocket *socket, const QUrl& url);
    };
}
