#include "PatientTimelineView.h"
#include "core/HospitalController.h"
#include "core/MedicalRecordController.h"
#include "core/RegistrationController.h"
#include "utils/PDFExporter.h"
#include "widgets/ToastNotification.h"

#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>
#include <algorithm>

namespace HMS {

PatientTimelineView::PatientTimelineView(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void PatientTimelineView::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 24);
    root->setSpacing(12);

    // ── Toolbar ───────────────────────────────────────────────────────────────
    auto* tb = new QHBoxLayout;

    auto* lbl = new QLabel("Patient:", this);
    lbl->setStyleSheet("font-weight:600;font-size:13px;");

    m_patientCombo = new QComboBox(this);
    m_patientCombo->setMinimumWidth(320);
    m_patientCombo->setFixedHeight(36);

    // Populate patients
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients) {
        for (const auto& p : *patients) {
            m_patientCombo->addItem(
                p.patientUid + "  —  " + p.fullName(), p.id);
        }
    }

    m_exportBtn = new QPushButton("📄 Export Report", this);
    m_exportBtn->setObjectName("secondaryBtn");
    m_exportBtn->setEnabled(false);

    tb->addWidget(lbl);
    tb->addWidget(m_patientCombo);
    tb->addStretch();
    tb->addWidget(m_exportBtn);
    root->addLayout(tb);

    // ── Patient info bar ──────────────────────────────────────────────────────
    m_patientInfoLabel = new QLabel("Select a patient to view their timeline.", this);
    m_patientInfoLabel->setStyleSheet(R"(
        background:#EBF8FF;border:1px solid #BEE3F8;border-radius:8px;
        padding:10px 16px;color:#2B6CB0;font-size:13px;
    )");
    root->addWidget(m_patientInfoLabel);

    // ── Timeline scroll area ──────────────────────────────────────────────────
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_timelineWidget = new QWidget;
    m_timelineLayout = new QVBoxLayout(m_timelineWidget);
    m_timelineLayout->setContentsMargins(0, 0, 0, 0);
    m_timelineLayout->setSpacing(0);
    m_timelineLayout->addStretch();

    m_scroll->setWidget(m_timelineWidget);
    root->addWidget(m_scroll);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_patientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                if (idx < 0) return;
                setPatient(m_patientCombo->currentData().toLongLong());
            });

    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (m_patientId <= 0) return;
        auto patResult = HospitalController::instance().patients().findById(m_patientId);
        if (!patResult) return;
        auto records = HospitalController::instance().medicalRecords().findByPatient(m_patientId);
        if (!records) { ToastManager::instance().error(records.error()); return; }
        auto result = PDFExporter::exportConsultationReportDialog(*patResult, *records, this);
        if (result) ToastManager::instance().success("Report exported successfully.");
        else if (result.error() != "Cancelled")
            ToastManager::instance().error("Export failed: " + result.error());
    });

    // Load first patient if available
    if (m_patientCombo->count() > 0)
        setPatient(m_patientCombo->currentData().toLongLong());
}

void PatientTimelineView::setPatient(qint64 patientId) {
    m_patientId = patientId;
    m_exportBtn->setEnabled(patientId > 0);
    buildTimeline();
}

void PatientTimelineView::refresh() {
    // Reload patient combo
    qint64 currentId = m_patientId;
    m_patientCombo->blockSignals(true);
    m_patientCombo->clear();
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients) {
        for (const auto& p : *patients)
            m_patientCombo->addItem(p.patientUid + "  —  " + p.fullName(), p.id);
    }
    m_patientCombo->blockSignals(false);

    // Restore selection
    int idx = m_patientCombo->findData(currentId);
    if (idx >= 0) m_patientCombo->setCurrentIndex(idx);
    buildTimeline();
}

void PatientTimelineView::clearTimeline() {
    // Remove all items except the final stretch
    QLayoutItem* item;
    while ((item = m_timelineLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void PatientTimelineView::buildTimeline() {
    clearTimeline();

    if (m_patientId <= 0) return;

    // ── Load patient info ─────────────────────────────────────────────────────
    auto patResult = HospitalController::instance().patients().findById(m_patientId);
    if (!patResult) return;
    const auto& patient = *patResult;

    m_patientInfoLabel->setText(QString(
        "👤  <b>%1</b>  &nbsp;|&nbsp;  ID: %2  &nbsp;|&nbsp;  "
        "Age: %3  &nbsp;|&nbsp;  Blood: %4  &nbsp;|&nbsp;  Status: <b>%5</b>")
        .arg(patient.fullName(), patient.patientUid)
        .arg(patient.age())
        .arg(patient.bloodType, patient.status));

    // ── Gather all events ─────────────────────────────────────────────────────
    std::vector<TimelineEvent> events;

    // Registration events
    auto regs = HospitalController::instance().registrations().findAll(200);
    if (regs) {
        for (const auto& r : *regs) {
            if (r.patientId != m_patientId) continue;
            events.push_back({
                r.admissionDate, "registration", "🏨",
                QString("%1 Admission").arg(r.type.toUpper()),
                QString("Ward: %1  |  Bed: %2  |  Reason: %3")
                    .arg(r.ward, r.bedNumber, r.reason),
                r.status,
                QColor(r.status == "admitted" ? "#38A169" : "#718096")
            });
            if (!r.dischargeDate.isNull() && r.dischargeDate.isValid() &&
                r.dischargeDate.date().year() > 2000) {
                events.push_back({
                    r.dischargeDate, "discharge", "🏠",
                    "Patient Discharged",
                    "From ward: " + r.ward,
                    "discharged",
                    QColor("#718096")
                });
            }
        }
    }

    // Appointment events
    auto appts = HospitalController::instance().appointments().findByPatient(m_patientId);
    if (appts) {
        for (const auto& a : *appts) {
            QColor col;
            if      (a.status == "completed")   col = QColor("#38A169");
            else if (a.status == "cancelled")   col = QColor("#E53E3E");
            else if (a.status == "no-show")     col = QColor("#718096");
            else                                col = QColor("#3182CE");

            events.push_back({
                QDateTime(a.appointmentDate, a.appointmentTime),
                "appointment", "📅",
                QString("Appointment — %1").arg(a.type.toUpper()),
                QString("Dr. %1  |  %2  |  %3 min")
                    .arg(a.doctorName, a.departmentName)
                    .arg(a.durationMin),
                a.status, col
            });
        }
    }

    // Medical record events
    auto records = HospitalController::instance().medicalRecords().findByPatient(m_patientId);
    if (records) {
        for (const auto& r : *records) {
            QString detail = "Diagnosis: " + r.diagnosis;
            if (!r.prescription.isEmpty())
                detail += "\nPrescription: " + r.prescription.left(80)
                        + (r.prescription.length() > 80 ? "…" : "");
            if (r.followUpDate.isValid())
                detail += "\nFollow-up: " + r.followUpDate.toString("yyyy-MM-dd");

            events.push_back({
                r.createdAt, "record", "📋",
                "Medical Record",
                detail,
                "",
                QColor("#805AD5")
            });
        }
    }

    // Billing events
    auto bills = HospitalController::instance().billing().findByPatient(m_patientId);
    if (bills) {
        for (const auto& b : *bills) {
            QColor col = (b.status == "paid")    ? QColor("#38A169")
                       : (b.status == "pending") ? QColor("#E53E3E")
                       : QColor("#D69E2E");

            events.push_back({
                b.createdAt, "billing", "💳",
                QString("Invoice — %1").arg(b.invoiceNumber),
                QString("Total: $%1  |  Paid: $%2  |  Balance: $%3")
                    .arg(b.total, 0,'f',2)
                    .arg(b.paidAmount, 0,'f',2)
                    .arg(b.balance(), 0,'f',2),
                b.status, col
            });
        }
    }

    // ── Sort descending (newest first) ────────────────────────────────────────
    std::sort(events.begin(), events.end(),
        [](const TimelineEvent& a, const TimelineEvent& b) {
            return a.timestamp > b.timestamp;
        });

    // ── Render ────────────────────────────────────────────────────────────────
    if (events.empty()) {
        auto* emptyLbl = new QLabel("No events found for this patient.", m_timelineWidget);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet("color:#A0AEC0;font-size:14px;padding:40px;");
        m_timelineLayout->addWidget(emptyLbl);
    } else {
        // Summary count bar
        auto* summaryLbl = new QLabel(QString(
            "📊  %1 total events  —  "
            "📅 %2 appointments  |  "
            "📋 %3 records  |  "
            "💳 %4 invoices  |  "
            "🏨 %5 admissions")
            .arg(events.size())
            .arg(std::count_if(events.begin(),events.end(),[](const TimelineEvent& e){return e.type=="appointment";}))
            .arg(std::count_if(events.begin(),events.end(),[](const TimelineEvent& e){return e.type=="record";}))
            .arg(std::count_if(events.begin(),events.end(),[](const TimelineEvent& e){return e.type=="billing";}))
            .arg(std::count_if(events.begin(),events.end(),[](const TimelineEvent& e){return e.type=="registration";})),
            m_timelineWidget);
        summaryLbl->setStyleSheet("color:#4A5568;font-size:12px;padding:6px 0 12px 0;");
        m_timelineLayout->addWidget(summaryLbl);

        for (const auto& ev : events)
            addEventCard(ev);
    }

    m_timelineLayout->addStretch();
}

void PatientTimelineView::addEventCard(const TimelineEvent& ev) {
    // ── Row: timeline dot + card ──────────────────────────────────────────────
    auto* row = new QHBoxLayout;
    row->setSpacing(0);
    row->setContentsMargins(0, 0, 0, 0);

    // Left: vertical line + dot
    auto* lineWidget = new QWidget(m_timelineWidget);
    lineWidget->setFixedWidth(48);
    auto* lineLayout = new QVBoxLayout(lineWidget);
    lineLayout->setContentsMargins(0, 0, 0, 0);
    lineLayout->setSpacing(0);

    // Dot
    auto* dot = new QLabel(ev.icon, lineWidget);
    dot->setFixedSize(36, 36);
    dot->setAlignment(Qt::AlignCenter);
    dot->setStyleSheet(QString(
        "background:%1;border-radius:18px;font-size:16px;"
        "border:3px solid white;"
    ).arg(ev.color.name()));

    auto* dotRow = new QHBoxLayout;
    dotRow->addStretch();
    dotRow->addWidget(dot);
    dotRow->setContentsMargins(0, 6, 0, 0);
    lineLayout->addLayout(dotRow);

    // Vertical line
    auto* line = new QFrame(lineWidget);
    line->setFrameShape(QFrame::VLine);
    line->setStyleSheet("color:#E2E8F0;");
    auto* lineRow = new QHBoxLayout;
    lineRow->addStretch();
    lineRow->addWidget(line);
    lineRow->addStretch();
    lineLayout->addLayout(lineRow);

    row->addWidget(lineWidget);

    // Right: event card
    auto* card = new QWidget(m_timelineWidget);
    card->setStyleSheet(R"(
        QWidget {
            background:#FFFFFF;
            border-radius:10px;
            border:1px solid #E2E8F0;
            border-left: 3px solid )" + ev.color.name() + R"(;
        }
    )");
    card->setContentsMargins(0,0,0,0);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 10, 14, 10);
    cardLayout->setSpacing(4);

    // Title row
    auto* titleRow = new QHBoxLayout;
    auto* titleLbl = new QLabel(ev.title, card);
    titleLbl->setStyleSheet(QString(
        "font-size:13px;font-weight:700;color:%1;background:transparent;border:none;"
    ).arg(ev.color.name()));

    auto* timeLbl = new QLabel(ev.timestamp.toString("MMM d, yyyy  hh:mm AP"), card);
    timeLbl->setStyleSheet("font-size:11px;color:#A0AEC0;background:transparent;border:none;");

    titleRow->addWidget(titleLbl);
    titleRow->addStretch();
    titleRow->addWidget(timeLbl);
    cardLayout->addLayout(titleRow);

    // Detail text
    if (!ev.detail.isEmpty()) {
        auto* detailLbl = new QLabel(ev.detail, card);
        detailLbl->setWordWrap(true);
        detailLbl->setStyleSheet(
            "font-size:12px;color:#4A5568;background:transparent;border:none;");
        cardLayout->addWidget(detailLbl);
    }

    // Status badge
    if (!ev.status.isEmpty()) {
        auto* statusRow = new QHBoxLayout;
        auto* badge = new QLabel(ev.status.toUpper(), card);
        badge->setStyleSheet(QString(
            "background:%1;color:white;border-radius:8px;font-size:10px;"
            "font-weight:700;padding:2px 8px;border:none;"
        ).arg(ev.color.name()));
        badge->setFixedHeight(18);
        statusRow->addWidget(badge);
        statusRow->addStretch();
        cardLayout->addLayout(statusRow);
    }

    auto* cardWrapper = new QWidget(m_timelineWidget);
    auto* wl = new QVBoxLayout(cardWrapper);
    wl->setContentsMargins(8, 4, 4, 4);
    wl->addWidget(card);

    row->addWidget(cardWrapper, 1);

    auto* rowWidget = new QWidget(m_timelineWidget);
    rowWidget->setLayout(row);
    rowWidget->setMinimumHeight(80);
    rowWidget->setStyleSheet("background:transparent;");

    m_timelineLayout->addWidget(rowWidget);
}

} // namespace HMS
