#include "RegistrationController.h"
namespace HMS {

RegistrationController::RegistrationController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Registration, QString> RegistrationController::create(Registration reg) {
    auto r = m_db.execute(R"(
        INSERT INTO registrations
            (patient_id, department_id, admission_date, bed_number, ward,
             type, admitting_doctor, reason, status)
        VALUES (?,?,datetime('now'),?,?,?,?,?,?))",
        {reg.patientId,
         reg.departmentId > 0 ? QVariant(reg.departmentId) : QVariant(),
         reg.bedNumber, reg.ward, reg.type,
         reg.admittingDoctor > 0 ? QVariant(reg.admittingDoctor) : QVariant(),
         reg.reason, reg.status});
    if (!r) return std::unexpected(r.error());
    reg.id = m_db.lastInsertId();
    emit registrationCreated(reg);
    return reg;
}

std::expected<Registration, QString> RegistrationController::findById(qint64 id) {
    auto r = m_db.execute(JOIN_SQL + " WHERE r.id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Registration not found");
    return Registration::fromQuery(*r);
}

std::expected<std::vector<Registration>, QString> RegistrationController::findAll(int limit, int offset) {
    auto r = m_db.execute(JOIN_SQL + " ORDER BY r.admission_date DESC LIMIT ? OFFSET ?", {limit, offset});
    if (!r) return std::unexpected(r.error());
    std::vector<Registration> list;
    while ((*r).next()) list.push_back(Registration::fromQuery(*r));
    return list;
}

std::expected<std::vector<Registration>, QString> RegistrationController::findAdmitted() {
    auto r = m_db.execute(JOIN_SQL + " WHERE r.status='admitted' ORDER BY r.admission_date DESC");
    if (!r) return std::unexpected(r.error());
    std::vector<Registration> list;
    while ((*r).next()) list.push_back(Registration::fromQuery(*r));
    return list;
}

std::expected<void, QString> RegistrationController::discharge(qint64 id) {
    auto r = m_db.execute(
        "UPDATE registrations SET status='discharged', discharge_date=datetime('now'), updated_at=datetime('now') WHERE id=?",
        {id});
    if (!r) return std::unexpected(r.error());
    emit patientDischarged(id);
    return {};
}

std::expected<void, QString> RegistrationController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM registrations WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    return {};
}

} // namespace HMS
