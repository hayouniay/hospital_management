#pragma once
#include "models/Appointment.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class AppointmentController : public QObject {
    Q_OBJECT
public:
    explicit AppointmentController(QObject* parent = nullptr);

    std::expected<Appointment, QString>             create(Appointment appt);
    std::expected<Appointment, QString>             findById(qint64 id);
    std::expected<std::vector<Appointment>, QString> findAll(int limit=100, int offset=0);
    std::expected<std::vector<Appointment>, QString> findByPatient(qint64 patientId);
    std::expected<std::vector<Appointment>, QString> findByDoctor(qint64 doctorId);
    std::expected<std::vector<Appointment>, QString> findByDate(const QDate& date);
    std::expected<std::vector<Appointment>, QString> findToday();
    std::expected<Appointment, QString>             update(const Appointment& appt);
    std::expected<void, QString>                    updateStatus(qint64 id, const QString& status);
    std::expected<void, QString>                    remove(qint64 id);
    std::expected<int, QString>                     countByStatus(const QString& status);

signals:
    void appointmentCreated(const Appointment& a);
    void appointmentUpdated(const Appointment& a);
    void statusChanged(qint64 id, const QString& status);

private:
    DatabaseManager& m_db;
    const QString JOIN_SQL = R"(
        SELECT a.*,
               p.first_name || ' ' || p.last_name AS patient_name,
               d.first_name || ' ' || d.last_name AS doctor_name,
               dep.name AS department_name
        FROM appointments a
        LEFT JOIN patients p ON a.patient_id = p.id
        LEFT JOIN doctors d ON a.doctor_id = d.id
        LEFT JOIN departments dep ON a.department_id = dep.id
    )";
};

} // namespace HMS
