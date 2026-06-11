#include "Patient.h"
#include <QtSql/QSqlQuery>
#include <QVariant>

namespace HMS {

QVariantMap Patient::toMap() const {
    return {
        {"id",                id},
        {"patient_uid",       patientUid},
        {"first_name",        firstName},
        {"last_name",         lastName},
        {"date_of_birth",     dateOfBirth.toString(Qt::ISODate)},
        {"gender",            gender},
        {"blood_type",        bloodType},
        {"phone",             phone},
        {"email",             email},
        {"address",           address},
        {"emergency_contact", emergencyContact},
        {"emergency_phone",   emergencyPhone},
        {"insurance_id",      insuranceId},
        {"status",            status},
        {"qr_code",           qrCode},
    };
}

std::expected<Patient, QString> Patient::fromMap(const QVariantMap& map) {
    Patient p;
    if (!map.contains("first_name") || map["first_name"].toString().isEmpty()) {
        return std::unexpected("First name is required");
    }
    p.id            = map.value("id", 0).toLongLong();
    p.patientUid    = map.value("patient_uid").toString();
    p.firstName     = map.value("first_name").toString();
    p.lastName      = map.value("last_name").toString();
    p.dateOfBirth   = QDate::fromString(map.value("date_of_birth").toString(), Qt::ISODate);
    p.gender        = map.value("gender").toString();
    p.bloodType     = map.value("blood_type").toString();
    p.phone         = map.value("phone").toString();
    p.email         = map.value("email").toString();
    p.address       = map.value("address").toString();
    p.emergencyContact = map.value("emergency_contact").toString();
    p.emergencyPhone   = map.value("emergency_phone").toString();
    p.insuranceId   = map.value("insurance_id").toString();
    p.status        = map.value("status", "active").toString();
    p.qrCode        = map.value("qr_code").toString();
    return p;
}

Patient Patient::fromQuery(const QSqlQuery& q) {
    Patient p;
    p.id              = q.value("id").toLongLong();
    p.patientUid      = q.value("patient_uid").toString();
    p.firstName       = q.value("first_name").toString();
    p.lastName        = q.value("last_name").toString();
    p.dateOfBirth     = QDate::fromString(q.value("date_of_birth").toString(), Qt::ISODate);
    p.gender          = q.value("gender").toString();
    p.bloodType       = q.value("blood_type").toString();
    p.phone           = q.value("phone").toString();
    p.email           = q.value("email").toString();
    p.address         = q.value("address").toString();
    p.emergencyContact= q.value("emergency_contact").toString();
    p.emergencyPhone  = q.value("emergency_phone").toString();
    p.insuranceId     = q.value("insurance_id").toString();
    p.status          = q.value("status").toString();
    p.qrCode          = q.value("qr_code").toString();
    p.createdAt       = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    p.updatedAt       = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    return p;
}

} // namespace HMS
