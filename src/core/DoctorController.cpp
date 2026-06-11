#include "DoctorController.h"
#include <QDate>
#include <QRandomGenerator>

namespace HMS {

DoctorController::DoctorController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Doctor, QString> DoctorController::create(Doctor d) {
    d.employeeId = generateEmployeeId();
    auto r = m_db.execute(R"(
        INSERT INTO doctors
            (employee_id, first_name, last_name, specialization,
             department_id, phone, email, license_number, schedule, status)
        VALUES (?,?,?,?,?,?,?,?,?,?))",
        {d.employeeId, d.firstName, d.lastName, d.specialization,
         d.departmentId > 0 ? QVariant(d.departmentId) : QVariant(),
         d.phone, d.email, d.licenseNumber, d.schedule, d.status});
    if (!r) return std::unexpected(r.error());
    d.id = m_db.lastInsertId();
    emit doctorCreated(d);
    return d;
}

std::expected<Doctor, QString> DoctorController::findById(qint64 id) {
    auto r = m_db.execute("SELECT * FROM doctors WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Doctor not found");
    return Doctor::fromQuery(*r);
}

std::expected<std::vector<Doctor>, QString> DoctorController::findAll(int limit, int offset) {
    auto r = m_db.execute(
        "SELECT * FROM doctors ORDER BY last_name, first_name LIMIT ? OFFSET ?",
        {limit, offset});
    if (!r) return std::unexpected(r.error());
    std::vector<Doctor> list;
    while ((*r).next()) list.push_back(Doctor::fromQuery(*r));
    return list;
}

std::expected<std::vector<Doctor>, QString> DoctorController::findByDepartment(qint64 deptId) {
    auto r = m_db.execute(
        "SELECT * FROM doctors WHERE department_id=? AND status='active' ORDER BY last_name",
        {deptId});
    if (!r) return std::unexpected(r.error());
    std::vector<Doctor> list;
    while ((*r).next()) list.push_back(Doctor::fromQuery(*r));
    return list;
}

std::expected<std::vector<Doctor>, QString> DoctorController::search(const QString& query) {
    const QString like = "%" + query + "%";
    auto r = m_db.execute(R"(
        SELECT * FROM doctors
        WHERE first_name LIKE ? OR last_name LIKE ?
           OR specialization LIKE ? OR employee_id LIKE ? OR email LIKE ?
        ORDER BY last_name LIMIT 50)",
        {like, like, like, like, like});
    if (!r) return std::unexpected(r.error());
    std::vector<Doctor> list;
    while ((*r).next()) list.push_back(Doctor::fromQuery(*r));
    return list;
}

std::expected<Doctor, QString> DoctorController::update(const Doctor& d) {
    auto r = m_db.execute(R"(
        UPDATE doctors SET
            first_name=?, last_name=?, specialization=?, department_id=?,
            phone=?, email=?, license_number=?, schedule=?, status=?,
            updated_at=datetime('now')
        WHERE id=?)",
        {d.firstName, d.lastName, d.specialization,
         d.departmentId > 0 ? QVariant(d.departmentId) : QVariant(),
         d.phone, d.email, d.licenseNumber, d.schedule, d.status, d.id});
    if (!r) return std::unexpected(r.error());
    emit doctorUpdated(d);
    return d;
}

std::expected<void, QString> DoctorController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM doctors WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    emit doctorDeleted(id);
    return {};
}

std::expected<int, QString> DoctorController::countActive() {
    auto r = m_db.execute("SELECT COUNT(*) FROM doctors WHERE status='active'");
    if (!r) return std::unexpected(r.error());
    (*r).next(); return (*r).value(0).toInt();
}

std::expected<std::vector<Department>, QString> DoctorController::allDepartments() {
    auto r = m_db.execute("SELECT * FROM departments WHERE status='active' ORDER BY name");
    if (!r) return std::unexpected(r.error());
    std::vector<Department> list;
    while ((*r).next()) list.push_back(Department::fromQuery(*r));
    return list;
}

std::expected<Department, QString> DoctorController::findDepartmentById(qint64 id) {
    auto r = m_db.execute("SELECT * FROM departments WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Department not found");
    return Department::fromQuery(*r);
}

QString DoctorController::generateEmployeeId() {
    quint32 rand = QRandomGenerator::global()->bounded(10000);
    return QString("DR-%1-%2")
        .arg(QDate::currentDate().toString("yyyy"))
        .arg(rand, 4, 10, QChar('0'));
}

} // namespace HMS
