#include "Registration.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap Registration::toMap() const {
    return {{"id",id},{"patient_id",patientId},{"department_id",departmentId},
            {"admission_date",admissionDate.toString(Qt::ISODate)},
            {"discharge_date",dischargeDate.toString(Qt::ISODate)},
            {"bed_number",bedNumber},{"ward",ward},{"type",type},
            {"admitting_doctor",admittingDoctor},{"reason",reason},{"status",status}};
}
Registration Registration::fromQuery(const QSqlQuery& q) {
    Registration r;
    r.id             = q.value("id").toLongLong();
    r.patientId      = q.value("patient_id").toLongLong();
    r.departmentId   = q.value("department_id").toLongLong();
    r.admissionDate  = QDateTime::fromString(q.value("admission_date").toString(), Qt::ISODate);
    r.dischargeDate  = QDateTime::fromString(q.value("discharge_date").toString(), Qt::ISODate);
    r.bedNumber      = q.value("bed_number").toString();
    r.ward           = q.value("ward").toString();
    r.type           = q.value("type").toString();
    r.admittingDoctor= q.value("admitting_doctor").toLongLong();
    r.reason         = q.value("reason").toString();
    r.status         = q.value("status").toString();
    r.patientName    = q.value("patient_name").toString();
    r.doctorName     = q.value("doctor_name").toString();
    r.departmentName = q.value("department_name").toString();
    return r;
}
} // namespace HMS
