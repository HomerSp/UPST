#ifndef SERIALPROVISIONDATA_H
#define SERIALPROVISIONDATA_H

#include <QJsonObject>
#include <QJsonValue>

#include "serialcommand.h"
#include "serialdevice.h"

namespace Serial {
    class SerialProvisionData
    {
    public:
        SerialProvisionData(SerialDevice* device);
        virtual ~SerialProvisionData();

        enum UserType {
            UserTypeMDN = 0,
            UserTypeESN,
            UserTypeMIN,
            UserTypeMEID,
        };

        static QString getUser(SerialDevice* device, UserType type, const QString& userNai);
        static QString getPassword(SerialDevice* device, const QString& str);

        const QString& carrierSPC() const {
            return mCarrierSPC;
        }

        bool sequentialOffline() const {
            return mSequentialOffline;
        }

        const QString& password16() const {
            return mPassword16;
        }

        const QList<SerialCommand*> constCommands() const {
            return mCommands;
        }

    protected:
        SerialDevice* device() {
            return mDevice;
        }

        const QString& user() const {
            return mUser;
        }

        UserType userType() const {
            return mUserType;
        }

        uint8_t userProfIndex() const {
            return mUserProfIndex;
        }

        QList<SerialCommand*> &commands() {
            return mCommands;
        }

        virtual SerialCommand* getCommand(const QString& parent, const QString& name, const QJsonValue& jsonValue) = 0;
        virtual void update(const QString& data) = 0;

        void update(const QJsonObject& rootObject);

    private:
        void updateObj(const QString& parent, const QJsonObject& obj);

        SerialDevice* mDevice;

        QString mCarrierSPC;
        bool mSequentialOffline;
        QString mPassword16;
        UserType mUserType;
        QString mUser;
        uint8_t mUserProfIndex;

        QList<SerialCommand*> mCommands;
    };
}

#endif // SERIALPROVISIONDATA_H
