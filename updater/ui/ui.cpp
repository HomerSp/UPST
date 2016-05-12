#include <QDebug>
#include <QQmlContext>
#include <QQmlProperty>
#include <QSettings>
#include <QThread>

#include "utils/fileutils.h"
#include "ui.h"

UI::MainUI::MainUI(const QGuiApplication& app, const QString& updateID, const QString& installDir)
    : QObject(),
      mApp(app)
{
    mEngine = new QQmlApplicationEngine();
    mEngine->rootContext()->setContextProperty("programVersion", QString(PROG_VERSION));
    mEngine->rootContext()->setContextProperty("qtVersion", QString(QT_VERSION_STR));

    QObject::connect(mEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    mEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

    QThread* thread = new QThread;

    mWorker = new UpdateWorker(updateID, installDir);
    mWorker->moveToThread(thread);

    connect(mWorker, &UpdateWorker::downloadProgress, this, &MainUI::downloadProgress);
    connect(mWorker, &UpdateWorker::installProgress, this, &MainUI::installProgress);
    connect(mWorker, &UpdateWorker::installFinished, this, &MainUI::installFinished);
    connect(mWorker, &UpdateWorker::updateStatus, this, &MainUI::updateStatus);

    connect(thread, &QThread::started, mWorker, &UpdateWorker::process);
    connect(mWorker, &UpdateWorker::finished, thread, &QThread::quit);
    connect(mWorker, &UpdateWorker::finished, mWorker, &QThread::deleteLater);
    connect(thread, &QThread::finished, mWorker, &UpdateWorker::deleteLater);

    thread->start();
}

UI::MainUI::~MainUI() {
    if(mWorker != nullptr) {
        delete mWorker;
    }

    delete mEngine;
}

void UI::MainUI::downloadProgress(qint64 received, qint64 total) {
    if(total == 0) {
        return;
    }

    QObject* rootObject = mEngine->rootObjects().first();

    int p = (received / (float)total) * 50.0f;

    QObject* progressObject = rootObject->findChild<QObject*>("updatingProgress");
    progressObject->setProperty("value", p);
}

void UI::MainUI::installProgress(quint64 received, quint64 total) {
    if(total == 0) {
        return;
    }

    QObject* rootObject = mEngine->rootObjects().first();

    int p = (received / (float)total) * 50.0f;

    QObject* progressObject = rootObject->findChild<QObject*>("updatingProgress");
    progressObject->setProperty("value", 50 + p);
}

void UI::MainUI::installFinished(const QString& installDir) {
#ifdef Q_OS_WIN
    QString upstPath(installDir + "/UPST.exe");
#else
    QString upstPath(installDir + "/UPST");
#endif

    QStringList args;
    args << "update" << QCoreApplication::applicationDirPath();

    Utils::FileUtils::execute(upstPath, args, installDir);

    mApp.quit();
}

void UI::MainUI::updateStatus(const QString &status) {
    QObject* rootObject = mEngine->rootObjects().first();
    rootObject->setProperty("processStatus", status);
}
