#pragma once

#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <optional>
#include <expected>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Patient {
    qint64      id{0};
    QString     patientUid;     // HMS-YYYYMMDD-XXXX
    QString     firstName;
    QString     lastName;
    QDate       dateOfBirth;
    QString     gender;
    QString     bloodType;
    QString     phone;
    QString     email;
    QString     address;
    QString     emergencyContact;
    QString     emergencyPhone;
    QString     insuranceId;
    QString     status{"active"};
    QString     qrCode;         // Base64 QR data
    QDateTime   createdAt;
    QDateTime   updatedAt;

    // Computed
    [[nodiscard]] QString fullName() const {
        return firstName + " " + lastName;
    }

    [[nodiscard]] int age() const {
        return dateOfBirth.daysTo(QDate::currentDate()) / 365;
    }

    [[nodiscard]] QVariantMap toMap() const;
    static std::expected<Patient, QString> fromMap(const QVariantMap& map);
    static Patient fromQuery(const QSqlQuery& q);
};

} // namespace HMS
