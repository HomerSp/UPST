#include <QJsonDocument>

#include "../../serialcommand.h"
#include "nvprovisiondata.h"

#include "../../../web/webutils.h"

#include "../commands/prlcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/aoccommand.h"
#include "../commands/nvcommands/nvprovisioncommands/genuserprofcommand.h"
#include "../commands/nvcommands/nvprovisioncommands/genusersscommand.h"
#include "../commands/nvcommands/nvprovisioncommands/evrccommand.h"
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
        qDebug()<<"prlData size"<<prlData.size();
        commands().append(new Serial::QCDM::Commands::PRLCommand(device(), false, prlData));
    }
}

void NvProvisionData::update(const QString &data) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toLatin1());

    QJsonObject rootObject = jsonDoc.object();

    updateStart(rootObject);
    SerialProvisionData::update(rootObject);
}

Serial::SerialCommand* NvProvisionData::getCommand(const QString& parent, const QString& name, const QJsonValue& jsonValue) {
#define PROVISION_CMD(n, className, args...) \
        if(QString(name).toLower() == QString(n).toLower() && (!jsonValue.isString() || (jsonValue.isString() && jsonValue.toString().toInt() != -1))) {\
            cmd = new Serial::QCDM::Commands::Nv::Provision::className(device(), false, &jsonValue,##args);\
            break;\
        }

    Serial::SerialCommand* cmd = nullptr;
    do {
        if(parent == "") {
            PROVISION_CMD("genUserProf", GenUserProfCommand, user());
            PROVISION_CMD("genUserSS", GenUserSSCommand);
            PROVISION_CMD("password", PasswordCommand);
        } else if(parent == "mobileIP") {
            PROVISION_CMD("mipPref", ProvisionCommand, NvItem::NV_DS_QCMIP_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("numOfProf", ProvisionCommand, NvItem::NV_DS_MIP_NUM_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("currentActiveProf", ProvisionCommand, NvItem::NV_DS_MIP_ACTIVE_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("MIPNaiUser", GenericNaiCommand, NvItem::NV_DS_MIP_RM_NAI_I, NvItemType::NV_GENERIC_MIP_NAI_BYTE_ARRAY, userType(), userProfIndex());
            PROVISION_CMD("MIPDMUPkoid", ProvisionCommand, NvItem::NV_DS_MIP_DMU_PKOID_I, NvItemType::NV_GENERIC_MIP_BYTE_ARRAY, userProfIndex());
            PROVISION_CMD("mipProfEnable", ProvisionCommand, NvItem::NV_DS_MIP_ENABLE_PROF_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipRegistrationRetries", ProvisionCommand, NvItem::NV_DS_MIP_RETRIES_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipRetryInterval", ProvisionCommand, NvItem::NV_DS_MIP_RETRY_INT_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipPreReRRQTime", ProvisionCommand, NvItem::NV_DS_MIP_PRE_RE_RRQ_TIME_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("mipRRQ", ProvisionCommand, NvItem::NV_DS_MIP_RRQ_IF_TFRK_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
            PROVISION_CMD("dormantHandOffOptEnable", ProvisionCommand, NvItem::NV_DS_MIP_QC_HANDDOWN_TO_1X_OPT_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
            PROVISION_CMD("mnHARFC2002bis", ProvisionCommand, NvItem::NV_DS_MIP_2002BIS_MN_HA_AUTH_I, NvItemType::NV_GENERIC_FLAG_ARRAY);
        } else if(parent == "data") {
            PROVISION_CMD("PPPUser", GenericNaiCommand, NvItem::NV_PPP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("PAPUser", GenericNaiCommand, NvItem::NV_PAP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("HDRANUser", GenericNaiCommand, NvItem::NV_HDR_AN_AUTH_NAI_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("HDRANLongUser", GenericNaiCommand, NvItem::NV_HDR_AN_AUTH_USER_ID_LONG_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("HDRPPPUser", GenericNaiCommand, NvItem::NV_HDR_AN_PPP_USER_ID_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("SIPNAIUser", GenericNaiCommand, NvItem::NV_DS_SIP_RM_NAI_I, NvItemType::NV_GENERIC_NAI_BYTE_ARRAY, userType());
            PROVISION_CMD("PPPDetection", ProvisionCommand, NvItem::NV_DATA_AUTO_PACKET_DETECTION_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("EvdoRxControl", ProvisionCommand, NvItem::NV_HDR_RX_DIVERSITY_CTRL_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("SCPSession", ProvisionCommand, NvItem::NV_HDRSCP_SESSION_STATUS_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("SCPHDRATConfig", ProvisionCommand, NvItem::NV_HDRSCP_FORCE_AT_CONFIG_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("SCPHDRRel0Config", ProvisionCommand, NvItem::NV_HDRSCP_FORCE_REL0_CONFIG_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
        } else if(parent == "nam") {
            PROVISION_CMD("evrc", EvrcCommand);
            PROVISION_CMD("sid_nid_list", SidNidListCommand);

            PROVISION_CMD("name", ProvisionCommand, NvItem::NV_NAME_NAM_I, NvItemType::NV_GENERIC_NAM_ANSI_ARRAY);
            PROVISION_CMD("homeSID", ProvisionCommand, NvItem::NV_ANALOG_HOME_SID_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
            PROVISION_CMD("voicePriv", ProvisionCommand, NvItem::NV_VOICE_PRIV_I, NvItemType::NV_GENERIC_BYTE_ARRAY);
            PROVISION_CMD("prefMode", ProvisionCommand, NvItem::NV_PREF_MODE_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("primaryDNS", ProvisionCommand, NvItem::NV_PRIMARY_DNS_I, NvItemType::NV_GENERIC_IP_BYTE_ARRAY);
            PROVISION_CMD("secondaryDNS", ProvisionCommand, NvItem::NV_SECONDARY_DNS_I, NvItemType::NV_GENERIC_IP_BYTE_ARRAY);
            PROVISION_CMD("namLock", ProvisionCommand, NvItem::NV_NAM_LOCK_I, NvItemType::NV_GENERIC_NAM_FLAG_ARRAY);
            PROVISION_CMD("otapaEnabled", ProvisionCommand, NvItem::NV_OTAPA_ENABLED_I, NvItemType::NV_GENERIC_NAM_FLAG_ARRAY);
            PROVISION_CMD("useIMSI", ProvisionCommand, NvItem::NV_IMSI_ADDR_NUM_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("useIMSIT", ProvisionCommand, NvItem::NV_IMSI_T_ADDR_NUM_I, NvItemType::NV_GENERIC_NAM_BYTE_ARRAY);
            PROVISION_CMD("mccIMSI", ProvisionCommand, NvItem::NV_IMSI_MCC_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
            PROVISION_CMD("mncIMSI", ProvisionCommand, NvItem::NV_IMSI_11_12_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
            PROVISION_CMD("mccIMSIT", ProvisionCommand, NvItem::NV_IMSI_T_MCC_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
            PROVISION_CMD("mncIMSIT", ProvisionCommand, NvItem::NV_IMSI_T_11_12_I, NvItemType::NV_GENERIC_NAM_UINT16_ARRAY);
        }
    } while(false);

#ifdef TESTING_MODE
    if(cmd != nullptr) {
        cmd->setDebuggingName(parent + ((parent.size() > 0)?"/":"") + name);
    }
#endif

    return cmd;
}
