#include "GlobalSearchBar.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QKeyEvent>

namespace HMS {

GlobalSearchBar::GlobalSearchBar(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void GlobalSearchBar::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText("🔍  Global search — patients, doctors, appointments...");
    m_edit->setFixedHeight(34);
    m_edit->setMinimumWidth(340);
    m_edit->setStyleSheet(R"(
        QLineEdit {
            background: rgba(255,255,255,0.12);
            border: 1px solid rgba(255,255,255,0.25);
            border-radius: 17px;
            padding: 0 14px 0 14px;
            font-size: 13px;
            color: #E2E8F0;
        }
        QLineEdit:focus {
            background: rgba(255,255,255,0.18);
            border-color: rgba(255,255,255,0.5);
        }
        QLineEdit::placeholder { color: rgba(255,255,255,0.5); }
    )");
    layout->addWidget(m_edit);

    // ── Popup results ─────────────────────────────────────────────────────────
    m_popup = new QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint);
    m_popup->setFixedWidth(440);
    m_popup->setMaximumHeight(400);
    m_popup->setStyleSheet(R"(
        QFrame { background:#FFFFFF; border:1px solid #E2E8F0; border-radius:10px; }
    )");
    m_popup->hide();

    auto* popLayout = new QVBoxLayout(m_popup);
    popLayout->setContentsMargins(6, 6, 6, 6);
    popLayout->setSpacing(0);

    m_resultList = new QListWidget(m_popup);
    m_resultList->setFrameShape(QFrame::NoFrame);
    m_resultList->setStyleSheet(R"(
        QListWidget { background:transparent; border:none; }
        QListWidget::item { padding:10px 12px; border-radius:6px; margin:1px; }
        QListWidget::item:hover { background:#EBF8FF; }
        QListWidget::item:selected { background:#3182CE; color:white; }
    )");
    popLayout->addWidget(m_resultList);

    // ── Debounce timer ────────────────────────────────────────────────────────
    m_debounce = new QTimer(this);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(250);

    connect(m_edit,     &QLineEdit::textChanged, this, &GlobalSearchBar::onTextChanged);
    connect(m_debounce, &QTimer::timeout,        this, &GlobalSearchBar::performSearch);
    connect(m_resultList, &QListWidget::itemClicked, this, &GlobalSearchBar::onResultClicked);
}

void GlobalSearchBar::onTextChanged(const QString& text) {
    if (text.length() < 2) { hideResults(); return; }
    m_debounce->start();
}

void GlobalSearchBar::performSearch() {
    const QString q = m_edit->text().trimmed();
    if (q.length() < 2) { hideResults(); return; }

    m_resultList->clear();

    auto addSection = [&](const QString& title) {
        auto* item = new QListWidgetItem(title, m_resultList);
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(QColor("#A0AEC0"));
        QFont f; f.setPointSize(9); f.setBold(true); item->setFont(f);
    };

    // Patients
    auto patients = HospitalController::instance().patients().search(q);
    if (patients && !patients->empty()) {
        addSection("  👥  PATIENTS");
        for (const auto& p : *patients) {
            auto* item = new QListWidgetItem(
                QString("  %1  —  %2  (%3 yrs)").arg(p.patientUid, p.fullName()).arg(p.age()),
                m_resultList);
            item->setData(Qt::UserRole,     1);  // page: patients
            item->setData(Qt::UserRole + 1, p.id);
            item->setIcon(QIcon());
        }
    }

    // Doctors
    auto doctors = HospitalController::instance().doctors().search(q);
    if (doctors && !doctors->empty()) {
        addSection("  🩺  DOCTORS");
        for (const auto& d : *doctors) {
            auto* item = new QListWidgetItem(
                QString("  Dr. %1  —  %2").arg(d.fullName(), d.specialization),
                m_resultList);
            item->setData(Qt::UserRole,     2);  // page: doctors
            item->setData(Qt::UserRole + 1, d.id);
        }
    }

    // Appointments (today)
    auto appts = HospitalController::instance().appointments().findAll(50);
    if (appts) {
        bool headerAdded = false;
        for (const auto& a : *appts) {
            if (!a.patientName.contains(q, Qt::CaseInsensitive) &&
                !a.doctorName.contains(q, Qt::CaseInsensitive)) continue;
            if (!headerAdded) { addSection("  📅  APPOINTMENTS"); headerAdded = true; }
            auto* item = new QListWidgetItem(
                QString("  %1  —  Dr.%2  (%3)")
                    .arg(a.patientName, a.doctorName, a.appointmentDate.toString("yyyy-MM-dd")),
                m_resultList);
            item->setData(Qt::UserRole, 3); // page: appointments
            item->setData(Qt::UserRole + 1, a.id);
        }
    }

    if (m_resultList->count() == 0) {
        auto* item = new QListWidgetItem("  No results found for: " + q, m_resultList);
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(QColor("#A0AEC0"));
    }

    showResults();
}

void GlobalSearchBar::onResultClicked(QListWidgetItem* item) {
    int page = item->data(Qt::UserRole).toInt();
    if (page > 0) {
        hideResults();
        m_edit->clear();
        emit navigateTo(page);
    }
}

void GlobalSearchBar::showResults() {
    QPoint pos = m_edit->mapToGlobal(QPoint(0, m_edit->height() + 4));
    m_popup->move(pos);
    m_popup->adjustSize();
    m_resultList->setMaximumHeight(
        qMin(m_resultList->count() * 36 + 12, 380));
    m_popup->adjustSize();
    m_popup->show();
    m_popup->raise();
}

void GlobalSearchBar::hideResults() {
    m_popup->hide();
}

void GlobalSearchBar::focusSearch() {
    m_edit->setFocus();
    m_edit->selectAll();
}

} // namespace HMS
