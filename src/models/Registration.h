#pragma once
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Registration {
    qint64  id{0};
    qint64  patientId{0};
    qint64  departmentId{0};
    QDateTime admissionDate;
    QDateTime dischargeDate;
    QString bedNumber;
    QString ward;
    QString type{"outpatient"};
    qint64  admittingDoctor{0};
    QString reason;
    QString status{"admitted"};
    QDateTime createdAt;

    // Joined fields
    QString patientName;
    QString doctorName;
    QString departmentName;

    [[nodiscard]] QVariantMap toMap() const;
    static Registration fromQuery(const QSqlQuery& q);
};

} // namespace HMS
