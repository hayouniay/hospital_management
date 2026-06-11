#pragma once

#include "models/Patient.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <optional>
#include <expected>

namespace HMS {

class PatientController : public QObject {
    Q_OBJECT
public:
    explicit PatientController(QObject* parent = nullptr);

    // CRUD
    std::expected<Patient, QString> create(Patient patient);
    std::expected<Patient, QString> findById(qint64 id);
    std::expected<Patient, QString> findByUid(const QString& uid);
    std::expected<std::vector<Patient>, QString> findAll(int limit = 100, int offset = 0);
    std::expected<std::vector<Patient>, QString> search(const QString& query);
    std::expected<Patient, QString> update(const Patient& patient);
    std::expected<void, QString>    remove(qint64 id);

    // Stats
    std::expected<int, QString> countAll();
    std::expected<int, QString> countActive();
    std::expected<int, QString> countAdmitted();

    // QR
    std::expected<QString, QString> generateQRCode(const Patient& patient);

signals:
    void patientCreated(const Patient& p);
    void patientUpdated(const Patient& p);
    void patientDeleted(qint64 id);

private:
    QString generateUid();
    DatabaseManager& m_db;
};

} // namespace HMS
