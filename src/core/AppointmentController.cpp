#include "AppointmentController.h"

namespace HMS {

AppointmentController::AppointmentController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Appointment, QString> AppointmentController::create(Appointment appt) {
    const QString sql = R"(
        INSERT INTO appointments
            (patient_id, doctor_id, department_id, appointment_date,
             appointment_time, duration_min, type, status, notes)
        VALUES (?,?,?,?,?,?,?,?,?)
    )";

    auto r = m_db.execute(sql, {
        appt.patientId, appt.doctorId, appt.departmentId,
        appt.appointmentDate.toString(Qt::ISODate),
        appt.appointmentTime.toString("HH:mm"),
        appt.durationMin, appt.type, appt.status, appt.notes
    });

    if (!r) return std::unexpected(r.error());
    appt.id = m_db.lastInsertId();
    emit appointmentCreated(appt);
    return appt;
}

std::expected<Appointment, QString> AppointmentController::findById(qint64 id) {
    auto r = m_db.execute(JOIN_SQL + " WHERE a.id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Appointment not found");
    return Appointment::fromQuery(*r);
}

std::expected<std::vector<Appointment>, QString> AppointmentController::findAll(int limit, int offset) {
    auto r = m_db.execute(JOIN_SQL + " ORDER BY a.appointment_date DESC, a.appointment_time LIMIT ? OFFSET ?",
        {limit, offset});
    if (!r) return std::unexpected(r.error());
    std::vector<Appointment> list;
    while ((*r).next()) list.push_back(Appointment::fromQuery(*r));
    return list;
}

std::expected<std::vector<Appointment>, QString> AppointmentController::findByPatient(qint64 patientId) {
    auto r = m_db.execute(JOIN_SQL + " WHERE a.patient_id=? ORDER BY a.appointment_date DESC", {patientId});
    if (!r) return std::unexpected(r.error());
    std::vector<Appointment> list;
    while ((*r).next()) list.push_back(Appointment::fromQuery(*r));
    return list;
}

std::expected<std::vector<Appointment>, QString> AppointmentController::findByDate(const QDate& date) {
    auto r = m_db.execute(JOIN_SQL + " WHERE a.appointment_date=? ORDER BY a.appointment_time",
        {date.toString(Qt::ISODate)});
    if (!r) return std::unexpected(r.error());
    std::vector<Appointment> list;
    while ((*r).next()) list.push_back(Appointment::fromQuery(*r));
    return list;
}

std::expected<std::vector<Appointment>, QString> AppointmentController::findToday() {
    return findByDate(QDate::currentDate());
}

std::expected<std::vector<Appointment>, QString> AppointmentController::findByDoctor(qint64 doctorId) {
    auto r = m_db.execute(JOIN_SQL + " WHERE a.doctor_id=? ORDER BY a.appointment_date DESC, a.appointment_time", {doctorId});
    if (!r) return std::unexpected(r.error());
    std::vector<Appointment> list;
    while ((*r).next()) list.push_back(Appointment::fromQuery(*r));
    return list;
}

std::expected<Appointment, QString> AppointmentController::update(const Appointment& appt) {
    const QString sql = R"(
        UPDATE appointments SET
            patient_id=?, doctor_id=?, department_id=?,
            appointment_date=?, appointment_time=?, duration_min=?,
            type=?, status=?, notes=?, updated_at=datetime('now')
        WHERE id=?
    )";
    auto r = m_db.execute(sql, {
        appt.patientId, appt.doctorId, appt.departmentId,
        appt.appointmentDate.toString(Qt::ISODate),
        appt.appointmentTime.toString("HH:mm"),
        appt.durationMin, appt.type, appt.status, appt.notes, appt.id
    });
    if (!r) return std::unexpected(r.error());
    emit appointmentUpdated(appt);
    return appt;
}

std::expected<void, QString> AppointmentController::updateStatus(qint64 id, const QString& status) {
    auto r = m_db.execute("UPDATE appointments SET status=?, updated_at=datetime('now') WHERE id=?",
        {status, id});
    if (!r) return std::unexpected(r.error());
    emit statusChanged(id, status);
    return {};
}

std::expected<void, QString> AppointmentController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM appointments WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    return {};
}

std::expected<int, QString> AppointmentController::countByStatus(const QString& status) {
    auto r = m_db.execute("SELECT COUNT(*) FROM appointments WHERE status=?", {status});
    if (!r) return std::unexpected(r.error());
    (*r).next();
    return (*r).value(0).toInt();
}

} // namespace HMS
