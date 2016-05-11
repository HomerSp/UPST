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

bool WebUtils::download(const QUrl& url, QByteArray& output, const QHash<QString, QString> &headers, QString postData, WebDownloadStatus* status) {
    WebLoop loop;

    QNetworkAccessManager *manager = new QNetworkAccessManager(&loop);
    QObject::connect(manager, &QNetworkAccessManager::finished, &loop, &WebLoop::replyFinished);

    QNetworkRequest req(url);
    for(QHash<QString, QString>::const_iterator i = headers.begin(); i != headers.end(); ++i) {
        req.setRawHeader(i.key().toLatin1(), i.value().toLatin1());
    }

    QNetworkReply* reply = nullptr;
    if(postData.size() > 0) {
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = manager->post(req, postData.toLatin1());
    } else {
        reply = manager->get(req);
    }

    if(reply == nullptr) {
        manager->deleteLater();
        return false;
    }

    if(status != nullptr) {
        QObject::connect(reply, &QNetworkReply::downloadProgress, status, &WebDownloadStatus::progress);
    }

    loop.exec();

    if(reply->error() == QNetworkReply::NoError) {
        output.clear();
        output = reply->readAll();
    }

    reply->deleteLater();
    manager->deleteLater();

    return reply->error() == QNetworkReply::NoError;
}
