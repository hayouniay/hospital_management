#include "MainWindow.h"
#include "DashboardView.h"
#include "PatientView.h"
#include "DoctorView.h"
#include "AppointmentView.h"
#include "RegistrationView.h"
#include "BillingView.h"
#include "MedicalRecordView.h"
#include "DepartmentView.h"
#include "ReportsView.h"
#include "PatientTimelineView.h"
#include "AuditLogView.h"
#include "SettingsDialog.h"

#include "widgets/SidebarMenu.h"
#include "widgets/GlobalSearchBar.h"
#include "widgets/ToastNotification.h"

#include "auth/AuthController.h"
#include "auth/LoginDialog.h"
#include "settings/AppSettings.h"
#include "settings/ThemeManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QStatusBar>
#include <QShortcut>

namespace HMS {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("MediCare — Hospital Management System");
    setMinimumSize(1280, 780);
    resize(1440, 860);
    setupUi();
    setupMenuBar();
    setupConnections();
    applyInitialTheme();

    // Register parent for toast notifications
    ToastManager::instance().setParent(this);

    // Clock timer
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(60000);
    updateClock();

    // Session timer
    auto& settings = AppSettings::instance();
    m_sessionTicksLeft = settings.sessionTimeoutMinutes() * 60;
    m_sessionTimer = new QTimer(this);
    connect(m_sessionTimer, &QTimer::timeout, this, &MainWindow::onSessionTick);
    m_sessionTimer->start(1000);

    qApp->installEventFilter(this);

    // ── Global keyboard shortcuts ────────────────────────────────────────────
    auto mkShortcut = [this](const QKeySequence& key, auto slot) {
        auto* sc = new QShortcut(key, this);
        connect(sc, &QShortcut::activated, this, slot);
    };
    // Navigation
    mkShortcut(QKeySequence("Ctrl+1"), [this](){ onNavigateTo(0); });
    mkShortcut(QKeySequence("Ctrl+2"), [this](){ onNavigateTo(1); });
    mkShortcut(QKeySequence("Ctrl+3"), [this](){ onNavigateTo(2); });
    mkShortcut(QKeySequence("Ctrl+4"), [this](){ onNavigateTo(3); });
    mkShortcut(QKeySequence("Ctrl+5"), [this](){ onNavigateTo(4); });
    mkShortcut(QKeySequence("Ctrl+6"), [this](){ onNavigateTo(5); });
    mkShortcut(QKeySequence("Ctrl+7"), [this](){ onNavigateTo(6); });
    mkShortcut(QKeySequence("Ctrl+8"), [this](){ onNavigateTo(7); });
    mkShortcut(QKeySequence("Ctrl+9"), [this](){ onNavigateTo(8); });
    // Actions
    mkShortcut(QKeySequence("Ctrl+F"), [this](){
        m_globalSearch->focusSearch();
    });
    mkShortcut(QKeySequence("Ctrl+,"), [this](){
        onOpenSettings();
    });
    mkShortcut(QKeySequence("F5"), [this](){
        onMenuItemSelected(m_stack->currentIndex());
    });
    mkShortcut(QKeySequence("Ctrl+D"), [this](){ onToggleTheme(); });

    onMenuItemSelected(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // ── Sidebar ───────────────────────────────────────────────────────────────
    m_sidebar = new SidebarMenu(this);
    mainLayout->addWidget(m_sidebar);

    // ── Content ───────────────────────────────────────────────────────────────
    auto* contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->setSpacing(0);

    // Top bar
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(58);
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20,0,20,0);
    topLayout->setSpacing(12);

    m_titleLabel = new QLabel("Dashboard", topBar);
    m_titleLabel->setObjectName("pageTitle");

    m_globalSearch = new GlobalSearchBar(topBar);

    m_clockLabel = new QLabel(topBar);
    m_clockLabel->setObjectName("clockLabel");

    // Theme toggle button
    auto* themeBtn = new QPushButton("🌙", topBar);
    themeBtn->setFixedSize(34,34);
    themeBtn->setObjectName("secondaryBtn");
    themeBtn->setToolTip("Toggle Dark Mode");
    themeBtn->setStyleSheet("QPushButton{border-radius:17px;font-size:16px;padding:0;}");
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme);

    // Settings button
    auto* settingsBtn = new QPushButton("⚙", topBar);
    settingsBtn->setFixedSize(34,34);
    settingsBtn->setObjectName("secondaryBtn");
    settingsBtn->setToolTip("Settings");
    settingsBtn->setStyleSheet("QPushButton{border-radius:17px;font-size:16px;padding:0;}");
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onOpenSettings);

    // User info
    auto& auth = AuthController::instance();
    QString userText = "👤  ";
    if (auth.currentUser()) userText += auth.currentUser()->fullName;
    m_userLabel = new QLabel(userText, topBar);
    m_userLabel->setObjectName("clockLabel");

    // Logout button
    auto* logoutBtn = new QPushButton("⏻", topBar);
    logoutBtn->setFixedSize(34,34);
    logoutBtn->setObjectName("dangerBtn");
    logoutBtn->setToolTip("Logout");
    logoutBtn->setStyleSheet("QPushButton{border-radius:17px;font-size:14px;padding:0;}");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);

    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_globalSearch);
    topLayout->addStretch();
    topLayout->addWidget(m_clockLabel);
    topLayout->addWidget(themeBtn);
    topLayout->addWidget(settingsBtn);
    topLayout->addWidget(m_userLabel);
    topLayout->addWidget(logoutBtn);

    contentLayout->addWidget(topBar);

    // ── Page stack ────────────────────────────────────────────────────────────
    m_stack = new QStackedWidget(this);

    m_dashboard    = new DashboardView(this);
    m_patients     = new PatientView(this);
    m_doctors      = new DoctorView(this);
    m_appointments = new AppointmentView(this);
    m_registrations= new RegistrationView(this);
    m_billing      = new BillingView(this);
    m_medicalRecords=new MedicalRecordView(this);
    m_departments  = new DepartmentView(this);
    m_reports      = new ReportsView(this);
    m_timeline     = new PatientTimelineView(this);
    m_auditLog     = new AuditLogView(this);

    m_stack->addWidget(m_dashboard);      // 0
    m_stack->addWidget(m_patients);       // 1
    m_stack->addWidget(m_doctors);        // 2
    m_stack->addWidget(m_appointments);   // 3
    m_stack->addWidget(m_registrations);  // 4
    m_stack->addWidget(m_billing);        // 5
    m_stack->addWidget(m_medicalRecords); // 6
    m_stack->addWidget(m_departments);    // 7
    m_stack->addWidget(m_reports);        // 8
    m_stack->addWidget(m_timeline);       // 9
    m_stack->addWidget(m_auditLog);       // 10

    contentLayout->addWidget(m_stack);
    mainLayout->addWidget(contentWidget);

    statusBar()->showMessage("Ready  —  " + AppSettings::instance().hospitalName());
}

void MainWindow::setupMenuBar() {
    auto* mb = menuBar();
    mb->setVisible(false); // hidden — using top bar buttons instead
}

void MainWindow::setupConnections() {
    connect(m_sidebar, &SidebarMenu::itemSelected,
            this,      &MainWindow::onMenuItemSelected);
    connect(m_globalSearch, &GlobalSearchBar::navigateTo,
            this,           &MainWindow::onNavigateTo);
}

void MainWindow::applyInitialTheme() {
    ThemeManager::instance().applyTheme(AppSettings::instance().darkMode());
}

void MainWindow::onMenuItemSelected(int index) {
    m_stack->setCurrentIndex(index);
    const QStringList titles = {
        "Dashboard","Patients","Doctors","Appointments",
        "Registration","Billing","Medical Records","Departments",
        "Reports","Patient Timeline","Audit Log"
    };
    if (index < titles.size())
        m_titleLabel->setText(titles[index]);

    // Refresh active view
    switch (index) {
        case 0:  m_dashboard->refresh();     break;
        case 1:  m_patients->refresh();      break;
        case 2:  m_doctors->refresh();       break;
        case 3:  m_appointments->refresh();  break;
        case 4:  m_registrations->refresh(); break;
        case 5:  m_billing->refresh();       break;
        case 6:  m_medicalRecords->refresh();break;
        case 7:  m_departments->refresh();   break;
        case 8:  m_reports->refresh();       break;
        case 9:  m_timeline->refresh();      break;
        case 10: m_auditLog->refresh();      break;
        default: break;
    }
}

void MainWindow::onNavigateTo(int pageIndex) {
    m_sidebar->selectItem(pageIndex);
    onMenuItemSelected(pageIndex);
}

void MainWindow::onToggleTheme() {
    bool newDark = !ThemeManager::instance().isDark();
    AppSettings::instance().setDarkMode(newDark);
    ThemeManager::instance().applyTheme(newDark);
    ToastManager::instance().info(newDark ? "Dark mode enabled" : "Light mode enabled");
}

void MainWindow::onOpenSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::onLogout() {
    AuthController::instance().logout();
    hide();

    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        auto& auth = AuthController::instance();
        if (auth.currentUser())
            m_userLabel->setText("👤  " + auth.currentUser()->fullName);
        m_sessionTicksLeft = AppSettings::instance().sessionTimeoutMinutes() * 60;
        show();
        ToastManager::instance().success("Welcome back, " +
            (auth.currentUser() ? auth.currentUser()->fullName : "User") + "!");
    } else {
        qApp->quit();
    }
}

void MainWindow::onSessionTick() {
    --m_sessionTicksLeft;
    if (m_sessionTicksLeft <= 0) {
        m_sessionTimer->stop();
        ToastManager::instance().warning("Session expired. Please log in again.");
        QTimer::singleShot(1500, this, &MainWindow::onLogout);
    }
}

void MainWindow::resetSessionTimer() {
    m_sessionTicksLeft = AppSettings::instance().sessionTimeoutMinutes() * 60;
}

bool MainWindow::eventFilter(QObject* /*obj*/, QEvent* ev) {
    if (ev->type() == QEvent::MouseMove ||
        ev->type() == QEvent::KeyPress  ||
        ev->type() == QEvent::MouseButtonPress) {
        resetSessionTimer();
    }
    return false; // never consume events
}

void MainWindow::updateClock() {
    m_clockLabel->setText(
        QDateTime::currentDateTime().toString("ddd, MMM d  hh:mm AP"));
}

} // namespace HMS
