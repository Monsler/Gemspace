#pragma once
#include <QSslSocket>
#include <qstringview.h>

namespace Gemspace {
    class Requester : public QObject {
        Q_OBJECT

        public:
        Requester(QObject *parent = nullptr);
        void sendRequest(const std::string &url);
        QSslSocket *currentSocket;

        signals:
            void requestFinished(const QByteArray& data);

        private:

    };
}
