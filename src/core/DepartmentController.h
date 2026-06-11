#pragma once
#include "models/Department.h"
#include "database/DatabaseManager.h"
#include <QObject>
#include <vector>
#include <expected>

namespace HMS {

class DepartmentController : public QObject {
    Q_OBJECT
public:
    explicit DepartmentController(QObject* parent = nullptr);

    std::expected<Department, QString>              create(Department dept);
    std::expected<Department, QString>              findById(qint64 id);
    std::expected<std::vector<Department>, QString> findAll();
    std::expected<std::vector<Department>, QString> search(const QString& query);
    std::expected<Department, QString>              update(const Department& dept);
    std::expected<void, QString>                    remove(qint64 id);
    std::expected<int, QString>                     count();

signals:
    void departmentCreated(const Department& d);
    void departmentUpdated(const Department& d);
    void departmentDeleted(qint64 id);

private:
    DatabaseManager& m_db;
};

} // namespace HMS
