#pragma once
#include <QString>
#include <expected>

namespace HMS {

class Validator {
public:
    static std::expected<void, QString> notEmpty(const QString& v, const QString& fieldName);
    static std::expected<void, QString> email(const QString& v);
    static std::expected<void, QString> phone(const QString& v);
    static std::expected<void, QString> minLength(const QString& v, int min, const QString& field);
    static std::expected<void, QString> positiveAmount(double v, const QString& field);
    static bool isValidEmail(const QString& v);
    static bool isValidPhone(const QString& v);
    static QString sanitize(const QString& v);   // trim + collapse spaces
};

} // namespace HMS
