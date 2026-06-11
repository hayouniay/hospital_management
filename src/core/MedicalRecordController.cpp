#include "MedicalRecordController.h"

namespace HMS {

MedicalRecordController::MedicalRecordController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<MedicalRecord, QString> MedicalRecordController::create(MedicalRecord rec) {
    auto r = m_db.execute(R"(
        INSERT INTO medical_records
            (patient_id, doctor_id, appointment_id, diagnosis, symptoms,
             prescription, lab_results, notes, follow_up_date)
        VALUES (?,?,?,?,?,?,?,?,?))",
        {rec.patientId, rec.doctorId,
         rec.appointmentId > 0 ? QVariant(rec.appointmentId) : QVariant(),
         rec.diagnosis, rec.symptoms, rec.prescription,
         rec.labResults, rec.notes,
         rec.followUpDate.isValid() ? rec.followUpDate.toString(Qt::ISODate) : QVariant()});
    if (!r) return std::unexpected(r.error());
    rec.id = m_db.lastInsertId();
    emit recordCreated(rec);
    return rec;
}

std::expected<MedicalRecord, QString> MedicalRecordController::findById(qint64 id) {
    auto r = m_db.execute(JOIN_SQL + " WHERE mr.id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Record not found");
    return MedicalRecord::fromQuery(*r);
}

std::expected<std::vector<MedicalRecord>, QString> MedicalRecordController::findAll(int limit, int offset) {
    auto r = m_db.execute(JOIN_SQL + " ORDER BY mr.created_at DESC LIMIT ? OFFSET ?", {limit, offset});
    if (!r) return std::unexpected(r.error());
    std::vector<MedicalRecord> list;
    while ((*r).next()) list.push_back(MedicalRecord::fromQuery(*r));
    return list;
}

std::expected<std::vector<MedicalRecord>, QString> MedicalRecordController::findByPatient(qint64 patientId) {
    auto r = m_db.execute(JOIN_SQL + " WHERE mr.patient_id=? ORDER BY mr.created_at DESC", {patientId});
    if (!r) return std::unexpected(r.error());
    std::vector<MedicalRecord> list;
    while ((*r).next()) list.push_back(MedicalRecord::fromQuery(*r));
    return list;
}

std::expected<std::vector<MedicalRecord>, QString> MedicalRecordController::findByDoctor(qint64 doctorId) {
    auto r = m_db.execute(JOIN_SQL + " WHERE mr.doctor_id=? ORDER BY mr.created_at DESC", {doctorId});
    if (!r) return std::unexpected(r.error());
    std::vector<MedicalRecord> list;
    while ((*r).next()) list.push_back(MedicalRecord::fromQuery(*r));
    return list;
}

std::expected<std::vector<MedicalRecord>, QString> MedicalRecordController::search(const QString& query) {
    const QString like = "%" + query + "%";
    auto r = m_db.execute(R"(
        SELECT mr.*,
               p.first_name || ' ' || p.last_name AS patient_name,
               d.first_name || ' ' || d.last_name AS doctor_name
        FROM medical_records mr
        LEFT JOIN patients p ON mr.patient_id = p.id
        LEFT JOIN doctors d  ON mr.doctor_id  = d.id
        WHERE p.first_name LIKE ? OR p.last_name LIKE ?
           OR mr.diagnosis LIKE ? OR mr.prescription LIKE ?
        ORDER BY mr.created_at DESC LIMIT 100)",
        {like, like, like, like});
    if (!r) return std::unexpected(r.error());
    std::vector<MedicalRecord> list;
    while ((*r).next()) list.push_back(MedicalRecord::fromQuery(*r));
    return list;
}

std::expected<MedicalRecord, QString> MedicalRecordController::update(const MedicalRecord& rec) {
    auto r = m_db.execute(R"(
        UPDATE medical_records SET
            patient_id=?, doctor_id=?, appointment_id=?, diagnosis=?,
            symptoms=?, prescription=?, lab_results=?, notes=?,
            follow_up_date=?, updated_at=datetime('now')
        WHERE id=?)",
        {rec.patientId, rec.doctorId,
         rec.appointmentId > 0 ? QVariant(rec.appointmentId) : QVariant(),
         rec.diagnosis, rec.symptoms, rec.prescription,
         rec.labResults, rec.notes,
         rec.followUpDate.isValid() ? rec.followUpDate.toString(Qt::ISODate) : QVariant(),
         rec.id});
    if (!r) return std::unexpected(r.error());
    emit recordUpdated(rec);
    return rec;
}

std::expected<void, QString> MedicalRecordController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM medical_records WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    return {};
}

} // namespace HMS
