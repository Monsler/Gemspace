#pragma once
#include <qobject.h>
#include <qtmetamacros.h>

namespace Gemspace
{
    class PlasmaPercent : public QObject {
        Q_OBJECT

        public:
        void emitPercent(const int percent);

        private:
        int progressPercent;
    };
} // namespace Kalorite
