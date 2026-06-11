#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

namespace HMS {

enum class ToastType { Success, Error, Warning, Info };

class Toast : public QWidget {
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)
public:
    explicit Toast(const QString& message, ToastType type, QWidget* parent);
    void show();

    float opacity() const { return m_opacity; }
    void setOpacity(float v) { m_opacity = v; setWindowOpacity(v); }

private:
    float m_opacity{1.0f};
    QTimer* m_timer{nullptr};
    QPropertyAnimation* m_anim{nullptr};
};

class ToastManager : public QObject {
    Q_OBJECT
public:
    static ToastManager& instance();
    void setParent(QWidget* parent) { m_parent = parent; }
    void success(const QString& msg);
    void error(const QString& msg);
    void warning(const QString& msg);
    void info(const QString& msg);

private:
    explicit ToastManager(QObject* parent = nullptr) : QObject(parent) {}
    void show(const QString& msg, ToastType type);
    void restack();

    QWidget*          m_parent{nullptr};
    QList<Toast*>     m_toasts;
};

} // namespace HMS
