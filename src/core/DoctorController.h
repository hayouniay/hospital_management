#pragma once
#include "models/Doctor.h"
#include "models/Department.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class DoctorController : public QObject {
    Q_OBJECT
public:
    explicit DoctorController(QObject* parent = nullptr);

    std::expected<Doctor, QString>              create(Doctor doctor);
    std::expected<Doctor, QString>              findById(qint64 id);
    std::expected<std::vector<Doctor>, QString> findAll(int limit=200, int offset=0);
    std::expected<std::vector<Doctor>, QString> findByDepartment(qint64 deptId);
    std::expected<std::vector<Doctor>, QString> search(const QString& query);
    std::expected<Doctor, QString>              update(const Doctor& doctor);
    std::expected<void, QString>                remove(qint64 id);
    std::expected<int, QString>                 countActive();

    // Department helpers
    std::expected<std::vector<Department>, QString> allDepartments();
    std::expected<Department, QString>              findDepartmentById(qint64 id);

signals:
    void doctorCreated(const Doctor& d);
    void doctorUpdated(const Doctor& d);
    void doctorDeleted(qint64 id);

private:
    DatabaseManager& m_db;
    QString generateEmployeeId();
};

} // namespace HMS
