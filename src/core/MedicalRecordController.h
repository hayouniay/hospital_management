#pragma once
#include "models/MedicalRecord.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class MedicalRecordController : public QObject {
    Q_OBJECT
public:
    explicit MedicalRecordController(QObject* parent = nullptr);

    std::expected<MedicalRecord, QString>              create(MedicalRecord record);
    std::expected<MedicalRecord, QString>              findById(qint64 id);
    std::expected<std::vector<MedicalRecord>, QString> findAll(int limit=200, int offset=0);
    std::expected<std::vector<MedicalRecord>, QString> findByPatient(qint64 patientId);
    std::expected<std::vector<MedicalRecord>, QString> findByDoctor(qint64 doctorId);
    std::expected<std::vector<MedicalRecord>, QString> search(const QString& query);
    std::expected<MedicalRecord, QString>              update(const MedicalRecord& record);
    std::expected<void, QString>                       remove(qint64 id);

signals:
    void recordCreated(const MedicalRecord& r);
    void recordUpdated(const MedicalRecord& r);

private:
    DatabaseManager& m_db;
    const QString JOIN_SQL = R"(
        SELECT mr.*,
               p.first_name || ' ' || p.last_name AS patient_name,
               d.first_name || ' ' || d.last_name AS doctor_name
        FROM medical_records mr
        LEFT JOIN patients p ON mr.patient_id = p.id
        LEFT JOIN doctors d  ON mr.doctor_id = d.id
    )";
};

} // namespace HMS
