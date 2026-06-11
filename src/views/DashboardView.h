#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>

namespace HMS {

class StatCard;

class DashboardView : public QWidget {
    Q_OBJECT
public:
    explicit DashboardView(QWidget* parent = nullptr);
    void refresh();

private:
    void setupUi();
    void buildStatCards(QGridLayout* grid);
    void buildCharts(QVBoxLayout* layout);
    void buildTodayTable(QVBoxLayout* layout);
    void buildQuickActions(QVBoxLayout* layout);
    void refreshCharts();

    StatCard* m_cardPatients{nullptr};
    StatCard* m_cardAdmitted{nullptr};
    StatCard* m_cardAppointments{nullptr};
    StatCard* m_cardRevenue{nullptr};
    StatCard* m_cardDoctors{nullptr};
    StatCard* m_cardPendingBills{nullptr};
    StatCard* m_cardDepartments{nullptr};

    QTableWidget* m_todayTable{nullptr};
    QWidget*      m_appointmentChartView{nullptr};
    QWidget*      m_revenueChartView{nullptr};
    QWidget*      m_statusPieView{nullptr};
};

} // namespace HMS
