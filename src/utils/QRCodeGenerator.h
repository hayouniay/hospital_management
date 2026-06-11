#pragma once
#include <QString>
#include <expected>

namespace HMS {

// Minimal QR data encoder - produces a data string for QR libraries
// In production, link to libqrencode or ZXing
class QRCodeGenerator {
public:
    // Returns base64-encoded QR payload string
    static std::expected<QString, QString> encode(const QString& data);

    // Returns an SVG string representation
    static std::expected<QString, QString> toSvg(const QString& data);
};

} // namespace HMS
