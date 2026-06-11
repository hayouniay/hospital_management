#pragma once
#include "models/Billing.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class BillingController : public QObject {
    Q_OBJECT
public:
    explicit BillingController(QObject* parent = nullptr);

    std::expected<Billing, QString>             create(Billing billing);
    std::expected<Billing, QString>             findById(qint64 id);
    std::expected<std::vector<Billing>, QString> findAll(int limit=100, int offset=0);
    std::expected<std::vector<Billing>, QString> findByPatient(qint64 patientId);
    std::expected<Billing, QString>             update(const Billing& billing);
    std::expected<void, QString>                recordPayment(qint64 id, double amount, const QString& method);
    std::expected<void, QString>                remove(qint64 id);
    std::expected<int, QString>                 countByStatus(const QString& status);
    std::expected<double, QString>              totalRevenue(const QDate& from, const QDate& to);

signals:
    void billingCreated(const Billing& b);
    void paymentRecorded(qint64 id, double amount);

private:
    DatabaseManager& m_db;
    QString generateInvoiceNumber();

    const QString JOIN_SQL = R"(
        SELECT b.*,
               p.first_name || ' ' || p.last_name AS patient_name
        FROM billings b
        LEFT JOIN patients p ON b.patient_id = p.id
    )";
};

} // namespace HMS
