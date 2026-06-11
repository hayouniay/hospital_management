#include "StatCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace HMS {

StatCard::StatCard(const QString& icon, const QString& title,
                   const QString& value, const QString& color,
                   QWidget* parent)
    : QWidget(parent), m_color(color) {

    setMinimumSize(200, 110);
    setMaximumHeight(130);

    auto* main = new QVBoxLayout(this);
    main->setContentsMargins(18, 14, 18, 14);
    main->setSpacing(6);

    // Top row: icon + title
    auto* topRow = new QHBoxLayout;
    auto* iconLabel = new QLabel(icon, this);
    iconLabel->setStyleSheet("font-size: 22px;");
    auto* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(QString("font-size: 12px; font-weight: 600; color: %1;").arg(color));
    topRow->addWidget(iconLabel);
    topRow->addWidget(titleLabel);
    topRow->addStretch();
    main->addLayout(topRow);

    // Value
    m_valueLabel = new QLabel(value, this);
    m_valueLabel->setStyleSheet("font-size: 30px; font-weight: 700; color: #1A202C;");
    main->addWidget(m_valueLabel);

    // Subtitle
    m_subLabel = new QLabel("", this);
    m_subLabel->setStyleSheet("font-size: 11px; color: #718096;");
    main->addWidget(m_subLabel);

    setStyleSheet(QString(R"(
        StatCard {
            background-color: #FFFFFF;
            border-radius: 12px;
            border-left: 4px solid %1;
            border: 1px solid #E2E8F0;
            border-left: 4px solid %1;
        }
    )").arg(color));
}

void StatCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

void StatCard::setSubtitle(const QString& sub) {
    m_subLabel->setText(sub);
}

} // namespace HMS
