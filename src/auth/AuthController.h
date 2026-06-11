#pragma once
#include "database/DatabaseManager.h"
#include <QString>
#include <QDateTime>
#include <QObject>
#include <expected>
#include <optional>

namespace HMS {

enum class UserRole { Admin, Doctor, Receptionist, Billing };

struct User {
    qint64   id{0};
    QString  username;
    QString  passwordHash;  // SHA-256 hex
    QString  fullName;
    QString  email;
    UserRole role{UserRole::Receptionist};
    bool     active{true};
    QDateTime lastLogin;
    QDateTime createdAt;

    [[nodiscard]] QString roleString() const {
        switch (role) {
            case UserRole::Admin:        return "admin";
            case UserRole::Doctor:       return "doctor";
            case UserRole::Receptionist: return "receptionist";
            case UserRole::Billing:      return "billing";
        }
        return "receptionist";
    }

    static UserRole roleFromString(const QString& s) {
        if (s == "admin")        return UserRole::Admin;
        if (s == "doctor")       return UserRole::Doctor;
        if (s == "billing")      return UserRole::Billing;
        return UserRole::Receptionist;
    }

    static User fromQuery(const QSqlQuery& q);
};

class AuthController : public QObject {
    Q_OBJECT
public:
    static AuthController& instance();

    // Auth
    std::expected<User, QString> login(const QString& username, const QString& password);
    void                         logout();
    bool                         isLoggedIn() const { return m_currentUser.has_value(); }
    std::optional<User>          currentUser() const { return m_currentUser; }
    bool                         hasRole(UserRole role) const;
    bool                         isAdmin() const { return hasRole(UserRole::Admin); }

    // User management (admin only)
    std::expected<User, QString>              createUser(const User& user, const QString& password);
    std::expected<std::vector<User>, QString> allUsers();
    std::expected<void, QString>              updateUser(const User& user);
    std::expected<void, QString>              changePassword(qint64 userId, const QString& newPassword);
    std::expected<void, QString>              deleteUser(qint64 id);

    // Schema
    std::expected<void, QString> ensureSchema();
    std::expected<void, QString> ensureDefaultAdmin();

    static QString hashPassword(const QString& password);

signals:
    void userLoggedIn(const User& user);
    void userLoggedOut();

private:
    AuthController();
    std::optional<User> m_currentUser;
    DatabaseManager&    m_db;
};

} // namespace HMS
