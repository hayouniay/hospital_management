#include "Billing.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap Billing::toMap() const {
    return {{"id",id},{"invoice_number",invoiceNumber},{"patient_id",patientId},
            {"appointment_id",appointmentId},{"amount",amount},{"discount",discount},
            {"tax",tax},{"total",total},{"paid_amount",paidAmount},
            {"payment_method",paymentMethod},{"status",status},
            {"due_date",dueDate.toString(Qt::ISODate)},{"notes",notes}};
}
Billing Billing::fromQuery(const QSqlQuery& q) {
    Billing b;
    b.id            = q.value("id").toLongLong();
    b.invoiceNumber = q.value("invoice_number").toString();
    b.patientId     = q.value("patient_id").toLongLong();
    b.appointmentId = q.value("appointment_id").toLongLong();
    b.amount        = q.value("amount").toDouble();
    b.discount      = q.value("discount").toDouble();
    b.tax           = q.value("tax").toDouble();
    b.total         = q.value("total").toDouble();
    b.paidAmount    = q.value("paid_amount").toDouble();
    b.paymentMethod = q.value("payment_method").toString();
    b.status        = q.value("status").toString();
    b.dueDate       = QDate::fromString(q.value("due_date").toString(), Qt::ISODate);
    b.notes         = q.value("notes").toString();
    b.patientName   = q.value("patient_name").toString();
    return b;
}
} // namespace HMS
