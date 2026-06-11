#include "SidebarMenu.h"
#include <QLabel>
#include <QStyle>
#include <QApplication>
#include <QScrollArea>
#include <QFrame>

namespace HMS {

SidebarMenu::SidebarMenu(QWidget* parent) : QWidget(parent) {
    setFixedWidth(224);
    setObjectName("sidebar");

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->setSpacing(0);

    // Logo area
    auto* logoWidget = new QWidget(this);
    logoWidget->setObjectName("sidebarLogo");
    logoWidget->setFixedHeight(68);
    auto* ll = new QVBoxLayout(logoWidget);
    ll->setContentsMargins(16,10,16,10);
    auto* logoLabel = new QLabel("🏥  MediCare HMS", logoWidget);
    logoLabel->setObjectName("logoLabel");
    auto* versionLabel = new QLabel("v1.0  ·  Qt6 + SQLite", logoWidget);
    versionLabel->setObjectName("versionLabel");
    ll->addWidget(logoLabel);
    ll->addWidget(versionLabel);
    outerLayout->addWidget(logoWidget);

    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setObjectName("sidebarSep");
    outerLayout->addWidget(sep);

    // Scrollable menu area
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("background:transparent;");

    auto* menuWidget = new QWidget;
    menuWidget->setObjectName("sidebar");
    auto* menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->setContentsMargins(0,4,0,4);
    menuLayout->setSpacing(1);

    m_items = {
        {"📊","Dashboard"},
        {"👥","Patients"},
        {"🩺","Doctors"},
        {"📅","Appointments"},
        {"🏨","Registration"},
        {"💳","Billing"},
        {"📋","Medical Records"},
        {"🏢","Departments"},
        {"📈","Reports"},
        {"🕐","Patient Timeline"},
        {"🔍","Audit Log"},
    };

    // Section labels
    auto addSection = [&](const QString& title) {
        auto* lbl = new QLabel("  " + title, menuWidget);
        lbl->setObjectName("menuSectionLabel");
        lbl->setFixedHeight(28);
        menuLayout->addWidget(lbl);
    };

    addSection("MAIN MENU");
    for (int i = 0; i <= 8; ++i) {
        auto& item = m_items[i];
        item.button = new QPushButton(item.icon + "  " + item.label, menuWidget);
        item.button->setFixedHeight(42);
        item.button->setCursor(Qt::PointingHandCursor);
        item.button->setProperty("active", false);
        connect(item.button, &QPushButton::clicked, this, [this, i]() { selectItem(i); });
        menuLayout->addWidget(item.button);
    }

    addSection("ANALYTICS");
    for (int i = 9; i < static_cast<int>(m_items.size()); ++i) {
        auto& item = m_items[i];
        item.button = new QPushButton(item.icon + "  " + item.label, menuWidget);
        item.button->setFixedHeight(42);
        item.button->setCursor(Qt::PointingHandCursor);
        item.button->setProperty("active", false);
        connect(item.button, &QPushButton::clicked, this, [this, i]() { selectItem(i); });
        menuLayout->addWidget(item.button);
    }

    menuLayout->addStretch();
    scroll->setWidget(menuWidget);
    outerLayout->addWidget(scroll);

    // Bottom user area
    auto* bottomWidget = new QWidget(this);
    bottomWidget->setObjectName("sidebarBottom");
    bottomWidget->setFixedHeight(44);
    auto* bl = new QHBoxLayout(bottomWidget);
    bl->setContentsMargins(16,0,16,0);
    auto* userLabel = new QLabel("👤  Administrator", bottomWidget);
    userLabel->setObjectName("userLabel");
    bl->addWidget(userLabel);
    outerLayout->addWidget(bottomWidget);
}

void SidebarMenu::selectItem(int index) {
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_items.size())) {
        auto* btn = m_items[m_currentIndex].button;
        btn->setProperty("active", false);
        QApplication::style()->unpolish(btn);
        QApplication::style()->polish(btn);
        btn->update();
    }
    m_currentIndex = index;
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        auto* btn = m_items[index].button;
        btn->setProperty("active", true);
        QApplication::style()->unpolish(btn);
        QApplication::style()->polish(btn);
        btn->update();
    }
    emit itemSelected(index);
}

} // namespace HMS
