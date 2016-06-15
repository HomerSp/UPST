#ifndef USERGUIDEHELPER_H
#define USERGUIDEHELPER_H

#include <QObject>

namespace UI {
namespace Guide {
    class UserGuideHelper : public QObject
    {
        Q_OBJECT
    public:
        UserGuideHelper(QObject* parent = 0);

        Q_INVOKABLE QString getData();
    };
}
}

#endif // USERGUIDEHELPER_H
