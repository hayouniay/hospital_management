#include "DashboardView.h"
#include "widgets/StatCard.h"
#include "core/HospitalController.h"
#include "auth/AuthController.h"
#include "views/PatientView.h"
#include "views/AppointmentView.h"

// Qt Charts
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QPieSeries>
#include <QChart>
#include <QLineSeries>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QHeaderView>



namespace HMS {

DashboardView::DashboardView(QWidget* parent) : QWidget(parent) { setupUi(); }

void DashboardView::setupUi() {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget(scroll);
    auto* layout  = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(20);

    // ── Banner ────────────────────────────────────────────────────────────────
    auto* banner = new QWidget(content);
    banner->setObjectName("dashBanner");
    banner->setFixedHeight(80);
    auto* bl = new QVBoxLayout(banner);
    bl->setContentsMargins(20,12,20,12);

    auto& auth = AuthController::instance();
    QString greeting = auth.currentUser()
        ? "👋  Welcome back, " + auth.currentUser()->fullName
        : "👋  Welcome back";

    auto* greet  = new QLabel(greeting, banner);
    greet->setStyleSheet("font-size:18px;font-weight:700;color:#FFF;");
    auto* dateLbl = new QLabel(
        QDateTime::currentDateTime().toString("dddd, MMMM d, yyyy  •  hh:mm AP"), banner);
    dateLbl->setStyleSheet("font-size:12px;color:#BEE3F8;");
    bl->addWidget(greet);
    bl->addWidget(dateLbl);
    banner->setStyleSheet("#dashBanner{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                         "stop:0 #2B6CB0,stop:1 #3182CE);border-radius:12px;}");
    layout->addWidget(banner);

    // ── Quick Actions ─────────────────────────────────────────────────────────
    buildQuickActions(layout);

    // ── Stat cards ────────────────────────────────────────────────────────────
    auto* statsLbl = new QLabel("📊  Overview", content);
    statsLbl->setStyleSheet("font-size:15px;font-weight:700;color:#2D3748;");
    layout->addWidget(statsLbl);

    auto* grid = new QGridLayout;
    grid->setSpacing(14);
    buildStatCards(grid);
    layout->addLayout(grid);

    // ── Charts row ────────────────────────────────────────────────────────────
    buildCharts(layout);

    // ── Today's appointments ──────────────────────────────────────────────────
    buildTodayTable(layout);

    layout->addStretch();
    scroll->setWidget(content);

    auto* ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);
    ml->addWidget(scroll);
}

void DashboardView::buildQuickActions(QVBoxLayout* layout) {
    auto* row = new QHBoxLayout;
    row->setSpacing(10);

    auto makeQA = [&](const QString& icon, const QString& label,
                      const QString& color, auto slot) {
        auto* btn = new QPushButton(icon + "  " + label);
        btn->setFixedHeight(42);
        btn->setStyleSheet(QString(
            "QPushButton{background:%1;color:white;border-radius:8px;"
            "font-size:13px;font-weight:600;border:none;}"
            "QPushButton:hover{background:%1;opacity:0.9;}"
        ).arg(color));
        connect(btn, &QPushButton::clicked, this, slot);
        row->addWidget(btn);
    };

    makeQA("👥", "New Patient",       "#3182CE", [](){});
    makeQA("📅", "Book Appointment",   "#38A169", [](){});
    makeQA("🏨", "Admit Patient",      "#805AD5", [](){});
    makeQA("💳", "Create Invoice",     "#D69E2E", [](){});
    makeQA("📋", "Add Medical Record", "#DD6B20", [](){});

    auto* qaLbl = new QLabel("⚡  Quick Actions", this);
    qaLbl->setStyleSheet("font-size:15px;font-weight:700;color:#2D3748;");
    layout->addWidget(qaLbl);
    layout->addLayout(row);
}

void DashboardView::buildStatCards(QGridLayout* grid) {
    m_cardPatients     = new StatCard("👥","Total Patients",      "0","#3182CE",this);
    m_cardAdmitted     = new StatCard("🏨","Currently Admitted",  "0","#38A169",this);
    m_cardAppointments = new StatCard("📅","Today's Appointments","0","#D69E2E",this);
    m_cardRevenue      = new StatCard("💰","Today's Revenue",    "$0","#805AD5",this);
    m_cardDoctors      = new StatCard("🩺","Active Doctors",      "0","#DD6B20",this);
    m_cardPendingBills = new StatCard("💳","Pending Bills",       "0","#E53E3E",this);
    m_cardDepartments  = new StatCard("🏢","Departments",         "0","#319795",this);

    grid->addWidget(m_cardPatients,     0, 0);
    grid->addWidget(m_cardAdmitted,     0, 1);
    grid->addWidget(m_cardAppointments, 0, 2);
    grid->addWidget(m_cardRevenue,      1, 0);
    grid->addWidget(m_cardDoctors,      1, 1);
    grid->addWidget(m_cardPendingBills, 1, 2);
    grid->addWidget(m_cardDepartments,  2, 0);
}

void DashboardView::buildCharts(QVBoxLayout* layout) {
    auto* chartsLbl = new QLabel("📈  Analytics", this);
    chartsLbl->setStyleSheet("font-size:15px;font-weight:700;color:#2D3748;");
    layout->addWidget(chartsLbl);

    auto* chartsRow = new QHBoxLayout;
    chartsRow->setSpacing(16);

    // ── Appointment status pie chart ──────────────────────────────────────────
    auto* pieSeries = new QPieSeries();
    pieSeries->append("Scheduled",   1)->setColor(QColor("#3182CE"));
    pieSeries->append("Completed",   1)->setColor(QColor("#38A169"));
    pieSeries->append("Cancelled",   1)->setColor(QColor("#E53E3E"));
    pieSeries->append("In Progress", 1)->setColor(QColor("#D69E2E"));

    auto* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Appointment Status");
    pieChart->legend()->setAlignment(Qt::AlignBottom);
    pieChart->setAnimationOptions(QChart::AllAnimations);
    pieChart->setBackgroundRoundness(8);
    pieChart->setMargins(QMargins(4,4,4,4));

    auto* pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    pieView->setFixedHeight(240);
    m_statusPieView = pieView;
    chartsRow->addWidget(pieView);

    // ── Weekly appointments bar chart ─────────────────────────────────────────
    auto* barSet = new QBarSet("Appointments");
    *barSet << 3 << 5 << 2 << 8 << 4 << 6 << 3;
    barSet->setColor(QColor("#3182CE"));

    auto* barSeries = new QBarSeries();
    barSeries->append(barSet);

    auto* barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Appointments This Week");
    barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->setBackgroundRoundness(8);
    barChart->setMargins(QMargins(4,4,4,4));

    auto* axisX = new QBarCategoryAxis();
    axisX->append({"Mon","Tue","Wed","Thu","Fri","Sat","Sun"});
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto* axisY = new QValueAxis();
    axisY->setRange(0, 12);
    axisY->setLabelFormat("%d");
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
    barChart->legend()->hide();

    auto* barView = new QChartView(barChart);
    barView->setRenderHint(QPainter::Antialiasing);
    barView->setFixedHeight(240);
    m_appointmentChartView = barView;
    chartsRow->addWidget(barView);

    // ── Revenue line chart ────────────────────────────────────────────────────
    auto* lineSeries = new QLineSeries();
    lineSeries->setName("Revenue ($)");
    lineSeries->setColor(QColor("#38A169"));
    QList<QPoint> pts = {{0,120},{1,340},{2,280},{3,510},{4,380},{5,620},{6,490}};
    for (auto& pt : pts) lineSeries->append(pt.x(), pt.y());

    QPen pen(QColor("#38A169"));
    pen.setWidth(2);
    lineSeries->setPen(pen);

    auto* lineChart = new QChart();
    lineChart->addSeries(lineSeries);
    lineChart->setTitle("Revenue This Week ($)");
    lineChart->setAnimationOptions(QChart::SeriesAnimations);
    lineChart->setBackgroundRoundness(8);
    lineChart->setMargins(QMargins(4,4,4,4));

    auto* lAxisX = new QBarCategoryAxis();
    lAxisX->append({"Mon","Tue","Wed","Thu","Fri","Sat","Sun"});
    lineChart->addAxis(lAxisX, Qt::AlignBottom);
    lineSeries->attachAxis(lAxisX);

    auto* lAxisY = new QValueAxis();
    lAxisY->setRange(0, 800);
    lAxisY->setLabelFormat("$%d");
    lineChart->addAxis(lAxisY, Qt::AlignLeft);
    lineSeries->attachAxis(lAxisY);
    lineChart->legend()->hide();

    auto* lineView = new QChartView(lineChart);
    lineView->setRenderHint(QPainter::Antialiasing);
    lineView->setFixedHeight(240);
    m_revenueChartView = lineView;
    chartsRow->addWidget(lineView);

    layout->addLayout(chartsRow);
}

void DashboardView::buildTodayTable(QVBoxLayout* layout) {
    auto* lbl = new QLabel("📅  Today's Schedule", this);
    lbl->setStyleSheet("font-size:15px;font-weight:700;color:#2D3748;");
    layout->addWidget(lbl);

    m_todayTable = new QTableWidget(this);
    m_todayTable->setColumnCount(6);
    m_todayTable->setHorizontalHeaderLabels({"Time","Patient","Doctor","Type","Duration","Status"});
    m_todayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_todayTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_todayTable->verticalHeader()->setVisible(false);
    m_todayTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_todayTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_todayTable->setAlternatingRowColors(true);
    m_todayTable->setMaximumHeight(220);
    layout->addWidget(m_todayTable);
}

void DashboardView::refreshCharts() {
    // Update pie chart with real data
    auto& ctrl = HospitalController::instance();

    if (auto* pv = qobject_cast<QChartView*>(m_statusPieView)) {
        auto* chart  = pv->chart();
        auto series = qobject_cast<QPieSeries*>(chart->series().first());
        if (series) {
            auto getCount = [&](const QString& status) -> int {
                auto r = ctrl.appointments().countByStatus(status);
                return r ? *r : 0;
            };
            int scheduled   = getCount("scheduled");
            int completed   = getCount("completed");
            int cancelled   = getCount("cancelled");
            int inProgress  = getCount("in-progress");
            int total = scheduled + completed + cancelled + inProgress;
            if (total > 0) {
                series->slices().at(0)->setValue(qMax(1, scheduled));
                series->slices().at(1)->setValue(qMax(1, completed));
                series->slices().at(2)->setValue(qMax(1, cancelled));
                series->slices().at(3)->setValue(qMax(1, inProgress));
            }
        }
    }
}

void DashboardView::refresh() {
    auto& ctrl = HospitalController::instance();
    auto statsResult = ctrl.getDashboardStats();
    if (statsResult) {
        auto& s = *statsResult;
        m_cardPatients->setValue(QString::number(s.totalPatients));
        m_cardAdmitted->setValue(QString::number(s.admittedPatients));
        m_cardAppointments->setValue(QString::number(s.todayAppointments));
        m_cardRevenue->setValue(QString("$%1").arg(s.todayRevenue, 0, 'f', 2));
        m_cardDoctors->setValue(QString::number(s.totalDoctors));
        m_cardPendingBills->setValue(QString::number(s.pendingBills));
        m_cardDepartments->setValue(QString::number(s.totalDepartments));
    }

    refreshCharts();

    // Today's appointments table
    m_todayTable->setRowCount(0);
    auto appts = ctrl.appointments().findToday();
    if (appts) {
        for (const auto& a : *appts) {
            int row = m_todayTable->rowCount();
            m_todayTable->insertRow(row);
            m_todayTable->setItem(row,0,new QTableWidgetItem(a.appointmentTime.toString("hh:mm AP")));
            m_todayTable->setItem(row,1,new QTableWidgetItem(a.patientName));
            m_todayTable->setItem(row,2,new QTableWidgetItem("Dr. " + a.doctorName));
            m_todayTable->setItem(row,3,new QTableWidgetItem(a.type));
            m_todayTable->setItem(row,4,new QTableWidgetItem(QString::number(a.durationMin)+" min"));
            auto* si = new QTableWidgetItem(a.status);
            if      (a.status=="completed")  si->setForeground(QColor("#38A169"));
            else if (a.status=="scheduled")  si->setForeground(QColor("#3182CE"));
            else if (a.status=="in-progress")si->setForeground(QColor("#D69E2E"));
            else if (a.status=="cancelled")  si->setForeground(QColor("#E53E3E"));
            m_todayTable->setItem(row,5,si);
        }
    }
    m_todayTable->resizeColumnsToContents();
}

} // namespace HMS
