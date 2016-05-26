#ifndef UI_H
#define UI_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "updateworker.h"

namespace UI {
    class UISection;

    class MainUI : public QObject
    {
        Q_OBJECT
    public:
        MainUI(const QGuiApplication& app, const QString& updateID, uint64_t updateTime, const QString& installDir);
        ~MainUI();

        QQmlApplicationEngine* engine() {
            return mEngine;
        }

    public slots:
        void downloadProgress(qint64 received, qint64 total);
        void installProgress(quint64 received, quint64 total);
        void installFinished(const QString& installDir);

        void updateStatus(const QString &status);

    private:
        const QGuiApplication& mApp;

        QQmlApplicationEngine *mEngine;

        UpdateWorker* mWorker;

        uint64_t mUpdateTime;

    };
}

#endif // UI_H
