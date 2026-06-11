#pragma once

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QString>
#include <QVariant>
#include <expected>
#include <string>
#include <vector>
#include <functional>

namespace HMS {

// C++23 expected for error handling
using DbResult = std::expected<QSqlQuery, QString>;
using DbVoidResult = std::expected<void, QString>;

class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Lifecycle
    DbVoidResult initialize(const QString& dbPath = "hospital.db");
    void close();
    bool isOpen() const;

    // Query execution
    DbResult execute(const QString& sql);
    DbResult execute(const QString& sql, const std::vector<QVariant>& params);

    // Transaction support
    DbVoidResult beginTransaction();
    DbVoidResult commit();
    DbVoidResult rollback();

    // Schema management
    DbVoidResult createSchema();
    DbVoidResult runMigrations();

    // Utility
    qint64 lastInsertId() const;
    int rowsAffected() const;
    QString lastError() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    DbVoidResult createTablesPatient();
    DbVoidResult createTablesDoctor();
    DbVoidResult createTablesAppointment();
    DbVoidResult createTablesBilling();
    DbVoidResult createTablesMedicalRecord();
    DbVoidResult createTablesDepartment();
    DbVoidResult createTablesRegistration();
    DbVoidResult createTablesAudit();
    DbVoidResult createAuditTriggers();

    QSqlDatabase m_db;
    qint64 m_lastInsertId{0};
    int m_rowsAffected{0};
};

} // namespace HMS
