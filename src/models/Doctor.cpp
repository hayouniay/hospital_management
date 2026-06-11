#include "Doctor.h"
#include <QtSql/QSqlQuery>
namespace HMS {
QVariantMap Doctor::toMap() const {
    return {{"id",id},{"employee_id",employeeId},{"first_name",firstName},
            {"last_name",lastName},{"specialization",specialization},
            {"department_id",departmentId},{"phone",phone},{"email",email},
            {"license_number",licenseNumber},{"schedule",schedule},{"status",status}};
}
Doctor Doctor::fromQuery(const QSqlQuery& q) {
    Doctor d;
    d.id             = q.value("id").toLongLong();
    d.employeeId     = q.value("employee_id").toString();
    d.firstName      = q.value("first_name").toString();
    d.lastName       = q.value("last_name").toString();
    d.specialization = q.value("specialization").toString();
    d.departmentId   = q.value("department_id").toLongLong();
    d.phone          = q.value("phone").toString();
    d.email          = q.value("email").toString();
    d.licenseNumber  = q.value("license_number").toString();
    d.schedule       = q.value("schedule").toString();
    d.status         = q.value("status").toString();
    return d;
}
} // namespace HMS
