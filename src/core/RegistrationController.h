#pragma once
#include "models/Registration.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class RegistrationController : public QObject {
    Q_OBJECT
public:
    explicit RegistrationController(QObject* parent = nullptr);

    std::expected<Registration, QString>              create(Registration reg);
    std::expected<Registration, QString>              findById(qint64 id);
    std::expected<std::vector<Registration>, QString> findAll(int limit=200, int offset=0);
    std::expected<std::vector<Registration>, QString> findAdmitted();
    std::expected<void, QString>                      discharge(qint64 id);
    std::expected<void, QString>                      remove(qint64 id);

signals:
    void registrationCreated(const Registration& r);
    void patientDischarged(qint64 id);

private:
    DatabaseManager& m_db;
    const QString JOIN_SQL = R"(
        SELECT r.*,
               p.first_name || ' ' || p.last_name AS patient_name,
               d.first_name || ' ' || d.last_name AS doctor_name,
               dep.name AS department_name
        FROM registrations r
        LEFT JOIN patients p   ON r.patient_id = p.id
        LEFT JOIN doctors d    ON r.admitting_doctor = d.id
        LEFT JOIN departments dep ON r.department_id = dep.id
    )";
};

} // namespace HMS
