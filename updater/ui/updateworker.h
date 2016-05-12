#ifndef UPDATEWORKER_H
#define UPDATEWORKER_H

#include <QObject>

namespace UI {
    class UpdateWorker: public QObject
    {
        Q_OBJECT
    public:
        UpdateWorker(const QString &id, const QString& installDir);

    public slots:
        void process();

    signals:
        void finished();

        void downloadProgress(qint64 received, qint64 total);
        void installProgress(quint64 received, quint64 total);
        void installFinished(const QString& installDir);

        void updateStatus(const QString &status);

    private:
        QString mID;
        QString mInstallDir;
    };
}

#endif // UPDATEWORKER_H
