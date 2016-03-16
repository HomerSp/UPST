#ifndef WEBUTILS_H
#define WEBUTILS_H

#include <QEventLoop>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>

namespace Web {
    class WebLoop : public QEventLoop {
        Q_OBJECT
    public:
        WebLoop(QObject* parent = 0)
            : QEventLoop(parent),
            mReply(nullptr)
        {

        }

        QNetworkReply* reply() const {
            return mReply;
        }

    public slots:
        void replyFinished(QNetworkReply* reply) {
            mReply = reply;
            QEventLoop::quit();
        }

    private:
        QNetworkReply* mReply;
    };

    class WebUtils
    {
    public:
        static bool download(const QUrl& url, QByteArray& output);

    private:
        WebUtils();
    };
}

#endif // WEBUTILS_H
