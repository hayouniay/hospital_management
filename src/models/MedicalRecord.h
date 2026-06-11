#pragma once
#include <QString>
#include <QDate>
#include <QDateTime>
#include <QVariantMap>
#include <QtSql/QSqlQuery>

namespace HMS {

struct MedicalRecord {
    qint64  id{0};
    qint64  patientId{0};
    qint64  doctorId{0};
    qint64  appointmentId{0};
    QString diagnosis;
    QString symptoms;
    QString prescription;
    QString labResults;
    QString notes;
    QDate   followUpDate;
    QDateTime createdAt;
    QDateTime updatedAt;

    // Joined
    QString patientName;
    QString doctorName;

    [[nodiscard]] QVariantMap toMap() const;
    static MedicalRecord fromQuery(const QSqlQuery& q);
};

} // namespace HMS
