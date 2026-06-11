#include "MedicalRecord.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap MedicalRecord::toMap() const {
    return {{"id",id},{"patient_id",patientId},{"doctor_id",doctorId},
            {"appointment_id",appointmentId},{"diagnosis",diagnosis},
            {"symptoms",symptoms},{"prescription",prescription},
            {"lab_results",labResults},{"notes",notes},
            {"follow_up_date",followUpDate.toString(Qt::ISODate)}};
}
MedicalRecord MedicalRecord::fromQuery(const QSqlQuery& q) {
    MedicalRecord m;
    m.id            = q.value("id").toLongLong();
    m.patientId     = q.value("patient_id").toLongLong();
    m.doctorId      = q.value("doctor_id").toLongLong();
    m.appointmentId = q.value("appointment_id").toLongLong();
    m.diagnosis     = q.value("diagnosis").toString();
    m.symptoms      = q.value("symptoms").toString();
    m.prescription  = q.value("prescription").toString();
    m.labResults    = q.value("lab_results").toString();
    m.notes         = q.value("notes").toString();
    m.followUpDate  = QDate::fromString(q.value("follow_up_date").toString(), Qt::ISODate);
    m.patientName   = q.value("patient_name").toString();
    m.doctorName    = q.value("doctor_name").toString();
    return m;
}
} // namespace HMS
