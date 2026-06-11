#include "AuthController.h"
#include <QCryptographicHash>
#include <QtSql/QSqlQuery>

namespace HMS {

AuthController& AuthController::instance() {
    static AuthController inst;
    return inst;
}

AuthController::AuthController() : m_db(DatabaseManager::instance()) {}

QString AuthController::hashPassword(const QString& password) {
    QByteArray data = password.toUtf8();
    // Salt with app-specific prefix for extra security
    data.prepend("HMS_SALT_2024_");
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

std::expected<void, QString> AuthController::ensureSchema() {
    const QString sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            username     TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            full_name    TEXT NOT NULL,
            email        TEXT,
            role         TEXT DEFAULT 'receptionist'
                         CHECK(role IN ('admin','doctor','receptionist','billing')),
            active       INTEGER DEFAULT 1,
            last_login   TEXT,
            created_at   TEXT DEFAULT (datetime('now'))
        )
    )";
    auto r = m_db.execute(sql);
    if (!r) return std::unexpected(r.error());
    return ensureDefaultAdmin();
}

std::expected<void, QString> AuthController::ensureDefaultAdmin() {
    // Check if any admin exists
    auto r = m_db.execute("SELECT COUNT(*) FROM users WHERE role='admin'");
    if (!r) return std::unexpected(r.error());
    (*r).next();
    if ((*r).value(0).toInt() > 0) return {}; // already exists

    // Create default admin
    const QString hash = hashPassword("admin123");
    auto r2 = m_db.execute(R"(
        INSERT INTO users (username, password_hash, full_name, email, role)
        VALUES ('admin', ?, 'System Administrator', 'admin@hospital.com', 'admin'))",
        {hash});
    if (!r2) return std::unexpected(r2.error());
    return {};
}

User User::fromQuery(const QSqlQuery& q) {
    User u;
    u.id           = q.value("id").toLongLong();
    u.username     = q.value("username").toString();
    u.passwordHash = q.value("password_hash").toString();
    u.fullName     = q.value("full_name").toString();
    u.email        = q.value("email").toString();
    u.role         = User::roleFromString(q.value("role").toString());
    u.active       = q.value("active").toBool();
    u.lastLogin    = QDateTime::fromString(q.value("last_login").toString(), Qt::ISODate);
    u.createdAt    = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    return u;
}

std::expected<User, QString> AuthController::login(const QString& username, const QString& password) {
    const QString hash = hashPassword(password);
    auto r = m_db.execute(
        "SELECT * FROM users WHERE username=? AND password_hash=? AND active=1",
        {username, hash});
    if (!r) return std::unexpected(r.error());
    if (!(*r).next()) return std::unexpected("Invalid username or password.");

    User user = User::fromQuery(*r);
    m_currentUser = user;

    // Update last login
    m_db.execute("UPDATE users SET last_login=datetime('now') WHERE id=?", {user.id});

    emit userLoggedIn(user);
    return user;
}

void AuthController::logout() {
    m_currentUser.reset();
    emit userLoggedOut();
}

bool AuthController::hasRole(UserRole role) const {
    if (!m_currentUser) return false;
    if (m_currentUser->role == UserRole::Admin) return true; // Admin has all roles
    return m_currentUser->role == role;
}

std::expected<User, QString> AuthController::createUser(const User& user, const QString& password) {
    const QString hash = hashPassword(password);
    auto r = m_db.execute(R"(
        INSERT INTO users (username, password_hash, full_name, email, role, active)
        VALUES (?,?,?,?,?,?))",
        {user.username, hash, user.fullName, user.email, user.roleString(), user.active ? 1 : 0});
    if (!r) return std::unexpected(r.error());
    User created = user;
    created.id = m_db.lastInsertId();
    return created;
}

std::expected<std::vector<User>, QString> AuthController::allUsers() {
    auto r = m_db.execute("SELECT * FROM users ORDER BY full_name");
    if (!r) return std::unexpected(r.error());
    std::vector<User> users;
    while ((*r).next()) users.push_back(User::fromQuery(*r));
    return users;
}

std::expected<void, QString> AuthController::updateUser(const User& user) {
    auto r = m_db.execute(R"(
        UPDATE users SET full_name=?, email=?, role=?, active=? WHERE id=?)",
        {user.fullName, user.email, user.roleString(), user.active ? 1 : 0, user.id});
    if (!r) return std::unexpected(r.error());
    return {};
}

std::expected<void, QString> AuthController::changePassword(qint64 userId, const QString& newPassword) {
    const QString hash = hashPassword(newPassword);
    auto r = m_db.execute("UPDATE users SET password_hash=? WHERE id=?", {hash, userId});
    if (!r) return std::unexpected(r.error());
    return {};
}

std::expected<void, QString> AuthController::deleteUser(qint64 id) {
    if (m_currentUser && m_currentUser->id == id)
        return std::unexpected("Cannot delete the currently logged-in user.");
    auto r = m_db.execute("DELETE FROM users WHERE id=?", {id});
    if (!r) return std::unexpected(r.error());
    return {};
}

} // namespace HMS
