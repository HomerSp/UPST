#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "webutils.h"

using namespace Web;

WebUtils::WebUtils()
{

}

bool WebUtils::download(const QUrl& url, QByteArray& output) {
    qDebug()<<"download"<<url.toString();

    WebLoop loop;

    QNetworkAccessManager *manager = new QNetworkAccessManager(&loop);
    QObject::connect(manager, &QNetworkAccessManager::finished, &loop, &WebLoop::replyFinished);

    manager->get(QNetworkRequest(url));
    loop.exec();

    QNetworkReply* reply = loop.reply();
    if(reply != nullptr) {
        output.clear();
        output = reply->readAll();

        reply->deleteLater();
        manager->deleteLater();

        return true;
    }

    manager->deleteLater();
    return false;
}
