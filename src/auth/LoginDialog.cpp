#include "LoginDialog.h"
#include "settings/AppSettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QKeyEvent>
#include <QApplication>

namespace HMS {

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("MediCare HMS — Login");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setFixedSize(420, 520);
    setupUi();
}

void LoginDialog::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Left accent bar + header ──────────────────────────────────────────────
    auto* header = new QWidget(this);
    header->setFixedHeight(160);
    header->setStyleSheet(R"(
        background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1A365D,stop:1 #2B6CB0);
    )");
    auto* hl = new QVBoxLayout(header);
    hl->setContentsMargins(32, 24, 32, 24);
    hl->setSpacing(6);

    auto* iconLbl = new QLabel("🏥", header);
    iconLbl->setStyleSheet("font-size:36px;");
    auto* nameLbl = new QLabel(AppSettings::instance().hospitalName(), header);
    nameLbl->setStyleSheet("font-size:20px;font-weight:700;color:#FFFFFF;");
    auto* subLbl  = new QLabel("Hospital Management System", header);
    subLbl->setStyleSheet("font-size:12px;color:#90CDF4;");

    hl->addWidget(iconLbl);
    hl->addWidget(nameLbl);
    hl->addWidget(subLbl);
    root->addWidget(header);

    // ── Form area ─────────────────────────────────────────────────────────────
    auto* body = new QWidget(this);
    body->setStyleSheet("background:#FFFFFF;");
    auto* bl = new QVBoxLayout(body);
    bl->setContentsMargins(40, 32, 40, 32);
    bl->setSpacing(16);

    auto* signInLbl = new QLabel("Sign In", body);
    signInLbl->setStyleSheet("font-size:22px;font-weight:700;color:#1A202C;");
    bl->addWidget(signInLbl);

    auto* hintLbl = new QLabel("Default: admin / admin123", body);
    hintLbl->setStyleSheet("font-size:11px;color:#A0AEC0;margin-bottom:8px;");
    bl->addWidget(hintLbl);

    // Username
    auto* userLbl = new QLabel("Username", body);
    userLbl->setStyleSheet("font-size:12px;font-weight:600;color:#4A5568;");
    bl->addWidget(userLbl);

    m_usernameEdit = new QLineEdit(body);
    m_usernameEdit->setPlaceholderText("Enter your username");
    m_usernameEdit->setFixedHeight(44);
    m_usernameEdit->setStyleSheet(R"(
        QLineEdit { background:#F7FAFC;border:1.5px solid #CBD5E0;border-radius:8px;
                    padding:0 14px;font-size:14px;color:#2D3748; }
        QLineEdit:focus { border-color:#3182CE;background:#FFFFFF; }
    )");
    bl->addWidget(m_usernameEdit);

    // Password
    auto* passLbl = new QLabel("Password", body);
    passLbl->setStyleSheet("font-size:12px;font-weight:600;color:#4A5568;");
    bl->addWidget(passLbl);

    auto* passRow = new QHBoxLayout;
    passRow->setSpacing(0);
    m_passwordEdit = new QLineEdit(body);
    m_passwordEdit->setPlaceholderText("Enter your password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setFixedHeight(44);
    m_passwordEdit->setStyleSheet(R"(
        QLineEdit { background:#F7FAFC;border:1.5px solid #CBD5E0;border-radius:8px 0 0 8px;
                    padding:0 14px;font-size:14px;color:#2D3748; }
        QLineEdit:focus { border-color:#3182CE;background:#FFFFFF; }
    )");

    m_eyeBtn = new QPushButton("👁", body);
    m_eyeBtn->setFixedSize(44, 44);
    m_eyeBtn->setStyleSheet(R"(
        QPushButton { background:#F7FAFC;border:1.5px solid #CBD5E0;border-left:none;
                      border-radius:0 8px 8px 0;font-size:16px; }
        QPushButton:hover { background:#EDF2F7; }
    )");
    m_eyeBtn->setCursor(Qt::PointingHandCursor);

    passRow->addWidget(m_passwordEdit);
    passRow->addWidget(m_eyeBtn);
    bl->addLayout(passRow);

    // Error label
    m_errorLabel = new QLabel("", body);
    m_errorLabel->setStyleSheet("color:#E53E3E;font-size:12px;font-weight:600;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setVisible(false);
    bl->addWidget(m_errorLabel);

    // Login button
    m_loginBtn = new QPushButton("Sign In", body);
    m_loginBtn->setFixedHeight(48);
    m_loginBtn->setStyleSheet(R"(
        QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2B6CB0,stop:1 #3182CE);
                      color:white;border:none;border-radius:8px;font-size:15px;font-weight:700; }
        QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2C5282,stop:1 #2B6CB0); }
        QPushButton:pressed { background:#2C5282; }
    )");
    bl->addWidget(m_loginBtn);

    bl->addStretch();

    // Footer
    auto* footerLbl = new QLabel("© 2024 MediCare Hospital Management System", body);
    footerLbl->setAlignment(Qt::AlignCenter);
    footerLbl->setStyleSheet("font-size:10px;color:#CBD5E0;");
    bl->addWidget(footerLbl);

    root->addWidget(body);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_loginBtn,    &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_eyeBtn,      &QPushButton::clicked, this, &LoginDialog::onTogglePassword);
    connect(m_usernameEdit,&QLineEdit::returnPressed, this, &LoginDialog::onLogin);
    connect(m_passwordEdit,&QLineEdit::returnPressed, this, &LoginDialog::onLogin);

    // Pre-fill last user
    m_usernameEdit->setText(AppSettings::instance().lastUser());
    m_passwordEdit->setFocus();
}

void LoginDialog::onLogin() {
    clearError();
    const QString user = m_usernameEdit->text().trimmed();
    const QString pass = m_passwordEdit->text();

    if (user.isEmpty()) { setError("Please enter your username."); return; }
    if (pass.isEmpty()) { setError("Please enter your password."); return; }

    m_loginBtn->setEnabled(false);
    m_loginBtn->setText("Signing in...");

    auto result = AuthController::instance().login(user, pass);
    if (result) {
        m_user = *result;
        AppSettings::instance().setLastUser(user);
        accept();
    } else {
        setError(result.error());
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }

    m_loginBtn->setEnabled(true);
    m_loginBtn->setText("Sign In");
}

void LoginDialog::onTogglePassword() {
    bool hidden = m_passwordEdit->echoMode() == QLineEdit::Password;
    m_passwordEdit->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
    m_eyeBtn->setText(hidden ? "🔒" : "👁");
}

void LoginDialog::setError(const QString& msg) {
    m_errorLabel->setText("⚠  " + msg);
    m_errorLabel->setVisible(true);
}

void LoginDialog::clearError() {
    m_errorLabel->setVisible(false);
}

} // namespace HMS
