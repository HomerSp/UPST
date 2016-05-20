#include <QStandardPaths>
#include <QTextStream>
#include <QThread>

#include "loghandler.h"

Log::LogHandlerWorker::LogHandlerWorker(const QString& name) {
    mFile = new QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + name);
    mFile->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
}

Log::LogHandlerWorker::~LogHandlerWorker() {
    mFile->flush();
    mFile->close();
    delete mFile;
}

void Log::LogHandlerWorker::handleLog(const QString& line) {
    // Skip debug messages when not in testing mode
#ifndef TESTING_MODE
    if(line.startsWith("[Debug]")) {
        return;
    }
#endif

    QTextStream(stdout) << line;
    if(mFile->isOpen()) {
        QTextStream stream(mFile);
        stream << line;
        stream.flush();
    }
}


Log::LogHandler::LogHandler(const QString& name, QObject* parent)
    : QObject(parent),
      mLogData("")
{
    mWorkerThread = new QThread;

    mWorker = new LogHandlerWorker(name);
    mWorker->moveToThread(mWorkerThread);

    QObject::connect(this, &LogHandler::log, mWorker, &LogHandlerWorker::handleLog);

    mWorkerThread->start();

    QObject::connect(this, &LogHandler::log, this, &LogHandler::handleLog);
}

Log::LogHandler::~LogHandler() {
    delete mWorker;
    delete mWorkerThread;
}

void Log::LogHandler::addLog(QtMsgType type, const QMessageLogContext &context, QString msg) {
    QString line = formatLog(type, context, msg);
    if(line.length() == 0) {
        return;
    }

    emit log(line);
}

void Log::LogHandler::handleLog(const QString& data) {
    mLogData += data;
    if(LOG_LIMIT > 0 && mLogData.length() > LOG_LIMIT) {
        mLogData.remove(0, mLogData.length() - LOG_LIMIT);
    }

    emit logDataChanged();
}

QString Log::LogHandler::formatLog(QtMsgType type, const QMessageLogContext& context, QString msg) {
#ifndef QT_DEBUG
    Q_UNUSED(context);
#endif
    QString data = "";
    switch(type) {
    case QtDebugMsg:
        data += "[Debug]";
        break;
    case QtInfoMsg:
        data += "[Info]";
        break;
    case QtWarningMsg:
        data += "[Warning]";
        break;
    case QtCriticalMsg:
        data += "[Critical]";
        break;
    case QtFatalMsg:
        data += "[Fatal]";
        break;
    default:
        data += "[Error]";
        break;
    }

#ifdef QT_DEBUG
    data += " " + context.file + "." + QString::number(context.line) + ":";
#endif
    data += " " + msg;

    if(data.length() > 0) {
        if(data.at(data.length() - 1) != '\n') {
            data += "\n";
        }
    }

    return data;
}
