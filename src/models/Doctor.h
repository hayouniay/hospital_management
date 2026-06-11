#pragma once
// Doctor.h
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <expected>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Doctor {
    qint64   id{0};
    QString  employeeId;
    QString  firstName;
    QString  lastName;
    QString  specialization;
    qint64   departmentId{0};
    QString  phone;
    QString  email;
    QString  licenseNumber;
    QString  schedule;     // JSON schedule blob
    QString  status{"active"};
    QDateTime createdAt;
    QDateTime updatedAt;

    [[nodiscard]] QString fullName() const { return firstName + " " + lastName; }
    [[nodiscard]] QVariantMap toMap() const;
    static Doctor fromQuery(const QSqlQuery& q);
};

} // namespace HMS
