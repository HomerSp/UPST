#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "web/webutils.h"
#include "userguidehelper.h"

using namespace UI::Guide;

UserGuideHelper::UserGuideHelper(QObject* parent)
    : QObject(parent)
{

}

QString UserGuideHelper::getData() {
    QByteArray userGuideData;
    if(!Web::WebUtils::download(QUrl("http://upst.ultimobile.net/endpoint/guide.php"), userGuideData, "g=user")) {
        return "";
    }

    QJsonDocument doc = QJsonDocument::fromJson(userGuideData);
    const QJsonObject &obj = doc.object();

    QString data = "<!DOCTYPE HTML>\
            <html>\
                <head>\
                    <meta charset=\"UTF-8\">\
                        <title>Document</title>\
                        <style type=\"text/css\">\
                        body { color: #262626; margin: 0; padding: 0; }\
                        h1 { color: white; margin: 0; padding: 0; }\
                        h3 { color: white; margin: 0; padding: 0; }\
                        h2 { color: #288CF2; margin-bottom: 12px; }\
                        p { color: #262626; font-size: 0.9em; margin: 0; padding: 0; }\
                        #body { padding-bottom: 150px; }\
                        #content { padding: 20px; }\
                        #content tr { height: 400px; }\
                        #content td { vertical-align: top; }\
                        .content-left { vertical-align: top; height: 400px; } \
                        .content-right { width: 480px; text-align: right; } \
                    </style>\
                    </head>\
                    <body>\
                        <div id=\"body\">\
                            <div id=\"content\">\
                                <table width=\"100%\" cellspacing=\"20\">";

    QJsonArray steps = obj["data"].toArray();
    for(int i = 0; i < steps.size(); i++) {
        QJsonObject step = steps.at(i).toObject();

        data += "<tr height=\"400\">\
                    <td class=\"content-left\">\
                        <h2>" + step["header"].toString("") + "</h2>\
                        <p>" + step["line"].toString() + "</p>\
                    </td>\
                    <td class=\"content-right\" width=\"480\"  height=\"400\">";
        if(step.contains("image") && step["image"].toString().length() > 0) {
            data += "<img src=\"" + step["image"].toString() + "\" align=\"right\"/>";
        }
        data += "</td></tr>";
    }

    data += "</table></div></div></body>";

    return data;
}
