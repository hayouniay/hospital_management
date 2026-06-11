#pragma once
#include <QWidget>
#include <QLabel>
#include <QString>

namespace HMS {

class StatCard : public QWidget {
    Q_OBJECT
public:
    StatCard(const QString& icon, const QString& title,
             const QString& value, const QString& color,
             QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setSubtitle(const QString& sub);

private:
    QLabel* m_valueLabel{nullptr};
    QLabel* m_subLabel{nullptr};
    QString m_color;
};

} // namespace HMS
