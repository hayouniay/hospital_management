#pragma once
#include <QObject>
#include <QString>
#include <QApplication>

namespace HMS {

class ThemeManager : public QObject {
    Q_OBJECT
public:
    static ThemeManager& instance();
    void applyTheme(bool dark);
    bool isDark() const { return m_dark; }

    static QString lightStylesheet();
    static QString darkStylesheet();

signals:
    void themeChanged(bool dark);

private:
    ThemeManager() = default;
    bool m_dark{false};
};

} // namespace HMS
