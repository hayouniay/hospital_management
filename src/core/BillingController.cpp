#include "BillingController.h"
#include <QDateTime>

namespace HMS {

BillingController::BillingController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Billing, QString> BillingController::create(Billing b) {
    b.invoiceNumber = generateInvoiceNumber();
    b.total = b.amount - b.discount + b.tax;

    auto r = m_db.execute(R"(
        INSERT INTO billings
            (invoice_number,patient_id,appointment_id,amount,discount,
             tax,total,paid_amount,payment_method,status,due_date,notes)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?))",
        {b.invoiceNumber, b.patientId, b.appointmentId, b.amount, b.discount,
         b.tax, b.total, b.paidAmount, b.paymentMethod, b.status,
         b.dueDate.toString(Qt::ISODate), b.notes});

    if (!r) return std::unexpected(r.error());
    b.id = m_db.lastInsertId();
    emit billingCreated(b);
    return b;
}

std::expected<Billing, QString> BillingController::findById(qint64 id) {
    auto r = m_db.execute(JOIN_SQL + " WHERE b.id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Invoice not found");
    return Billing::fromQuery(*r);
}

std::expected<std::vector<Billing>, QString> BillingController::findAll(int limit, int offset) {
    auto r = m_db.execute(JOIN_SQL + " ORDER BY b.created_at DESC LIMIT ? OFFSET ?", {limit, offset});
    if (!r) return std::unexpected(r.error());
    std::vector<Billing> list;
    while ((*r).next()) list.push_back(Billing::fromQuery(*r));
    return list;
}

std::expected<std::vector<Billing>, QString> BillingController::findByPatient(qint64 patientId) {
    auto r = m_db.execute(JOIN_SQL + " WHERE b.patient_id=? ORDER BY b.created_at DESC", {patientId});
    if (!r) return std::unexpected(r.error());
    std::vector<Billing> list;
    while ((*r).next()) list.push_back(Billing::fromQuery(*r));
    return list;
}

std::expected<Billing, QString> BillingController::update(const Billing& b) {
    auto r = m_db.execute(R"(
        UPDATE billings SET amount=?,discount=?,tax=?,total=?,paid_amount=?,
            payment_method=?,status=?,due_date=?,notes=?,updated_at=datetime('now')
        WHERE id=?)",
        {b.amount,b.discount,b.tax,b.total,b.paidAmount,
         b.paymentMethod,b.status,b.dueDate.toString(Qt::ISODate),b.notes,b.id});
    if (!r) return std::unexpected(r.error());
    return b;
}

std::expected<void, QString> BillingController::recordPayment(qint64 id, double amount, const QString& method) {
    auto billResult = findById(id);
    if (!billResult) return std::unexpected(billResult.error());

    auto& bill = *billResult;
    double newPaid = bill.paidAmount + amount;
    QString newStatus = (newPaid >= bill.total) ? "paid"
                      : (newPaid > 0) ? "partial" : "pending";

    auto r = m_db.execute(R"(
        UPDATE billings SET paid_amount=?,payment_method=?,status=?,updated_at=datetime('now')
        WHERE id=?)", {newPaid, method, newStatus, id});
    if (!r) return std::unexpected(r.error());
    emit paymentRecorded(id, amount);
    return {};
}

std::expected<void, QString> BillingController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM billings WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    return {};
}

std::expected<int, QString> BillingController::countByStatus(const QString& status) {
    auto r = m_db.execute("SELECT COUNT(*) FROM billings WHERE status=?", {status});
    if (!r) return std::unexpected(r.error());
    (*r).next(); return (*r).value(0).toInt();
}

std::expected<double, QString> BillingController::totalRevenue(const QDate& from, const QDate& to) {
    auto r = m_db.execute(
        "SELECT SUM(paid_amount) FROM billings WHERE status IN ('paid','partial') AND date(created_at) BETWEEN ? AND ?",
        {from.toString(Qt::ISODate), to.toString(Qt::ISODate)});
    if (!r) return std::unexpected(r.error());
    (*r).next(); return (*r).value(0).toDouble();
}

QString BillingController::generateInvoiceNumber() {
    return QString("INV-%1-%2")
        .arg(QDate::currentDate().toString("yyyyMM"))
        .arg(QTime::currentTime().msecsSinceStartOfDay());
}

} // namespace HMS
