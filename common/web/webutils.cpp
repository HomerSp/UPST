#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>

#include "webutils.h"

using namespace Web;

WebUtils::WebUtils()
{

}

bool WebUtils::download(const QUrl& url, QByteArray& output) {
    QHash<QString, QString> headers;
    return download(url, output, headers);
}

bool WebUtils::download(const QUrl& url, QByteArray& output, QString postData, WebDownloadStatus* status) {
    QHash<QString, QString> headers;
    return download(url, output, headers, postData, status);
}

bool WebUtils::download(const QUrl& url, QByteArray& output, const QHash<QString, QString> &headers, QString postData, WebDownloadStatus* status) {
    QNetworkRequest req(url);
    for(QHash<QString, QString>::const_iterator i = headers.begin(); i != headers.end(); ++i) {
        req.setRawHeader(i.key().toLatin1(), i.value().toLatin1());
    }

    QNetworkAccessManager manager;

    QNetworkReply* reply = nullptr;
    if(postData.size() > 0) {
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = manager.post(req, postData.toLatin1());
    } else {
        reply = manager.get(req);
    }

    if(reply == nullptr) {
        return false;
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if(status != nullptr) {
        QObject::connect(reply, &QNetworkReply::downloadProgress, status, &WebDownloadStatus::progress);
    }

    loop.exec();

    bool success = reply->error() == QNetworkReply::NoError;
    if(success) {
        output.clear();
        output = reply->readAll();
    }

    return success;
}
