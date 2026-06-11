#pragma once
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQuery>

namespace HMS {

struct Department {
    qint64  id{0};
    QString name;
    qint64  headDoctor{0};
    int     floor{1};
    int     capacity{20};
    QString phone;
    QString status{"active"};
    QDateTime createdAt;

    [[nodiscard]] QVariantMap toMap() const;
    static Department fromQuery(const QSqlQuery& q);
};

} // namespace HMS
