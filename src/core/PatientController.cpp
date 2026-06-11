#include "PatientController.h"
#include "utils/QRCodeGenerator.h"
#include <QDateTime>
#include <QRandomGenerator>

namespace HMS {

PatientController::PatientController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Patient, QString> PatientController::create(Patient patient) {
    patient.patientUid = generateUid();

    // Generate QR code data
    auto qrResult = generateQRCode(patient);
    if (qrResult) patient.qrCode = *qrResult;

    const QString sql = R"(
        INSERT INTO patients
            (patient_uid, first_name, last_name, date_of_birth, gender,
             blood_type, phone, email, address, emergency_contact,
             emergency_phone, insurance_id, status, qr_code)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)
    )";

    auto result = m_db.execute(sql, {
        patient.patientUid, patient.firstName, patient.lastName,
        patient.dateOfBirth.toString(Qt::ISODate), patient.gender,
        patient.bloodType, patient.phone, patient.email, patient.address,
        patient.emergencyContact, patient.emergencyPhone, patient.insuranceId,
        patient.status, patient.qrCode
    });

    if (!result) return std::unexpected(result.error());

    patient.id = m_db.lastInsertId();
    emit patientCreated(patient);
    return patient;
}

std::expected<Patient, QString> PatientController::findById(qint64 id) {
    auto result = m_db.execute(
        "SELECT * FROM patients WHERE id = ?", {id});
    if (!result) return std::unexpected(result.error());

    auto& q = *result;
    if (!q.next()) return std::unexpected("Patient not found");
    return Patient::fromQuery(q);
}

std::expected<Patient, QString> PatientController::findByUid(const QString& uid) {
    auto result = m_db.execute(
        "SELECT * FROM patients WHERE patient_uid = ?", {uid});
    if (!result) return std::unexpected(result.error());

    auto& q = *result;
    if (!q.next()) return std::unexpected("Patient not found");
    return Patient::fromQuery(q);
}

std::expected<std::vector<Patient>, QString> PatientController::findAll(int limit, int offset) {
    auto result = m_db.execute(
        "SELECT * FROM patients ORDER BY created_at DESC LIMIT ? OFFSET ?",
        {limit, offset});
    if (!result) return std::unexpected(result.error());

    std::vector<Patient> patients;
    auto& q = *result;
    while (q.next()) {
        patients.push_back(Patient::fromQuery(q));
    }
    return patients;
}

std::expected<std::vector<Patient>, QString> PatientController::search(const QString& query) {
    const QString like = "%" + query + "%";
    auto result = m_db.execute(
        R"(SELECT * FROM patients
           WHERE first_name LIKE ? OR last_name LIKE ?
              OR patient_uid LIKE ? OR phone LIKE ? OR email LIKE ?
           ORDER BY last_name, first_name LIMIT 50)",
        {like, like, like, like, like});

    if (!result) return std::unexpected(result.error());

    std::vector<Patient> patients;
    auto& q = *result;
    while (q.next()) patients.push_back(Patient::fromQuery(q));
    return patients;
}

std::expected<Patient, QString> PatientController::update(const Patient& patient) {
    const QString sql = R"(
        UPDATE patients SET
            first_name=?, last_name=?, date_of_birth=?, gender=?,
            blood_type=?, phone=?, email=?, address=?,
            emergency_contact=?, emergency_phone=?, insurance_id=?,
            status=?, updated_at=datetime('now')
        WHERE id=?
    )";

    auto result = m_db.execute(sql, {
        patient.firstName, patient.lastName,
        patient.dateOfBirth.toString(Qt::ISODate), patient.gender,
        patient.bloodType, patient.phone, patient.email, patient.address,
        patient.emergencyContact, patient.emergencyPhone, patient.insuranceId,
        patient.status, patient.id
    });

    if (!result) return std::unexpected(result.error());
    emit patientUpdated(patient);
    return patient;
}

std::expected<void, QString> PatientController::remove(qint64 id) {
    auto result = m_db.execute("DELETE FROM patients WHERE id=?", {id});
    if (!result) return std::unexpected(result.error());
    emit patientDeleted(id);
    return {};
}

std::expected<int, QString> PatientController::countAll() {
    auto r = m_db.execute("SELECT COUNT(*) FROM patients");
    if (!r) return std::unexpected(r.error());
    (*r).next();
    return (*r).value(0).toInt();
}

std::expected<int, QString> PatientController::countActive() {
    auto r = m_db.execute("SELECT COUNT(*) FROM patients WHERE status='active'");
    if (!r) return std::unexpected(r.error());
    (*r).next();
    return (*r).value(0).toInt();
}

std::expected<int, QString> PatientController::countAdmitted() {
    auto r = m_db.execute("SELECT COUNT(*) FROM registrations WHERE status='admitted'");
    if (!r) return std::unexpected(r.error());
    (*r).next();
    return (*r).value(0).toInt();
}

std::expected<QString, QString> PatientController::generateQRCode(const Patient& patient) {
    QString data = QString("HMS|%1|%2 %3|DOB:%4|BT:%5")
        .arg(patient.patientUid)
        .arg(patient.firstName, patient.lastName)
        .arg(patient.dateOfBirth.toString(Qt::ISODate))
        .arg(patient.bloodType);
    return QRCodeGenerator::encode(data);
}

QString PatientController::generateUid() {
    quint32 rand = QRandomGenerator::global()->bounded(10000);
    return QString("HMS-%1-%2")
        .arg(QDate::currentDate().toString("yyyyMMdd"))
        .arg(rand, 4, 10, QChar('0'));
}

} // namespace HMS
