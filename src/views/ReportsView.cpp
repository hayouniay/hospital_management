#include "ReportsView.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>

namespace HMS {

ReportsView::ReportsView(QWidget* parent) : QWidget(parent) { setupUi(); }

void ReportsView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    // Date range toolbar
    auto* tb = new QHBoxLayout;
    auto* fromLbl = new QLabel("From:", this);
    m_fromDate = new QDateEdit(this);
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setDate(QDate::currentDate().addMonths(-1));
    m_fromDate->setDisplayFormat("yyyy-MM-dd");

    auto* toLbl = new QLabel("To:", this);
    m_toDate = new QDateEdit(this);
    m_toDate->setCalendarPopup(true);
    m_toDate->setDate(QDate::currentDate());
    m_toDate->setDisplayFormat("yyyy-MM-dd");

    m_generateBtn = new QPushButton("📊 Generate Report", this);

    tb->addWidget(fromLbl); tb->addWidget(m_fromDate);
    tb->addSpacing(16);
    tb->addWidget(toLbl); tb->addWidget(m_toDate);
    tb->addSpacing(16);
    tb->addWidget(m_generateBtn);
    tb->addStretch();
    layout->addLayout(tb);

    // Tabs
    m_tabs = new QTabWidget(this);
    auto* summaryTab  = new QWidget;
    auto* revenueTab  = new QWidget;
    auto* patientTab  = new QWidget;
    auto* apptTab     = new QWidget;

    buildSummaryTab(summaryTab);
    buildRevenueTab(revenueTab);
    buildPatientsTab(patientTab);
    buildAppointmentsTab(apptTab);

    m_tabs->addTab(summaryTab, "📋 Summary");
    m_tabs->addTab(revenueTab, "💰 Revenue");
    m_tabs->addTab(patientTab, "👥 Patients");
    m_tabs->addTab(apptTab,    "📅 Appointments");

    layout->addWidget(m_tabs);
    connect(m_generateBtn, &QPushButton::clicked, this, &ReportsView::onGenerateReport);
}

void ReportsView::buildSummaryTab(QWidget* parent) {
    auto* scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* layout  = new QVBoxLayout(content);
    layout->setContentsMargins(16,16,16,16);
    layout->setSpacing(16);

    auto makeCard = [&](const QString& icon, const QString& title, QLabel*& valueLabel) -> QWidget* {
        auto* card = new QWidget(content);
        card->setStyleSheet("background:#FFF;border-radius:10px;border:1px solid #E2E8F0;");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(16,12,16,12);
        auto* topRow = new QHBoxLayout;
        auto* iconLbl = new QLabel(icon);
        iconLbl->setStyleSheet("font-size:22px;");
        auto* titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("font-size:12px;font-weight:600;color:#718096;");
        topRow->addWidget(iconLbl); topRow->addWidget(titleLbl); topRow->addStretch();
        cl->addLayout(topRow);
        valueLabel = new QLabel("—", card);
        valueLabel->setStyleSheet("font-size:28px;font-weight:700;color:#1A202C;");
        cl->addWidget(valueLabel);
        return card;
    };

    auto* title = new QLabel("Hospital Overview", content);
    title->setStyleSheet("font-size:17px;font-weight:700;color:#2D3748;");
    layout->addWidget(title);

    auto* grid = new QGridLayout;
    grid->setSpacing(14);
    grid->addWidget(makeCard("👥","Total Patients",    m_sumPatients),  0,0);
    grid->addWidget(makeCard("🩺","Total Doctors",     m_sumDoctors),   0,1);
    grid->addWidget(makeCard("🏢","Departments",       m_sumDepts),     0,2);
    grid->addWidget(makeCard("📅","Total Appointments",m_sumAppts),     1,0);
    grid->addWidget(makeCard("💰","Period Revenue",    m_sumRevenue),   1,1);
    grid->addWidget(makeCard("💳","Pending Bills",     m_sumPending),   1,2);
    layout->addLayout(grid);
    layout->addStretch();

    scroll->setWidget(content);
    auto* pl = new QVBoxLayout(parent);
    pl->setContentsMargins(0,0,0,0);
    pl->addWidget(scroll);
}

void ReportsView::buildRevenueTab(QWidget* parent) {
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(8,8,8,8);
    m_revenueTable = new QTableWidget(parent);
    m_revenueTable->setColumnCount(6);
    m_revenueTable->setHorizontalHeaderLabels({"Invoice","Patient","Amount","Paid","Balance","Status"});
    m_revenueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_revenueTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_revenueTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_revenueTable->verticalHeader()->setVisible(false);
    m_revenueTable->setAlternatingRowColors(true);
    layout->addWidget(m_revenueTable);
}

void ReportsView::buildPatientsTab(QWidget* parent) {
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(8,8,8,8);
    m_patientTable = new QTableWidget(parent);
    m_patientTable->setColumnCount(6);
    m_patientTable->setHorizontalHeaderLabels({"Patient ID","Name","Age","Gender","Blood Type","Status"});
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_patientTable->verticalHeader()->setVisible(false);
    m_patientTable->setAlternatingRowColors(true);
    layout->addWidget(m_patientTable);
}

void ReportsView::buildAppointmentsTab(QWidget* parent) {
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(8,8,8,8);
    m_apptTable = new QTableWidget(parent);
    m_apptTable->setColumnCount(6);
    m_apptTable->setHorizontalHeaderLabels({"Date","Patient","Doctor","Type","Duration","Status"});
    m_apptTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_apptTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_apptTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_apptTable->verticalHeader()->setVisible(false);
    m_apptTable->setAlternatingRowColors(true);
    layout->addWidget(m_apptTable);
}

void ReportsView::onGenerateReport() {
    loadSummaryReport();
    loadRevenueReport();
    loadPatientReport();
    loadAppointmentReport();
}

void ReportsView::refresh() { onGenerateReport(); }

void ReportsView::loadSummaryReport() {
    auto& ctrl = HospitalController::instance();
    auto stats = ctrl.getDashboardStats();
    if (!stats) return;
    m_sumPatients->setText(QString::number(stats->totalPatients));
    m_sumDoctors->setText(QString::number(stats->totalDoctors));
    m_sumDepts->setText(QString::number(stats->totalDepartments));

    auto appts = ctrl.appointments().findAll(9999);
    m_sumAppts->setText(appts ? QString::number(appts->size()) : "—");

    auto rev = ctrl.billing().totalRevenue(m_fromDate->date(), m_toDate->date());
    m_sumRevenue->setText(rev ? QString("$%1").arg(*rev,0,'f',2) : "—");
    m_sumPending->setText(QString::number(stats->pendingBills));
}

void ReportsView::loadRevenueReport() {
    m_revenueTable->setRowCount(0);
    auto bills = HospitalController::instance().billing().findAll(1000);
    if (!bills) return;
    for (const auto& b : *bills) {
        // Filter by date range
        if (b.createdAt.date() < m_fromDate->date() ||
            b.createdAt.date() > m_toDate->date()) continue;
        int row = m_revenueTable->rowCount();
        m_revenueTable->insertRow(row);
        m_revenueTable->setItem(row,0,new QTableWidgetItem(b.invoiceNumber));
        m_revenueTable->setItem(row,1,new QTableWidgetItem(b.patientName));
        m_revenueTable->setItem(row,2,new QTableWidgetItem(QString("$%1").arg(b.total,0,'f',2)));
        m_revenueTable->setItem(row,3,new QTableWidgetItem(QString("$%1").arg(b.paidAmount,0,'f',2)));
        double bal = b.balance();
        auto* balItem = new QTableWidgetItem(QString("$%1").arg(bal,0,'f',2));
        balItem->setForeground(bal>0 ? QColor("#E53E3E") : QColor("#38A169"));
        m_revenueTable->setItem(row,4,balItem);
        auto* si = new QTableWidgetItem(b.status);
        if (b.status=="paid")    si->setForeground(QColor("#38A169"));
        else if (b.status=="pending") si->setForeground(QColor("#3182CE"));
        else if (b.status=="partial") si->setForeground(QColor("#D69E2E"));
        m_revenueTable->setItem(row,5,si);
    }
    m_revenueTable->resizeColumnsToContents();
}

void ReportsView::loadPatientReport() {
    m_patientTable->setRowCount(0);
    auto patients = HospitalController::instance().patients().findAll(1000);
    if (!patients) return;
    for (const auto& p : *patients) {
        int row = m_patientTable->rowCount();
        m_patientTable->insertRow(row);
        m_patientTable->setItem(row,0,new QTableWidgetItem(p.patientUid));
        m_patientTable->setItem(row,1,new QTableWidgetItem(p.fullName()));
        m_patientTable->setItem(row,2,new QTableWidgetItem(QString::number(p.age())));
        m_patientTable->setItem(row,3,new QTableWidgetItem(p.gender));
        m_patientTable->setItem(row,4,new QTableWidgetItem(p.bloodType));
        auto* si = new QTableWidgetItem(p.status);
        si->setForeground(p.status=="active" ? QColor("#38A169") : QColor("#718096"));
        m_patientTable->setItem(row,5,si);
    }
    m_patientTable->resizeColumnsToContents();
}

void ReportsView::loadAppointmentReport() {
    m_apptTable->setRowCount(0);
    auto appts = HospitalController::instance().appointments().findAll(1000);
    if (!appts) return;
    for (const auto& a : *appts) {
        if (a.appointmentDate < m_fromDate->date() ||
            a.appointmentDate > m_toDate->date()) continue;
        int row = m_apptTable->rowCount();
        m_apptTable->insertRow(row);
        m_apptTable->setItem(row,0,new QTableWidgetItem(a.appointmentDate.toString("yyyy-MM-dd")));
        m_apptTable->setItem(row,1,new QTableWidgetItem(a.patientName));
        m_apptTable->setItem(row,2,new QTableWidgetItem(a.doctorName));
        m_apptTable->setItem(row,3,new QTableWidgetItem(a.type));
        m_apptTable->setItem(row,4,new QTableWidgetItem(QString::number(a.durationMin)+" min"));
        auto* si = new QTableWidgetItem(a.status);
        if      (a.status=="completed")  si->setForeground(QColor("#38A169"));
        else if (a.status=="cancelled")  si->setForeground(QColor("#E53E3E"));
        else if (a.status=="scheduled")  si->setForeground(QColor("#3182CE"));
        m_apptTable->setItem(row,5,si);
    }
    m_apptTable->resizeColumnsToContents();
}

} // namespace HMS
