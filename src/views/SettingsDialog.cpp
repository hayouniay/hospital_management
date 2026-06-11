#include "SettingsDialog.h"
#include "settings/AppSettings.h"
#include "settings/ThemeManager.h"
#include "auth/AuthController.h"
#include "widgets/ToastNotification.h"
#include "database/DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QProgressDialog>

namespace HMS {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumSize(700, 560);
    setupUi();
    loadSettings();
    loadUsers();
}

void SettingsDialog::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 16);

    m_tabs = new QTabWidget(this);

    auto* hospitalTab    = new QWidget; buildHospitalTab(hospitalTab);
    auto* appearanceTab  = new QWidget; buildAppearanceTab(appearanceTab);
    auto* securityTab    = new QWidget; buildSecurityTab(securityTab);
    auto* usersTab       = new QWidget; buildUsersTab(usersTab);
    auto* backupTab      = new QWidget; buildBackupTab(backupTab);

    m_tabs->addTab(hospitalTab,   "🏥  Hospital");
    m_tabs->addTab(appearanceTab, "🎨  Appearance");
    m_tabs->addTab(securityTab,   "🔐  Security");
    m_tabs->addTab(usersTab,      "👥  Users");
    m_tabs->addTab(backupTab,     "💾  Backup");

    layout->addWidget(m_tabs);

    auto* btns = new QHBoxLayout;
    btns->setContentsMargins(16, 0, 16, 0);
    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton("💾  Save Settings", this);
    btns->addStretch();
    btns->addWidget(cancelBtn);
    btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn,   &QPushButton::clicked, this, &SettingsDialog::onSave);
}

void SettingsDialog::buildHospitalTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 16, 24, 16);
    auto* group = new QGroupBox("Hospital Information", tab);
    auto* form  = new QFormLayout(group);

    m_nameEdit    = new QLineEdit(tab);
    m_addressEdit = new QLineEdit(tab);
    m_phoneEdit   = new QLineEdit(tab);
    m_emailEdit   = new QLineEdit(tab);

    auto* logoRow = new QHBoxLayout;
    m_logoEdit  = new QLineEdit(tab);
    m_logoEdit->setPlaceholderText("Path to logo image (optional)");
    auto* browseBtn = new QPushButton("Browse…", tab);
    browseBtn->setObjectName("secondaryBtn");
    browseBtn->setFixedWidth(80);
    logoRow->addWidget(m_logoEdit);
    logoRow->addWidget(browseBtn);

    form->addRow("Hospital Name *",  m_nameEdit);
    form->addRow("Address",          m_addressEdit);
    form->addRow("Phone",            m_phoneEdit);
    form->addRow("Email",            m_emailEdit);
    form->addRow("Logo",             logoRow);

    auto* note = new QLabel("These details appear on all generated PDF reports.", tab);
    note->setStyleSheet("color:#718096;font-size:11px;");

    layout->addWidget(group);
    layout->addWidget(note);
    layout->addStretch();

    connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseLogo);
}

void SettingsDialog::buildAppearanceTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 16, 24, 16);
    auto* group = new QGroupBox("Theme", tab);
    auto* gl = new QVBoxLayout(group);

    m_darkModeCheck = new QCheckBox("🌙  Dark Mode", group);
    m_darkModeCheck->setStyleSheet("font-size:14px;");
    gl->addWidget(m_darkModeCheck);

    auto* hint = new QLabel("Switch between light (default) and dark theme.\nThe change applies immediately.", group);
    hint->setStyleSheet("color:#718096;font-size:11px;margin-top:6px;");
    gl->addWidget(hint);

    layout->addWidget(group);
    layout->addStretch();

    connect(m_darkModeCheck, &QCheckBox::toggled, this, &SettingsDialog::onThemeToggle);
}

void SettingsDialog::buildSecurityTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 16, 24, 16);
    auto* group = new QGroupBox("Session Settings", tab);
    auto* form  = new QFormLayout(group);

    m_timeoutSpin = new QSpinBox(group);
    m_timeoutSpin->setRange(5, 480);
    m_timeoutSpin->setSuffix(" minutes");
    m_timeoutSpin->setValue(30);

    form->addRow("Auto-lock after:", m_timeoutSpin);

    auto* note = new QLabel("After this period of inactivity, the login screen will reappear.", group);
    note->setStyleSheet("color:#718096;font-size:11px;");
    form->addRow("", note);

    layout->addWidget(group);
    layout->addStretch();
}

void SettingsDialog::buildUsersTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 12, 16, 12);

    auto* tb = new QHBoxLayout;
    m_addUserBtn    = new QPushButton("+ Add User", tab);
    m_editUserBtn   = new QPushButton("✏ Edit", tab);
    m_changePwdBtn  = new QPushButton("🔑 Password", tab);
    m_deleteUserBtn = new QPushButton("🗑 Delete", tab);
    m_editUserBtn->setObjectName("secondaryBtn");
    m_changePwdBtn->setObjectName("secondaryBtn");
    m_deleteUserBtn->setObjectName("dangerBtn");
    m_editUserBtn->setEnabled(false);
    m_changePwdBtn->setEnabled(false);
    m_deleteUserBtn->setEnabled(false);
    tb->addStretch();
    tb->addWidget(m_changePwdBtn);
    tb->addWidget(m_editUserBtn);
    tb->addWidget(m_deleteUserBtn);
    tb->addWidget(m_addUserBtn);
    layout->addLayout(tb);

    m_usersTable = new QTableWidget(tab);
    m_usersTable->setColumnCount(5);
    m_usersTable->setHorizontalHeaderLabels({"Username","Full Name","Email","Role","Status"});
    m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_usersTable->horizontalHeader()->setStretchLastSection(true);
    m_usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_usersTable->verticalHeader()->setVisible(false);
    m_usersTable->setAlternatingRowColors(true);
    layout->addWidget(m_usersTable);

    connect(m_addUserBtn,    &QPushButton::clicked, this, &SettingsDialog::onAddUser);
    connect(m_editUserBtn,   &QPushButton::clicked, this, &SettingsDialog::onEditUser);
    connect(m_deleteUserBtn, &QPushButton::clicked, this, &SettingsDialog::onDeleteUser);
    connect(m_changePwdBtn,  &QPushButton::clicked, this, &SettingsDialog::onChangePassword);
    connect(m_usersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [this]() {
            bool sel = m_usersTable->currentRow() >= 0;
            m_editUserBtn->setEnabled(sel);
            m_changePwdBtn->setEnabled(sel);
            m_deleteUserBtn->setEnabled(sel && AuthController::instance().isAdmin());
        });
}

void SettingsDialog::buildBackupTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 16, 24, 16);
    auto* group = new QGroupBox("Database Backup & Restore", tab);
    auto* gl = new QVBoxLayout(group);

    auto* infoLbl = new QLabel("Database: hospital.db\nLocation: " +
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation), group);
    infoLbl->setStyleSheet("color:#4A5568;font-size:12px;");
    gl->addWidget(infoLbl);

    auto* backupBtn = new QPushButton("💾  Backup Database Now", group);
    backupBtn->setObjectName("successBtn");
    gl->addWidget(backupBtn);

    auto* vacuumBtn = new QPushButton("🔧  Optimize Database (VACUUM)", group);
    vacuumBtn->setObjectName("secondaryBtn");
    gl->addWidget(vacuumBtn);

    auto* note = new QLabel("Backup creates a timestamped copy of hospital.db in your home directory.", group);
    note->setStyleSheet("color:#718096;font-size:11px;margin-top:8px;");
    gl->addWidget(note);

    layout->addWidget(group);
    layout->addStretch();

    connect(backupBtn, &QPushButton::clicked, this, [this]() {
        QString dest = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + "/hospital_backup_"
            + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".db";
        if (QFile::copy("hospital.db", dest)) {
            ToastManager::instance().success("Backup saved: " + dest);
        } else {
            ToastManager::instance().error("Backup failed!");
        }
    });

    connect(vacuumBtn, &QPushButton::clicked, this, [this]() {
        auto r = DatabaseManager::instance().execute("VACUUM");
        if (r) ToastManager::instance().success("Database optimized successfully.");
        else   ToastManager::instance().error("Optimization failed: " + r.error());
    });
}

void SettingsDialog::loadSettings() {
    auto& s = AppSettings::instance();
    m_nameEdit->setText(s.hospitalName());
    m_addressEdit->setText(s.hospitalAddress());
    m_phoneEdit->setText(s.hospitalPhone());
    m_emailEdit->setText(s.hospitalEmail());
    m_logoEdit->setText(s.hospitalLogoPath());
    m_darkModeCheck->setChecked(s.darkMode());
    m_timeoutSpin->setValue(s.sessionTimeoutMinutes());
}

void SettingsDialog::loadUsers() {
    m_usersTable->setRowCount(0);
    auto users = AuthController::instance().allUsers();
    if (!users) return;
    for (const auto& u : *users) {
        int row = m_usersTable->rowCount();
        m_usersTable->insertRow(row);
        m_usersTable->setItem(row,0,new QTableWidgetItem(u.username));
        m_usersTable->setItem(row,1,new QTableWidgetItem(u.fullName));
        m_usersTable->setItem(row,2,new QTableWidgetItem(u.email));
        m_usersTable->setItem(row,3,new QTableWidgetItem(u.roleString()));
        auto* si = new QTableWidgetItem(u.active ? "Active" : "Inactive");
        si->setForeground(u.active ? QColor("#38A169") : QColor("#E53E3E"));
        m_usersTable->setItem(row,4,si);
        m_usersTable->item(row,0)->setData(Qt::UserRole, u.id);
    }
    m_usersTable->resizeColumnsToContents();
}

void SettingsDialog::saveSettings() {
    auto& s = AppSettings::instance();
    s.setHospitalName(m_nameEdit->text().trimmed());
    s.setHospitalAddress(m_addressEdit->text().trimmed());
    s.setHospitalPhone(m_phoneEdit->text().trimmed());
    s.setHospitalEmail(m_emailEdit->text().trimmed());
    s.setHospitalLogoPath(m_logoEdit->text().trimmed());
    s.setSessionTimeoutMinutes(m_timeoutSpin->value());
    s.sync();
}

void SettingsDialog::onSave() { saveSettings(); accept(); ToastManager::instance().success("Settings saved."); }

void SettingsDialog::onThemeToggle(bool dark) {
    AppSettings::instance().setDarkMode(dark);
    ThemeManager::instance().applyTheme(dark);
}

void SettingsDialog::onBrowseLogo() {
    QString path = QFileDialog::getOpenFileName(this, "Select Logo", "",
        "Images (*.png *.jpg *.jpeg *.svg)");
    if (!path.isEmpty()) m_logoEdit->setText(path);
}

void SettingsDialog::onAddUser() {
    QDialog dlg(this);
    dlg.setWindowTitle("Add User");
    dlg.setFixedWidth(400);
    auto* layout = new QVBoxLayout(&dlg);
    auto* form   = new QFormLayout;

    auto* unEdit  = new QLineEdit(&dlg);
    auto* nameEdit= new QLineEdit(&dlg);
    auto* emailEdit=new QLineEdit(&dlg);
    auto* pwEdit  = new QLineEdit(&dlg); pwEdit->setEchoMode(QLineEdit::Password);
    auto* roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({"admin","doctor","receptionist","billing"});
    auto* activeCheck = new QCheckBox("Active", &dlg); activeCheck->setChecked(true);

    form->addRow("Username *",  unEdit);
    form->addRow("Full Name *", nameEdit);
    form->addRow("Email",       emailEdit);
    form->addRow("Password *",  pwEdit);
    form->addRow("Role",        roleCombo);
    form->addRow("",            activeCheck);
    layout->addLayout(form);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg); cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn   = new QPushButton("💾 Create",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (unEdit->text().isEmpty() || nameEdit->text().isEmpty() || pwEdit->text().isEmpty()) {
            QMessageBox::warning(&dlg,"Validation","Username, name and password are required.");
            return;
        }
        User u;
        u.username = unEdit->text().trimmed();
        u.fullName = nameEdit->text().trimmed();
        u.email    = emailEdit->text().trimmed();
        u.role     = User::roleFromString(roleCombo->currentText());
        u.active   = activeCheck->isChecked();
        auto r = AuthController::instance().createUser(u, pwEdit->text());
        if (r) { dlg.accept(); loadUsers(); ToastManager::instance().success("User created."); }
        else   QMessageBox::critical(&dlg,"Error",r.error());
    });
    dlg.exec();
}

void SettingsDialog::onEditUser() {
    int row = m_usersTable->currentRow();
    if (row < 0) return;
    qint64 id = m_usersTable->item(row,0)->data(Qt::UserRole).toLongLong();
    auto users = AuthController::instance().allUsers();
    if (!users) return;
    auto it = std::find_if(users->begin(), users->end(), [id](const User& u){ return u.id == id; });
    if (it == users->end()) return;
    User user = *it;

    QDialog dlg(this);
    dlg.setWindowTitle("Edit User");
    dlg.setFixedWidth(400);
    auto* layout = new QVBoxLayout(&dlg);
    auto* form   = new QFormLayout;

    auto* nameEdit  = new QLineEdit(user.fullName, &dlg);
    auto* emailEdit = new QLineEdit(user.email, &dlg);
    auto* roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({"admin","doctor","receptionist","billing"});
    roleCombo->setCurrentText(user.roleString());
    auto* activeCheck = new QCheckBox("Active", &dlg); activeCheck->setChecked(user.active);

    form->addRow("Full Name *", nameEdit);
    form->addRow("Email",       emailEdit);
    form->addRow("Role",        roleCombo);
    form->addRow("",            activeCheck);
    layout->addLayout(form);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg); cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn   = new QPushButton("💾 Update",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        user.fullName = nameEdit->text().trimmed();
        user.email    = emailEdit->text().trimmed();
        user.role     = User::roleFromString(roleCombo->currentText());
        user.active   = activeCheck->isChecked();
        auto r = AuthController::instance().updateUser(user);
        if (r) { dlg.accept(); loadUsers(); ToastManager::instance().success("User updated."); }
        else QMessageBox::critical(&dlg,"Error",r.error());
    });
    dlg.exec();
}

void SettingsDialog::onDeleteUser() {
    int row = m_usersTable->currentRow();
    if (row < 0) return;
    qint64 id = m_usersTable->item(row,0)->data(Qt::UserRole).toLongLong();
    if (QMessageBox::question(this,"Confirm","Delete this user?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        auto r = AuthController::instance().deleteUser(id);
        if (r) { loadUsers(); ToastManager::instance().success("User deleted."); }
        else QMessageBox::critical(this,"Error",r.error());
    }
}

void SettingsDialog::onChangePassword() {
    int row = m_usersTable->currentRow();
    if (row < 0) return;
    qint64 id = m_usersTable->item(row,0)->data(Qt::UserRole).toLongLong();

    QDialog dlg(this);
    dlg.setWindowTitle("Change Password");
    dlg.setFixedWidth(340);
    auto* layout = new QVBoxLayout(&dlg);
    auto* form   = new QFormLayout;
    auto* pwEdit  = new QLineEdit(&dlg); pwEdit->setEchoMode(QLineEdit::Password);
    auto* pw2Edit = new QLineEdit(&dlg); pw2Edit->setEchoMode(QLineEdit::Password);
    form->addRow("New Password *",    pwEdit);
    form->addRow("Confirm Password *",pw2Edit);
    layout->addLayout(form);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg); cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn   = new QPushButton("🔑 Change",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (pwEdit->text() != pw2Edit->text()) {
            QMessageBox::warning(&dlg,"Validation","Passwords do not match."); return;
        }
        if (pwEdit->text().length() < 6) {
            QMessageBox::warning(&dlg,"Validation","Password must be at least 6 characters."); return;
        }
        auto r = AuthController::instance().changePassword(id, pwEdit->text());
        if (r) { dlg.accept(); ToastManager::instance().success("Password changed."); }
        else QMessageBox::critical(&dlg,"Error",r.error());
    });
    dlg.exec();
}

} // namespace HMS
