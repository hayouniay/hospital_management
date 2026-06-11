#pragma once
#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>

namespace HMS {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onSave();
    void onAddUser();
    void onEditUser();
    void onDeleteUser();
    void onChangePassword();
    void onThemeToggle(bool dark);
    void onBrowseLogo();

private:
    void setupUi();
    void buildHospitalTab(QWidget* tab);
    void buildAppearanceTab(QWidget* tab);
    void buildSecurityTab(QWidget* tab);
    void buildUsersTab(QWidget* tab);
    void buildBackupTab(QWidget* tab);
    void loadUsers();
    void loadSettings();
    void saveSettings();

    QTabWidget*   m_tabs{nullptr};

    // Hospital tab
    QLineEdit* m_nameEdit{nullptr};
    QLineEdit* m_addressEdit{nullptr};
    QLineEdit* m_phoneEdit{nullptr};
    QLineEdit* m_emailEdit{nullptr};
    QLineEdit* m_logoEdit{nullptr};

    // Appearance tab
    QCheckBox* m_darkModeCheck{nullptr};

    // Security tab
    QSpinBox*  m_timeoutSpin{nullptr};

    // Users tab
    QTableWidget* m_usersTable{nullptr};
    QPushButton*  m_addUserBtn{nullptr};
    QPushButton*  m_editUserBtn{nullptr};
    QPushButton*  m_deleteUserBtn{nullptr};
    QPushButton*  m_changePwdBtn{nullptr};
};

} // namespace HMS
