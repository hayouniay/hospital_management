#include "Appointment.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap Appointment::toMap() const {
    return {{"id",id},{"patient_id",patientId},{"doctor_id",doctorId},
            {"department_id",departmentId},
            {"appointment_date",appointmentDate.toString(Qt::ISODate)},
            {"appointment_time",appointmentTime.toString("HH:mm")},
            {"duration_min",durationMin},{"type",type},{"status",status},{"notes",notes}};
}
Appointment Appointment::fromQuery(const QSqlQuery& q) {
    Appointment a;
    a.id              = q.value("id").toLongLong();
    a.patientId       = q.value("patient_id").toLongLong();
    a.doctorId        = q.value("doctor_id").toLongLong();
    a.departmentId    = q.value("department_id").toLongLong();
    a.appointmentDate = QDate::fromString(q.value("appointment_date").toString(), Qt::ISODate);
    a.appointmentTime = QTime::fromString(q.value("appointment_time").toString(), "HH:mm");
    a.durationMin     = q.value("duration_min").toInt();
    a.type            = q.value("type").toString();
    a.status          = q.value("status").toString();
    a.notes           = q.value("notes").toString();
    a.patientName     = q.value("patient_name").toString();
    a.doctorName      = q.value("doctor_name").toString();
    a.departmentName  = q.value("department_name").toString();
    return a;
}
} // namespace HMS
