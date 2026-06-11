#include "DatabaseManager.h"
#include <QtSql/QSqlRecord>
#include <QDebug>
#include <QDir>

namespace HMS {

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager() {
    close();
}

DbVoidResult DatabaseManager::initialize(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "HMS_Connection");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        return std::unexpected(m_db.lastError().text());
    }

    // Enable WAL mode and foreign keys
    execute("PRAGMA journal_mode=WAL");
    execute("PRAGMA foreign_keys=ON");
    execute("PRAGMA synchronous=NORMAL");

    return createSchema();
}

void DatabaseManager::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase("HMS_Connection");
}

bool DatabaseManager::isOpen() const {
    return m_db.isOpen();
}

DbResult DatabaseManager::execute(const QString& sql) {
    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        return std::unexpected(query.lastError().text());
    }
    m_lastInsertId = query.lastInsertId().toLongLong();
    m_rowsAffected = query.numRowsAffected();
    return query;
}

DbResult DatabaseManager::execute(const QString& sql, const std::vector<QVariant>& params) {
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const auto& param : params) {
        query.addBindValue(param);
    }
    if (!query.exec()) {
        return std::unexpected(query.lastError().text());
    }
    m_lastInsertId = query.lastInsertId().toLongLong();
    m_rowsAffected = query.numRowsAffected();
    return query;
}

DbVoidResult DatabaseManager::beginTransaction() {
    if (!m_db.transaction()) {
        return std::unexpected(m_db.lastError().text());
    }
    return {};
}

DbVoidResult DatabaseManager::commit() {
    if (!m_db.commit()) {
        return std::unexpected(m_db.lastError().text());
    }
    return {};
}

DbVoidResult DatabaseManager::rollback() {
    if (!m_db.rollback()) {
        return std::unexpected(m_db.lastError().text());
    }
    return {};
}

DbVoidResult DatabaseManager::createSchema() {
    auto results = {
        createTablesDepartment(),
        createTablesPatient(),
        createTablesDoctor(),
        createTablesAppointment(),
        createTablesBilling(),
        createTablesMedicalRecord(),
        createTablesRegistration(),
        createTablesAudit()
    };

    for (const auto& r : results) {
        if (!r) return r;
    }
    return {};
}

DbVoidResult DatabaseManager::createTablesDepartment() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS departments (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT NOT NULL UNIQUE,
            head_doctor INTEGER,
            floor       INTEGER,
            capacity    INTEGER DEFAULT 20,
            phone       TEXT,
            status      TEXT DEFAULT 'active' CHECK(status IN ('active','inactive')),
            created_at  TEXT DEFAULT (datetime('now')),
            updated_at  TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesPatient() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS patients (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_uid     TEXT NOT NULL UNIQUE,
            first_name      TEXT NOT NULL,
            last_name       TEXT NOT NULL,
            date_of_birth   TEXT NOT NULL,
            gender          TEXT CHECK(gender IN ('Male','Female','Other')),
            blood_type      TEXT,
            phone           TEXT,
            email           TEXT,
            address         TEXT,
            emergency_contact TEXT,
            emergency_phone TEXT,
            insurance_id    TEXT,
            status          TEXT DEFAULT 'active' CHECK(status IN ('active','discharged','deceased')),
            qr_code         TEXT,
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesDoctor() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS doctors (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            employee_id     TEXT NOT NULL UNIQUE,
            first_name      TEXT NOT NULL,
            last_name       TEXT NOT NULL,
            specialization  TEXT NOT NULL,
            department_id   INTEGER REFERENCES departments(id),
            phone           TEXT,
            email           TEXT NOT NULL,
            license_number  TEXT UNIQUE,
            schedule        TEXT,
            status          TEXT DEFAULT 'active' CHECK(status IN ('active','inactive','on_leave')),
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesAppointment() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS appointments (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id      INTEGER NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
            doctor_id       INTEGER NOT NULL REFERENCES doctors(id),
            department_id   INTEGER REFERENCES departments(id),
            appointment_date TEXT NOT NULL,
            appointment_time TEXT NOT NULL,
            duration_min    INTEGER DEFAULT 30,
            type            TEXT DEFAULT 'consultation' CHECK(type IN ('consultation','follow-up','emergency','surgery','lab')),
            status          TEXT DEFAULT 'scheduled' CHECK(status IN ('scheduled','confirmed','in-progress','completed','cancelled','no-show')),
            notes           TEXT,
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesBilling() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS billings (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            invoice_number  TEXT NOT NULL UNIQUE,
            patient_id      INTEGER NOT NULL REFERENCES patients(id),
            appointment_id  INTEGER REFERENCES appointments(id),
            amount          REAL NOT NULL DEFAULT 0,
            discount        REAL DEFAULT 0,
            tax             REAL DEFAULT 0,
            total           REAL NOT NULL DEFAULT 0,
            paid_amount     REAL DEFAULT 0,
            payment_method  TEXT CHECK(payment_method IN ('cash','card','insurance','online','pending')),
            status          TEXT DEFAULT 'pending' CHECK(status IN ('pending','partial','paid','cancelled','refunded')),
            due_date        TEXT,
            notes           TEXT,
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesMedicalRecord() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS medical_records (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id      INTEGER NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
            doctor_id       INTEGER NOT NULL REFERENCES doctors(id),
            appointment_id  INTEGER REFERENCES appointments(id),
            diagnosis       TEXT NOT NULL,
            symptoms        TEXT,
            prescription    TEXT,
            lab_results     TEXT,
            notes           TEXT,
            follow_up_date  TEXT,
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesRegistration() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS registrations (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_id      INTEGER NOT NULL REFERENCES patients(id),
            department_id   INTEGER REFERENCES departments(id),
            admission_date  TEXT DEFAULT (datetime('now')),
            discharge_date  TEXT,
            bed_number      TEXT,
            ward            TEXT,
            type            TEXT DEFAULT 'outpatient' CHECK(type IN ('inpatient','outpatient','emergency','icu')),
            admitting_doctor INTEGER REFERENCES doctors(id),
            reason          TEXT,
            status          TEXT DEFAULT 'admitted' CHECK(status IN ('admitted','discharged','transferred','pending')),
            created_at      TEXT DEFAULT (datetime('now')),
            updated_at      TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return {};
}

DbVoidResult DatabaseManager::createTablesAudit() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS audit_log (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            table_name  TEXT NOT NULL,
            record_id   INTEGER NOT NULL,
            action      TEXT CHECK(action IN ('INSERT','UPDATE','DELETE')),
            old_data    TEXT,
            new_data    TEXT,
            user        TEXT DEFAULT 'system',
            created_at  TEXT DEFAULT (datetime('now'))
        )
    )";
    if (auto r = execute(sql); !r) return std::unexpected(r.error());
    return createAuditTriggers();
}
DbVoidResult DatabaseManager::createAuditTriggers() {
    // SQLite triggers to auto-populate audit_log on patients, doctors, appointments, billings
    const QStringList triggers = {
        // patients
        R"(CREATE TRIGGER IF NOT EXISTS trg_patients_insert AFTER INSERT ON patients
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('patients',NEW.id,'INSERT',
             'uid='||NEW.patient_uid||';name='||NEW.first_name||' '||NEW.last_name); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_patients_update AFTER UPDATE ON patients
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data,new_data)
           VALUES('patients',NEW.id,'UPDATE',
             'status='||OLD.status,'status='||NEW.status); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_patients_delete BEFORE DELETE ON patients
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data)
           VALUES('patients',OLD.id,'DELETE',
             'uid='||OLD.patient_uid||';name='||OLD.first_name||' '||OLD.last_name); END)",

        // doctors
        R"(CREATE TRIGGER IF NOT EXISTS trg_doctors_insert AFTER INSERT ON doctors
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('doctors',NEW.id,'INSERT',
             'id='||NEW.employee_id||';name='||NEW.first_name||' '||NEW.last_name); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_doctors_delete BEFORE DELETE ON doctors
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data)
           VALUES('doctors',OLD.id,'DELETE',
             'id='||OLD.employee_id||';name='||OLD.first_name||' '||OLD.last_name); END)",

        // appointments
        R"(CREATE TRIGGER IF NOT EXISTS trg_appt_insert AFTER INSERT ON appointments
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('appointments',NEW.id,'INSERT',
             'patient='||NEW.patient_id||';doctor='||NEW.doctor_id||';date='||NEW.appointment_date); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_appt_update AFTER UPDATE ON appointments
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data,new_data)
           VALUES('appointments',NEW.id,'UPDATE',
             'status='||OLD.status,'status='||NEW.status); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_appt_delete BEFORE DELETE ON appointments
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data)
           VALUES('appointments',OLD.id,'DELETE',
             'patient='||OLD.patient_id||';date='||OLD.appointment_date); END)",

        // billings
        R"(CREATE TRIGGER IF NOT EXISTS trg_billing_insert AFTER INSERT ON billings
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('billings',NEW.id,'INSERT',
             'invoice='||NEW.invoice_number||';total='||NEW.total); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_billing_update AFTER UPDATE ON billings
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data,new_data)
           VALUES('billings',NEW.id,'UPDATE',
             'status='||OLD.status||';paid='||OLD.paid_amount,
             'status='||NEW.status||';paid='||NEW.paid_amount); END)",

        // medical_records
        R"(CREATE TRIGGER IF NOT EXISTS trg_medrecord_insert AFTER INSERT ON medical_records
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('medical_records',NEW.id,'INSERT',
             'patient='||NEW.patient_id||';doctor='||NEW.doctor_id); END)",

        // registrations
        R"(CREATE TRIGGER IF NOT EXISTS trg_registration_insert AFTER INSERT ON registrations
           BEGIN INSERT INTO audit_log(table_name,record_id,action,new_data)
           VALUES('registrations',NEW.id,'INSERT',
             'patient='||NEW.patient_id||';type='||NEW.type||';status='||NEW.status); END)",

        R"(CREATE TRIGGER IF NOT EXISTS trg_registration_update AFTER UPDATE ON registrations
           BEGIN INSERT INTO audit_log(table_name,record_id,action,old_data,new_data)
           VALUES('registrations',NEW.id,'UPDATE',
             'status='||OLD.status,'status='||NEW.status); END)",
    };

    for (const auto& sql : triggers) {
        if (auto r = execute(sql); !r) {
            // Non-fatal: triggers may already exist or SQLite version may vary
            qWarning() << "Audit trigger warning:" << r.error();
        }
    }
    return {};
}


qint64 DatabaseManager::lastInsertId() const { return m_lastInsertId; }
int DatabaseManager::rowsAffected() const { return m_rowsAffected; }
QString DatabaseManager::lastError() const { return m_db.lastError().text(); }

} // namespace HMS
