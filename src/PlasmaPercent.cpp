#include "PlasmaPercent.hpp"

#if defined(Q_OS_LINUX)
#include <qcontainerfwd.h>
#include <qdbusmessage.h>
#include <qdbusconnection.h>
#endif

namespace Gemspace
{
    void PlasmaPercent::emitPercent(const int percent) {
#if defined(Q_OS_LINUX)
        double progressValue = percent / 100.0;

        QVariantMap properties;
        properties.insert("progress", progressValue);
        properties.insert("progress-visible", progressValue > 0.0 && progressValue < 1.0);

        QDBusMessage message = QDBusMessage::createSignal(
            "/io/github/monsler/Gemspace",
            "com.canonical.Unity.LauncherEntry",
            "Update"
        );

        message << QString("application://io.github.monsler.Gemspace.desktop");
        message << properties;

        QDBusConnection::sessionBus().send(message);
#endif
    }

    void PlasmaPercent::incrementPercent(const int percent) {
        progressPercent += float(percent) / 100.0;
        emitPercent(progressPercent);
    }
} // namespace Gemspace
