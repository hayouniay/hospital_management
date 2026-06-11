#include "ToastNotification.h"
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>

namespace HMS {

Toast::Toast(const QString& message, ToastType type, QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedHeight(52);
    setMinimumWidth(280);
    setMaximumWidth(420);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 0, 14, 0);
    layout->setSpacing(10);

    // Icon
    QString icon, bg, border;
    switch (type) {
        case ToastType::Success: icon="✔"; bg="#F0FFF4"; border="#38A169"; break;
        case ToastType::Error:   icon="✘"; bg="#FFF5F5"; border="#E53E3E"; break;
        case ToastType::Warning: icon="⚠"; bg="#FFFAF0"; border="#D69E2E"; break;
        case ToastType::Info:    icon="ℹ"; bg="#EBF8FF"; border="#3182CE"; break;
    }

    setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 10px;
            border-left: 4px solid %2;
            border: 1px solid %2;
        }
    )").arg(bg, border));

    auto* iconLbl = new QLabel(icon, this);
    iconLbl->setStyleSheet(QString("color:%1;font-size:16px;font-weight:700;background:transparent;border:none;").arg(border));
    iconLbl->setFixedWidth(20);

    auto* msgLbl = new QLabel(message, this);
    msgLbl->setStyleSheet("color:#2D3748;font-size:13px;font-weight:500;background:transparent;border:none;");
    msgLbl->setWordWrap(false);
    msgLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* closeBtn = new QLabel("×", this);
    closeBtn->setStyleSheet("color:#A0AEC0;font-size:18px;cursor:pointer;background:transparent;border:none;");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->installEventFilter(this);

    layout->addWidget(iconLbl);
    layout->addWidget(msgLbl);
    layout->addStretch();
    layout->addWidget(closeBtn);

    adjustSize();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_anim = new QPropertyAnimation(this, "opacity");
        m_anim->setDuration(400);
        m_anim->setStartValue(1.0f);
        m_anim->setEndValue(0.0f);
        connect(m_anim, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
        m_anim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void Toast::show() {
    QWidget::show();
    m_timer->start(3500);
}

// ── ToastManager ─────────────────────────────────────────────────────────────

ToastManager& ToastManager::instance() {
    static ToastManager inst;
    return inst;
}

void ToastManager::success(const QString& msg) { show(msg, ToastType::Success); }
void ToastManager::error(const QString& msg)   { show(msg, ToastType::Error);   }
void ToastManager::warning(const QString& msg) { show(msg, ToastType::Warning); }
void ToastManager::info(const QString& msg)    { show(msg, ToastType::Info);    }

void ToastManager::show(const QString& msg, ToastType type) {
    if (!m_parent) return;

    auto* toast = new Toast(msg, type, m_parent);

    connect(toast, &QObject::destroyed, this, [this, toast]() {
        m_toasts.removeAll(toast);
        restack();
    });

    m_toasts.append(toast);
    restack();
    toast->show();
}

void ToastManager::restack() {
    if (!m_parent) return;
    const int rightMargin = 16;
    const int bottomStart = m_parent->height() - 70;
    const int gap         = 8;

    for (int i = 0; i < m_toasts.size(); ++i) {
        auto* t = m_toasts[m_toasts.size() - 1 - i];
        int x = m_parent->width() - t->width() - rightMargin;
        int y = bottomStart - i * (t->height() + gap);
        t->move(m_parent->mapToGlobal(QPoint(x, y)));
    }
}

} // namespace HMS
