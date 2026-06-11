#include "DepartmentController.h"

namespace HMS {

DepartmentController::DepartmentController(QObject* parent)
    : QObject(parent), m_db(DatabaseManager::instance()) {}

std::expected<Department, QString> DepartmentController::create(Department d) {
    auto r = m_db.execute(R"(
        INSERT INTO departments (name, head_doctor, floor, capacity, phone, status)
        VALUES (?,?,?,?,?,?))",
        {d.name,
         d.headDoctor > 0 ? QVariant(d.headDoctor) : QVariant(),
         d.floor, d.capacity, d.phone, d.status});
    if (!r) return std::unexpected(r.error());
    d.id = m_db.lastInsertId();
    emit departmentCreated(d);
    return d;
}

std::expected<Department, QString> DepartmentController::findById(qint64 id) {
    auto r = m_db.execute("SELECT * FROM departments WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Department not found");
    return Department::fromQuery(*r);
}

std::expected<std::vector<Department>, QString> DepartmentController::findAll() {
    auto r = m_db.execute("SELECT * FROM departments ORDER BY name");
    if (!r) return std::unexpected(r.error());
    std::vector<Department> list;
    while ((*r).next()) list.push_back(Department::fromQuery(*r));
    return list;
}

std::expected<std::vector<Department>, QString> DepartmentController::search(const QString& query) {
    const QString like = "%" + query + "%";
    auto r = m_db.execute("SELECT * FROM departments WHERE name LIKE ? ORDER BY name", {like});
    if (!r) return std::unexpected(r.error());
    std::vector<Department> list;
    while ((*r).next()) list.push_back(Department::fromQuery(*r));
    return list;
}

std::expected<Department, QString> DepartmentController::update(const Department& d) {
    auto r = m_db.execute(R"(
        UPDATE departments SET
            name=?, head_doctor=?, floor=?, capacity=?, phone=?, status=?,
            updated_at=datetime('now')
        WHERE id=?)",
        {d.name,
         d.headDoctor > 0 ? QVariant(d.headDoctor) : QVariant(),
         d.floor, d.capacity, d.phone, d.status, d.id});
    if (!r) return std::unexpected(r.error());
    emit departmentUpdated(d);
    return d;
}

std::expected<void, QString> DepartmentController::remove(qint64 id) {
    auto r = m_db.execute("DELETE FROM departments WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    emit departmentDeleted(id);
    return {};
}

std::expected<int, QString> DepartmentController::count() {
    auto r = m_db.execute("SELECT COUNT(*) FROM departments");
    if (!r) return std::unexpected(r.error());
    (*r).next(); return (*r).value(0).toInt();
}

} // namespace HMS
