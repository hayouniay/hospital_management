#pragma once
#include <QString>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QVariantMap>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Appointment {
    qint64  id{0};
    qint64  patientId{0};
    qint64  doctorId{0};
    qint64  departmentId{0};
    QDate   appointmentDate;
    QTime   appointmentTime;
    int     durationMin{30};
    QString type{"consultation"};
    QString status{"scheduled"};
    QString notes;
    QDateTime createdAt;
    QDateTime updatedAt;

    // Joined fields (from queries)
    QString patientName;
    QString doctorName;
    QString departmentName;

    [[nodiscard]] QVariantMap toMap() const;
    static Appointment fromQuery(const QSqlQuery& q);
};

} // namespace HMS
