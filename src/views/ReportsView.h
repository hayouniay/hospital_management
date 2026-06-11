#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTabWidget>

namespace HMS {

class ReportsView : public QWidget {
    Q_OBJECT
public:
    explicit ReportsView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onGenerateReport();

private:
    void setupUi();
    void buildSummaryTab(QWidget* parent);
    void buildRevenueTab(QWidget* parent);
    void buildPatientsTab(QWidget* parent);
    void buildAppointmentsTab(QWidget* parent);

    void loadSummaryReport();
    void loadRevenueReport();
    void loadPatientReport();
    void loadAppointmentReport();

    QTabWidget*   m_tabs{nullptr};
    QDateEdit*    m_fromDate{nullptr};
    QDateEdit*    m_toDate{nullptr};
    QPushButton*  m_generateBtn{nullptr};

    // Summary
    QLabel* m_sumPatients{nullptr};
    QLabel* m_sumDoctors{nullptr};
    QLabel* m_sumDepts{nullptr};
    QLabel* m_sumAppts{nullptr};
    QLabel* m_sumRevenue{nullptr};
    QLabel* m_sumPending{nullptr};

    // Tables
    QTableWidget* m_revenueTable{nullptr};
    QTableWidget* m_patientTable{nullptr};
    QTableWidget* m_apptTable{nullptr};
};

} // namespace HMS
