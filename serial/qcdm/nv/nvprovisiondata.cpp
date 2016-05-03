#include <QJsonDocument>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCryptographicHash>

#include "../../serialcommand.h"
#include "nvprovisiondata.h"

#include "../../../web/webutils.h"

#include "../commands/prlcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/aoccommand.h"
#include "../commands/nvcommands/nvprovisioncommands/genuserprofcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/genusersscommand.h"
#include "../commands/nvcommands/nvprovisioncommands/evrccommand.h"
#include "../commands/nvcommands/nvprovisioncommands/hdranusercommand.h"
#include "../commands/nvcommands/nvprovisioncommands/mccimsicommand.h"
#include "../commands/nvcommands/nvprovisioncommands/mncimsicommand.h"
#include "../commands/nvcommands/nvprovisioncommands/sidnidlistcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/passwordcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/genericnaicommand.h"
#include "../commands/nvcommands/nvprovisioncommands/provisioncommand.h"

using namespace Serial::QCDM::Nv;

NvProvisionData::NvProvisionData(SerialDevice* device, const QString& data)
    : SerialProvisionData(device)
{
    update(data);
}

NvProvisionData::~NvProvisionData() {

}

void NvProvisionData::updateStart(const QJsonObject &rootObject) {
    commands().append(new Serial::QCDM::Commands::Nv::Provision::AOCCommand(device(), false));

    QByteArray prlData;
    if(Web::WebUtils::download(QUrl(rootObject["carrierPRL"].toString()), prlData)) {
        bool shouldAdd = true;
        if(rootObject.contains("carrierPRLmd5")) {
            QString carrierMd5 = rootObject["carrierPRLmd5"].toString().toLower();
            QString checkMd5 = QString(QCryptographicHash::hash(prlData, QCryptographicHash::Md5).toHex()).toLower();
            if(carrierMd5 != checkMd5) {
                qCritical()<<"PRL file md5 mismatch, found"<<carrierMd5<<"vs real"<<checkMd5;
                // Since UMPST doesn't use the md5, the md5 in the provisioning data doesn't match,
                // therefore we need to ignore that error here.
                // Update the provisioning data with the correct md5 pl0x.
                //shouldAdd = false;
            }
        }

        if(shouldAdd) {
            qDebug()<<"prlData size"<<prlData.size();
            commands().append(new Serial::QCDM::Commands::PRLCommand(device(), false, prlData));
        }
    }
}

void NvProvisionData::update(const QString &data) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toLatin1());
    if(jsonDoc.isObject()) {
        QJsonObject rootObject = jsonDoc.object();

        updateStart(rootObject);
        SerialProvisionData::update(rootObject);
    }
}

void NvProvisionData::updateCalibration(const QUrl& url, const QString& md5) {
    QByteArray calibrationData;
    if(Web::WebUtils::download(url, calibrationData)) {
        if(md5.size() > 0) {
            QString checkMd5 = QString(QCryptographicHash::hash(calibrationData, QCryptographicHash::Md5).toHex()).toLower();
            if(checkMd5 != md5) {
                qCritical()<<"Calibration file md5 mismatch, found"<<md5<<"vs real"<<checkMd5;
                // Since UMPST doesn't use the md5, the md5 in the provisioning data doesn't match,
                // therefore we need to ignore that error here.
                // Update the provisioning data with the correct md5 pl0x.
                //return;
            }
        }

        qDebug()<<"calibrationData"<<calibrationData.size();

        QString data = QString(calibrationData);

        int start = data.indexOf("[NV items]");
        if(start < 0) {
            return;
        }

        start = data.indexOf("[Complete items", start);

        QRegularExpressionMatch completeMatch = QRegularExpression("\\[Complete items - (\\d{1,}), Items size - (\\d{1,})\\]").match(data, start);
        if(!completeMatch.hasMatch()) {
            return;
        }

        int itemCount = completeMatch.captured(1).toUInt();
        int itemSize = completeMatch.captured(2).toUInt();
        int numRows = ceil(itemSize / 16.0f);

        qDebug()<<"itemCount"<<itemCount<<"itemSize"<<itemSize;
        start = data.indexOf('\n', start) + 1;

        for(int i = 0; i < itemCount; i++) {
            start = data.indexOf('\n', start) + 1;

            QRegularExpressionMatch nvItemMatch = QRegularExpression("(\\d{1,})").match(data, start);
            if(!nvItemMatch.hasMatch()) {
                return;
            }

            int nvItem = nvItemMatch.captured(1).toUInt();
            qDebug()<<"nvItem"<<nvItem;

            start = data.indexOf('\n', start) + 1;

            QByteArray nvItemData;
            for(int y = 0; y < numRows; y++) {
                for(int x = 0; x < 16; x++) {
                    if(x + (y * 16) >= itemSize) {
                        break;
                    }

                    QString str = data.mid(start + (x * 3), 2);
                    nvItemData.append(static_cast<char>(str.toUInt(0, 16)));
                }

                start = data.indexOf('\n', start) + 1;
            }

            qDebug()<<QString(nvItemData.toHex());

            commands().append(new Serial::QCDM::Commands::Nv::NvCommand(device(), false, static_cast<Serial::QCDM::NvItem>(nvItem), nvItemData));
        }
    }
}

Serial::SerialCommand* NvProvisionData::getCommand(const QString& parent, const QString& name, const QJsonValue& jsonValue) {
    bool found = false;
    QString parentLower = parent.toLower();
    QString nameLower = name.toLower();
#define PROVISION_CMD(n, className, args...) \
        if(nameLower == n) {\
            found = true;\
            if(!jsonValue.isString() || (jsonValue.isString() && jsonValue.toString().toInt() != -1)) {\
                cmd = new Serial::QCDM::Commands::Nv::Provision::className(device(), false, &jsonValue,##args);\
            }\
            break;\
        }
#define PROVISION_CMD_NULL(n) \
    if(nameLower == n) {\
        found = true; \
        break;\
    }

    Serial::SerialCommand* cmd = nullptr;
    do {
        if(parentLower == "") {
            PROVISION_CMD_NULL("calibrationfile");
            PROVISION_CMD_NULL("calibrationfilemd5");
            PROVISION_CMD_NULL("carrierprl");
            PROVISION_CMD_NULL("carrierprlmd5");
            PROVISION_CMD_NULL("carrierspc");
            PROVISION_CMD_NULL("data");
            PROVISION_CMD_NULL("mobileip");
            PROVISION_CMD_NULL("nam");
            PROVISION_CMD_NULL("nvitems");
            PROVISION_CMD_NULL("sequentialoffline");
            PROVISION_CMD_NULL("sixteendigitpassword");
            PROVISION_CMD_NULL("user");
            PROVISION_CMD_NULL("usertype");

            PROVISION_CMD("genuserprof", GenUserProfCommand, user());
            PROVISION_CMD("genuserss", GenUserSSCommand);
            PROVISION_CMD("password", PasswordCommand);
        } else if(parentLower == "mobileip") {
            PROVISION_CMD("mippref", ProvisionCommand, NvItem::NV_DS_QCMIP_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("numofprof", ProvisionCommand, NvItem::NV_DS_MIP_NUM_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("currentactiveprof", ProvisionCommand, NvItem::NV_DS_MIP_ACTIVE_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipnaiuser", GenericNaiCommand, NvItem::NV_DS_MIP_RM_NAI_I, NvItemType::NV_GENERIC_MIP_NAI_BYTE_ARRAY, userType(), userProfIndex());
            PROVISION_CMD("mipdmupkoid", ProvisionCommand, NvItem::NV_DS_MIP_DMU_PKOID_I, NvItemType::NV_GENERIC_MIP_BYTE_ARRAY, userProfIndex());
            PROVISION_CMD("mipprofenable", ProvisionCommand, NvItem::NV_DS_MIP_ENABLE_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipregistrationretries", ProvisionCommand, NvItem::NV_DS_MIP_RETRIES_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipretryinterval", ProvisionCommand, NvItem::NV_DS_MIP_RETRY_INT_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipprererrqtime", ProvisionCommand, NvItem::NV_DS_MIP_PRE_RE_RRQ_TIME_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("miprrq", ProvisionCommand, NvItem::NV_DS_MIP_RRQ_IF_TFRK_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
            PROVISION_CMD("dormanthandoffoptenable", ProvisionCommand, NvItem::NV_DS_MIP_QC_HANDDOWN_TO_1X_OPT_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
            PROVISION_CMD("mnharfc2002bis", ProvisionCommand, NvItem::NV_DS_MIP_2002BIS_MN_HA_AUTH_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
        } else if(parentLower == "data") {
            PROVISION_CMD_NULL("tetheredmipnai");
            PROVISION_CMD_NULL("tetherednai");

            PROVISION_CMD("pppuser", GenericNaiCommand, NvItem::NV_PPP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("papuser", GenericNaiCommand, NvItem::NV_PAP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("hdranuser", HDRAnUserCommand, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("hdrpppuser", GenericNaiCommand, NvItem::NV_HDR_AN_PPP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("sipnaiuser", GenericNaiCommand, NvItem::NV_DS_SIP_RM_NAI_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("pppdetection", ProvisionCommand, NvItem::NV_DATA_AUTO_PACKET_DETECTION_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("evdorxcontrol", ProvisionCommand, NvItem::NV_HDR_RX_DIVERSITY_CTRL_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("scpsession", ProvisionCommand, NvItem::NV_HDRSCP_SESSION_STATUS_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("hdrscpatconfig", ProvisionCommand, NvItem::NV_HDRSCP_FORCE_AT_CONFIG_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("scphdrrel0config", ProvisionCommand, NvItem::NV_HDRSCP_FORCE_REL0_CONFIG_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
        } else if(parentLower == "nam") {
            PROVISION_CMD_NULL("banner");
            PROVISION_CMD_NULL("cdmaprefserv");
            PROVISION_CMD_NULL("systempref");

            PROVISION_CMD("evrc", EvrcCommand);
            PROVISION_CMD("sid_nid_list", SidNidListCommand);

            PROVISION_CMD("name", ProvisionCommand, NvItem::NV_NAME_NAM_I, NvItemType::NV_GENERIC_NAM_ANSI_ARRAY);
            PROVISION_CMD("homesid", ProvisionCommand, NvItem::NV_ANALOG_HOME_SID_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
            PROVISION_CMD("voicepriv", ProvisionCommand, NvItem::NV_VOICE_PRIV_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("prefmode", ProvisionCommand, NvItem::NV_PREF_MODE_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("primarydns", ProvisionCommand, NvItem::NV_PRIMARY_DNS_I, NvItemType::NV_GENERIC_IP_BYTE_ARRAY);
            PROVISION_CMD("secondarydns", ProvisionCommand, NvItem::NV_SECONDARY_DNS_I, NvItemType::NV_GENERIC_IP_BYTE_ARRAY);
            PROVISION_CMD("namlock", ProvisionCommand, NvItem::NV_NAM_LOCK_I, NvItemType::NV_GENERIC_NAM_FLAG_ARRAY);
            PROVISION_CMD("otapaenable", ProvisionCommand, NvItem::NV_OTAPA_ENABLED_I, NvItemType::NV_GENERIC_NAM_FLAG_ARRAY);
            PROVISION_CMD("hybridmode", ProvisionCommand, NvItem::NV_HYBRID_PREF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("useimsi", ProvisionCommand, NvItem::NV_IMSI_ADDR_NUM_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("useimsit", ProvisionCommand, NvItem::NV_IMSI_T_ADDR_NUM_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("mccimsi", MccImsiCommand);
            PROVISION_CMD("mncimsi", MncImsiCommand);
        }
    } while(false);

    if(!found) {
        qWarning()<<"Could not find a handler for"<<parent<<"/"<<name;
    }

#ifdef TESTING_MODE
    if(cmd != nullptr) {
        cmd->setDebuggingName(parent + ((parent.size() > 0)?"/":"") + name);
    }
#endif

    return cmd;
}
