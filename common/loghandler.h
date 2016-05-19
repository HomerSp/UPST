#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>
#include <QFile>

namespace Log {
    class LogHandlerWorker : public QObject {
        Q_OBJECT
    public:
        LogHandlerWorker(const QString& name);
        ~LogHandlerWorker();

    signals:
        void finished();

    public slots:
        void handleLog(const QString& line);

    private:
        QFile* mFile;
    };

    class LogHandler : public QObject {
        Q_OBJECT
    public:
        LogHandler(const QString& name, QObject* parent = 0);
        ~LogHandler();

        void addLog(QtMsgType type, const QMessageLogContext& context, QString msg);

        Q_INVOKABLE QString getLogData() const {
            return mLogData;
        }

        int length() const {
            return mLogData.length();
        }

    private slots:
        void handleLog(const QString &line);

    signals:
        void log(const QString& line);

        void logDataChanged();

    private:
        QString formatLog(QtMsgType type, const QMessageLogContext& context, QString msg);

        QThread* mWorkerThread;
        LogHandlerWorker* mWorker;

        QString mLogData;
    };
}

#endif // LOGHANDLER_H
