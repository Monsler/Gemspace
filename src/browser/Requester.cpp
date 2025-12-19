#include "browser/Requester.hpp"
#include <QSslSocket>
#include <QUrl>
#include <qlogging.h>

namespace Gemspace {
    Requester::Requester(QObject *parent) : QObject(parent) {

    }

    void Requester::sendRequest(const std::string &rawUrl) {
        if (rawUrl.empty()) return;

        if (rawUrl == "home") {
                QByteArray mockData =
                    "20 text/gemini\r\n"
                    "# Welcome to Gemspace\n\n"
                    "## Useful pages\n"
                    "=> gemini://geminiprotocol.net Gemini protocol homepage\n"
                    "=> gemini://gemini.lehmann.cx/ Gemini protocol useful stuff\n"
                    "=> gemini://kennedy.gemi.dev/ Kennedy Search Engine\n"
                    "=> gemini://kennedy.gemi.dev/docs/search.gmi Kennedy Search Engine Documentation\n"
                    "## Fun sites\n"
                    "=> gemini://xandra.cities.yesterweb.org/ Strange cafe\n"
                    "## Credits\n\n"
                    "```\n"
                    "This browser is being developed by Monsler on GitHub: https://github.com/Monsler/Gemspace!!!\n"
                    "```";
            emit requestFinished(mockData);
            return;
        }

        qDebug() << "Supports SSL:" << QSslSocket::supportsSsl();
        qDebug() << "SSL Library Build Version:" << QSslSocket::sslLibraryBuildVersionString();
        qDebug() << "SSL Library Runtime Version:" << QSslSocket::sslLibraryVersionString();

        QString urlStr = QString::fromStdString(rawUrl);
        if (!urlStr.contains("://")) urlStr = "gemini://" + urlStr;
        QUrl url(urlStr);

        QSslSocket* socket = new QSslSocket(this);
        socket->setPeerVerifyMode(QSslSocket::VerifyNone);

        connect(socket, &QSslSocket::readyRead, [this, socket]() {
            QByteArray response = socket->readAll();
            qDebug() << "Request finished with data:\n" << response;
            emit requestFinished(response);
        });

        connect(socket, &QSslSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
            static_cast<QSslSocket*>(sender())->ignoreSslErrors();
        });
        connect(socket, &QSslSocket::disconnected, socket, &QObject::deleteLater);

        connect(socket, &QSslSocket::encrypted, [socket, url]() {
            socket->write(url.toString().toUtf8() + "\r\n");
        });

        connect(socket, &QSslSocket::errorOccurred, [url](QAbstractSocket::SocketError error) {
            qDebug() << "Socket Error for" << url.host() << ":" << error;
        });

        connect(socket, &QSslSocket::encrypted, []() {
            qDebug() << "TLS Encryption established!";
        });

        socket->connectToHostEncrypted(url.host(), 1965);
        qDebug() << "Connecting to " << url.host() << ":" << 1965;
    }
}
