#include "Validator.h"
#include <QRegularExpression>

namespace HMS {

std::expected<void, QString> Validator::notEmpty(const QString& v, const QString& field) {
    if (v.trimmed().isEmpty())
        return std::unexpected(field + " cannot be empty.");
    return {};
}

std::expected<void, QString> Validator::email(const QString& v) {
    if (v.isEmpty()) return {};   // optional by default
    if (!isValidEmail(v))
        return std::unexpected("Invalid email address: " + v);
    return {};
}

std::expected<void, QString> Validator::phone(const QString& v) {
    if (v.isEmpty()) return {};
    if (!isValidPhone(v))
        return std::unexpected("Invalid phone number: " + v);
    return {};
}

std::expected<void, QString> Validator::minLength(const QString& v, int min, const QString& field) {
    if (v.trimmed().length() < min)
        return std::unexpected(QString("%1 must be at least %2 characters.").arg(field).arg(min));
    return {};
}

std::expected<void, QString> Validator::positiveAmount(double v, const QString& field) {
    if (v < 0)
        return std::unexpected(field + " cannot be negative.");
    return {};
}

bool Validator::isValidEmail(const QString& v) {
    static QRegularExpression rx(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)");
    return rx.match(v).hasMatch();
}

bool Validator::isValidPhone(const QString& v) {
    static QRegularExpression rx(R"(^[\+\d\s\-\(\)]{6,20}$)");
    return rx.match(v).hasMatch();
}

QString Validator::sanitize(const QString& v) {
    return v.trimmed().simplified();
}

} // namespace HMS
