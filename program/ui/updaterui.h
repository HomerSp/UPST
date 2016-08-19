#ifndef UPDATERUI_H
#define UPDATERUI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QLocalServer>

namespace UI {
    class UISection;

    class UpdaterUI : public QObject
    {
        Q_OBJECT
    public:
        UpdaterUI(const QGuiApplication& app);
        ~UpdaterUI();

        QQmlApplicationEngine* engine() {
            return mEngine;
        }

    public slots:
        void updaterTimeout();

        void updaterConnection();
        void updaterDisconnected();
        void updaterDataReady();

    protected:
        void downloadProgress(quint64 received, quint64 total);
        void installProgress(quint64 received, quint64 total);
        void installFinished();

        void updateStatus(const QString &status);

    private:
        const QGuiApplication& mApp;

        QLocalServer* mStatusServer;
        bool mHaveClient;

        QQmlApplicationEngine *mEngine;

        QString mInstallDir;
        uint64_t mUpdateTime;

        QByteArray mBufferedData;
    };
}

#endif // UPDATERUI_H
