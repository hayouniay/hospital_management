#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <QMenuBar>
#include <memory>

namespace HMS {

class SidebarMenu;
class GlobalSearchBar;
class DashboardView;
class PatientView;
class DoctorView;
class AppointmentView;
class RegistrationView;
class BillingView;
class MedicalRecordView;
class DepartmentView;
class ReportsView;
class PatientTimelineView;
class AuditLogView;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

public slots:
    void onNavigateTo(int pageIndex);

private slots:
    void onMenuItemSelected(int index);
    void onToggleTheme();
    void onOpenSettings();
    void onLogout();
    void onSessionTick();
    void updateClock();

private:
    void setupUi();
    void setupMenuBar();
    void setupConnections();
    void applyInitialTheme();
    void resetSessionTimer();
    bool eventFilter(QObject* obj, QEvent* ev) override;

    SidebarMenu*         m_sidebar{nullptr};
    GlobalSearchBar*     m_globalSearch{nullptr};
    QStackedWidget*      m_stack{nullptr};

    DashboardView*       m_dashboard{nullptr};
    PatientView*         m_patients{nullptr};
    DoctorView*          m_doctors{nullptr};
    AppointmentView*     m_appointments{nullptr};
    RegistrationView*    m_registrations{nullptr};
    BillingView*         m_billing{nullptr};
    MedicalRecordView*   m_medicalRecords{nullptr};
    DepartmentView*      m_departments{nullptr};
    ReportsView*         m_reports{nullptr};
    PatientTimelineView* m_timeline{nullptr};
    AuditLogView*        m_auditLog{nullptr};

    QLabel*  m_titleLabel{nullptr};
    QLabel*  m_clockLabel{nullptr};
    QLabel*  m_userLabel{nullptr};
    QAction* m_themeAction{nullptr};

    QTimer* m_clockTimer{nullptr};
    QTimer* m_sessionTimer{nullptr};
    int     m_sessionTicksLeft{0};
};

} // namespace HMS
