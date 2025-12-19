#include "browser/Requester.hpp"
#include "ui/MainWindow.hpp"
#include <QSslSocket>
#include <QUrl>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qlogging.h>
#include "PlasmaPercent.hpp"
#include <QStandardPaths>
#include <qobject.h>
#include <qsslcertificate.h>
#include <qsslkey.h>
#include <qsslsocket.h>
#include <QDir>
#include <qstringview.h>
#include <QSslKey>
#include <QSslConfiguration>

namespace Gemspace {
    Requester::Requester(MainWindow *parent) : QObject(parent), parentWindow(parent) {
        /* Requester - It does all dirty work: Gets site content, handles errors, and emits signals. Also there is a home page
         * that displays every browser startup.
         */

         if (!certsPath.exists()) {
             qDebug() << "Creating certs dir: " << certsPath.absolutePath();
             certsPath.mkpath(".");
         }
    }

    void Requester::sendRequest(const std::string &rawUrl) {
        if (rawUrl.empty()) return;

        if (rawUrl == "gemspace://home") {
                QByteArray mockData =
                    "20 text/gemini\r\n\n```"
"\n:###:                 ###:                            \n"
".#: .#               #   .#                            \n"
":#:      ###   ## #   #      # ##   .###.    ##:   ###  \n"
"#         :#  #:#:#  # .    #   #  #: :#   #        :# \n"
"#   ## #   #  # # #    ##   #   #      #  #.     #   # \n"
"#    # #####  # # #       # #   #  :####  #      ##### \n"
"#. .# #      # # #       # #   #  #:  #  #.     #      \n"
":###:     #  # # #  #.   # #   #  #.  #   #         # \n"
":###:  ###:  # # #  :####. # ##   :##:#    ##:   ###: \n"
"                           #                          \n"
"                           #                          \n"
"                           #                          \n"
                    "```\n## Useful pages\n"
                    "=> gemini://geminiprotocol.net Gemini protocol homepage\n"
                    "=> gemini://gemini.lehmann.cx/ Gemini protocol useful stuff\n"
                    "=> gemini://kennedy.gemi.dev/ Kennedy Search Engine\n"
                    "=> gemini://kennedy.gemi.dev/docs/search.gmi Kennedy Search Engine Documentation\n"
                    "=> gemini://gemi.dev/cgi-bin/wp.cgi/view?Wiki Gemipedia\n"
                    "## Time killing\n"
                    "=> gemini://xandra.cities.yesterweb.org/ Alexandra's cafe\n"
                    "=> gemini://cities.yesterweb.org/ Yesterweb\n"
                    "=> gemini://gemi.dev/cgi-bin/waffle.cgi/ Waffle\n"
                    "## Credits\n"
                    "```\n"
                    "This project is being developed by Monsler\non GitHub: https://github.com/Monsler/Gemspace!!!\n"
                    "```";
            emit requestFinished(mockData);
            return;
        }
        QByteArray *buffer = new QByteArray();

        parentWindow->plasmaPercent->emitPercent(0);

        QString urlStr = QString::fromStdString(rawUrl);
        if (!urlStr.contains("://") && urlStr != "gemspace://home") urlStr = "gemini://kennedy.gemi.dev/search?" + urlStr;
        QUrl url(urlStr);

        QSslSocket* socket = new QSslSocket(this);
        this->currentSocket = socket;
        setupKeyForSocket(socket, url);
        socket->setPeerVerifyMode(QSslSocket::VerifyNone);

        connect(socket, &QSslSocket::readyRead, [this, socket, buffer]() {
            buffer->append(socket->readAll());
            parentWindow->plasmaPercent->emitPercent(buffer->size() / 1024);
        });

        connect(socket, &QSslSocket::connected, [socket]() {

        });

        connect(socket, &QSslSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
            static_cast<QSslSocket*>(sender())->ignoreSslErrors();
            qDebug() << "SSL Errors: " << errors;
        });

        connect(socket, &QSslSocket::stateChanged, [this](QAbstractSocket::SocketState state) {
            //qDebug() << "Socket state changed to: " << state;
        });

        connect(socket, &QSslSocket::disconnected, socket, [this, socket, buffer]() {
            QByteArray response = socket->readAll();
            socket->deleteLater();
            parentWindow->plasmaPercent->emitPercent(100);
            emit requestFinished(*buffer);
            delete buffer;
        });

        connect(socket, &QSslSocket::encrypted, [socket, url]() {
            socket->write(url.toString().toUtf8() + "\r\n");
        });

        connect(socket, &QSslSocket::errorOccurred, [url](QAbstractSocket::SocketError error) {

        });

        connect(socket, &QSslSocket::encrypted, [this]() {
            parentWindow->plasmaPercent->emitPercent(50);
        });

        parentWindow->plasmaPercent->emitPercent(10);
        socket->connectToHostEncrypted(url.host(), 1965);
    }

    void Requester::setupKeyForSocket(QSslSocket *socket, const QUrl& url) {
        QFile cert = QFile(certsPath.filePath(url.host() + ".crt"));
        if (cert.exists()) {
            if (cert.open(QIODevice::ReadOnly)) {
                QByteArray certData = cert.readAll();

                QSslCertificate cert(certData);
                QSslKey key = QSslKey(certData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
                if (!cert.isNull() && !key.isNull()) {
                    QSslConfiguration config = socket->sslConfiguration();
                    config.setLocalCertificate(cert);
                    config.setPrivateKey(key);
                    socket->setSslConfiguration(config);
                    qDebug() << "Hooked up certificate for" << url.host();
                }
            }
        }
    }

    void Requester::registerCert(const QString &siteHostName) {
        QString certPath = certsPath.filePath(siteHostName + ".crt");
        qDebug() << "Will write certificate file to" << certPath;

        QFile certFile(certPath);
        if (!certFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open certificate file for writing";
            return;
        }

        EVP_PKEY* pkey = EVP_PKEY_new();
        RSA* rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
        EVP_PKEY_assign(pkey, EVP_PKEY_RSA, rsa);

        X509* cert = X509_new();
        X509_set_version(cert, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);
        X509_gmtime_adj(X509_get_notBefore(cert), 0);
        X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);
        X509_set_pubkey(cert, pkey);
        X509_sign(cert, pkey, EVP_sha256());

        if (!X509_sign(cert, pkey, EVP_sha256())) return;

        BIO *certBio = BIO_new(BIO_s_mem());
        PEM_write_bio_X509(certBio, cert);
        PEM_write_bio_PrivateKey(certBio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
        char *keyData;
        long keyLen = BIO_get_mem_data(certBio, &keyData);
        QByteArray keyBytes(keyData, keyLen);
        certFile.write(keyBytes);
        certFile.close();
        qDebug() << "Certificate file written successfully";
        BIO_free(certBio);
    }
}
