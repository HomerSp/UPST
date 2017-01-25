#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

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
    qWarning()<<"download"<<url.path();

    if(url.host() == "upst.ultimobile.net" && url.path().startsWith("/endpoint")) {
        if(url.fileName() == "login.php") {
            QString token = "";
            if(headers.contains("U-Token")) {
                if(headers["U-Token"] == "choice" || headers["U-Token"] == "choicentua" || headers["U-Token"] == "choicevi" || headers["U-Token"] == "mio") {
                    token = headers["U-Token"];
                } else {
                    if(headers["U-Token"] == "e63ebc5a6fbcf122bed17a83c494f7afb116cdf7ef488f5e770837f54a160b3f" || headers["U-Token"] == "d4dc072f30090262eda15c809cc3e80495de6c44eb51ed82d7b75fd0e67cde89" || headers["U-Token"] == "da4f4e24b61152ce2931af0eb9178783757041a1a4116a7d1aa689bb2dc94016") {
                        token = "choice";
                    } else if(headers["U-Token"] == "abb40ec814950c4d8ba7a44ac7948a48d496bdbc04690fffc4893dc98de2d86f") {
                        token = "choicentua";
                    } else if(headers["U-Token"] == "252cac7d1da530dc7df84d64ac060ce05d0a284c1ea160e515cf819bf308cf36") {
                        token = "choicevi";
                    } else if(headers["U-Token"] == "229a29893191c1eeaee59928af95b0e1485e55bc7c17c2860fdfb2398961fd78") {
                        token = "mio";
                    }
                }
            } else {
                if(postData.contains("cf83f0f9828551b6247bfc9288d0740dd5b3ccb8501f8caed37c19b912ae4438") || postData.contains("705c26e619d6a3a59a3b1acfb2f1875715beaeadf149321202c62d4dbbdd358f") || postData.contains("97208fbe2152b970655e0aed7b5172edb03f32d44ce86f56a8feb9f531b446f0")) {
                    token = "choice";
                } else if(postData.contains("4b81dccfc02b91819a79df49b9854594c263799f43be308625d7c2880e65dd5e")) {
                    token = "choicentua";
                } else if(postData.contains("2a973bddfe0d42865d2c96409e8daf231a0c845f91877fea53309fa8a09b9708")) {
                    token = "choicevi";
                } else if(postData.contains("ea31ac1d48d1880bfd2b4179f5da29f202f5daf778715f1aeb3a1f06149941a2")) {
                    token = "mio";
                }
            }

            if(token.length() == 0) {
                return false;
            }

            QString displayName = "Choice";
            if(token == "choicentua") {
                displayName = "Choice NTUA";
            } else if(token == "choicevi") {
                displayName = "Choice VI";
            } else if(token == "mio") {
                displayName = "Mio";
            }

            output.clear();
            output = QString("{\"display_name\": \"" + displayName + "\", \"token\": \"" + token + "\"}").toLatin1();
            return true;
        } else if(url.fileName() == "check_update.php" || url.fileName() == "tracking.php") {
            output.clear();
            return true;
        } else if(url.fileName() == "devices.php") {
            QString name = headers["U-Token"];

            QFile sourceFile(QStringLiteral(":/res/data/devices.json"));
            if(!sourceFile.open(QIODevice::ReadOnly)) {
                return false;
            }

            QJsonDocument doc = QJsonDocument::fromJson(sourceFile.readAll());
            sourceFile.close();

            output.clear();
            output = QJsonDocument(doc.object().value(name).toObject()).toJson();

            return true;
        } else if(url.fileName() == "guide.php") {
            QFile sourceFile(QStringLiteral(":/res/data/user_guide.json"));
            if(!sourceFile.open(QIODevice::ReadOnly)) {
                return false;
            }

            output.clear();
            output = sourceFile.readAll();
            sourceFile.close();
            return true;
        } else if(url.fileName() == "provision.php") {
            QString name = headers["U-Token"];
            QString id = postData.mid(2);

            QFile sourceFile(QStringLiteral(":/res/data/provision_data.json"));
            if(!sourceFile.open(QIODevice::ReadOnly)) {
                return false;
            }

            QJsonDocument doc = QJsonDocument::fromJson(sourceFile.readAll());
            sourceFile.close();

            output.clear();
            output = QJsonDocument(doc.object().value(name).toObject().value(id).toObject()).toJson();

            return true;
        }
    } else if(url.host() == "umpst.ultimobile.com") {
        if(url.path().startsWith("/umpst/PRLS")) {
            QFile sourceFile(QStringLiteral(":/res/data/prl/") + url.fileName());
            sourceFile.open(QIODevice::ReadOnly);
            output = sourceFile.readAll();
            sourceFile.close();
            return true;
        } else if(url.path().startsWith("/umpst/CALIBRATIONS")) {
            QString name = url.path().mid(20);
            name = name.replace('/', '_');

            QFile sourceFile(QStringLiteral(":/res/data/calibration/") + name);
            if(!sourceFile.open(QIODevice::ReadOnly)) {
                return false;
            }

            output = sourceFile.readAll();
            sourceFile.close();
            return true;
        }
    }

    return false;
}
