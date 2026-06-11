#pragma once
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QVariantMap>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Billing {
    qint64  id{0};
    QString invoiceNumber;
    qint64  patientId{0};
    qint64  appointmentId{0};
    double  amount{0.0};
    double  discount{0.0};
    double  tax{0.0};
    double  total{0.0};
    double  paidAmount{0.0};
    QString paymentMethod{"pending"};
    QString status{"pending"};
    QDate   dueDate;
    QString notes;
    QDateTime createdAt;
    QDateTime updatedAt;

    // Joined
    QString patientName;

    [[nodiscard]] double balance() const { return total - paidAmount; }
    [[nodiscard]] QVariantMap toMap() const;
    static Billing fromQuery(const QSqlQuery& q);
};

} // namespace HMS
