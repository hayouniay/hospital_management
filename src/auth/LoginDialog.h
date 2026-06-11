#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "auth/AuthController.h"

namespace HMS {

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);
    User loggedInUser() const { return m_user; }

private slots:
    void onLogin();
    void onTogglePassword();

private:
    void setupUi();
    void setError(const QString& msg);
    void clearError();

    QLineEdit*   m_usernameEdit{nullptr};
    QLineEdit*   m_passwordEdit{nullptr};
    QPushButton* m_loginBtn{nullptr};
    QPushButton* m_eyeBtn{nullptr};
    QLabel*      m_errorLabel{nullptr};
    QLabel*      m_capsLockLabel{nullptr};

    User m_user;
};

} // namespace HMS
