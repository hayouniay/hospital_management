#include "Department.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap Department::toMap() const {
    return {{"id",id},{"name",name},{"head_doctor",headDoctor},
            {"floor",floor},{"capacity",capacity},{"phone",phone},{"status",status}};
}
Department Department::fromQuery(const QSqlQuery& q) {
    Department d;
    d.id       = q.value("id").toLongLong();
    d.name     = q.value("name").toString();
    d.headDoctor = q.value("head_doctor").toLongLong();
    d.floor    = q.value("floor").toInt();
    d.capacity = q.value("capacity").toInt();
    d.phone    = q.value("phone").toString();
    d.status   = q.value("status").toString();
    return d;
}
} // namespace HMS
