#ifndef UPDATEWORKER_H
#define UPDATEWORKER_H

#include <QObject>
#include <QLocalSocket>

#include "updater/updater.h"

namespace UI {
    class UpdateWorker: public QObject
    {
        Q_OBJECT
    public:
        UpdateWorker(const QString &id, uint64_t updateTime, const QString& installDir);
        ~UpdateWorker();

        void process();

    public slots:
        void downloadProgress(qint64 received, qint64 total);

    signals:
        void finished();

    protected:
        void updateStatus(Updater::UpdateStatus status, quint64 received = 0, quint64 total = 0);

    private:
        QLocalSocket* mSocket;

        QString mID;
        uint64_t mUpdateTime;
        QString mInstallDir;
    };
}

#endif // UPDATEWORKER_H
