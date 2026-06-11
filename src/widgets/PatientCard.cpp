#include "PatientCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>

namespace HMS {

PatientCard::PatientCard(const Patient& patient, QWidget* parent)
    : QWidget(parent), m_patient(patient) {
    setupUi();
    updateDisplay();
    setCursor(Qt::PointingHandCursor);
}

void PatientCard::setupUi() {
    setFixedHeight(90);
    setStyleSheet(R"(
        PatientCard {
            background:#FFFFFF;
            border-radius:10px;
            border:1px solid #E2E8F0;
        }
        PatientCard:hover {
            border-color:#3182CE;
            background:#EBF8FF;
        }
    )");

    auto* main = new QHBoxLayout(this);
    main->setContentsMargins(12, 10, 12, 10);
    main->setSpacing(12);

    // Avatar circle
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(52, 52);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet(
        "background:#3182CE;border-radius:26px;color:white;"
        "font-size:18px;font-weight:700;");
    main->addWidget(m_avatarLabel);

    // Info column
    auto* info = new QVBoxLayout;
    info->setSpacing(2);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-size:14px;font-weight:700;color:#1A202C;");

    m_idLabel = new QLabel(this);
    m_idLabel->setStyleSheet("font-size:11px;color:#718096;");

    m_ageGenderLabel = new QLabel(this);
    m_ageGenderLabel->setStyleSheet("font-size:12px;color:#4A5568;");

    info->addWidget(m_nameLabel);
    info->addWidget(m_idLabel);
    info->addWidget(m_ageGenderLabel);
    main->addLayout(info, 1);

    // Right: blood type + status
    auto* right = new QVBoxLayout;
    right->setSpacing(4);
    right->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_bloodTypeLabel = new QLabel(this);
    m_bloodTypeLabel->setStyleSheet(
        "background:#EBF8FF;color:#2B6CB0;border-radius:10px;"
        "padding:2px 8px;font-size:11px;font-weight:700;");
    m_bloodTypeLabel->setAlignment(Qt::AlignCenter);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    right->addWidget(m_bloodTypeLabel);
    right->addWidget(m_statusLabel);
    main->addLayout(right);
}

void PatientCard::setPatient(const Patient& patient) {
    m_patient = patient;
    updateDisplay();
}

void PatientCard::updateDisplay() {
    // Avatar initials
    QString initials = (m_patient.firstName.isEmpty() ? QStringLiteral("?") : m_patient.firstName.left(1))
                     + (m_patient.lastName.isEmpty()  ? QStringLiteral("?") : m_patient.lastName.left(1));
    m_avatarLabel->setText(initials.toUpper());

    m_nameLabel->setText(m_patient.fullName());
    m_idLabel->setText("ID: " + m_patient.patientUid);
    m_ageGenderLabel->setText(
        QString("%1 years  •  %2").arg(m_patient.age()).arg(m_patient.gender));
    m_bloodTypeLabel->setText(m_patient.bloodType.isEmpty() ? "?" : m_patient.bloodType);

    QColor statusColor = (m_patient.status == "active")   ? QColor("#38A169")
                       : (m_patient.status == "discharged")? QColor("#718096")
                       : QColor("#E53E3E");
    m_statusLabel->setText(m_patient.status.toUpper());
    m_statusLabel->setStyleSheet(QString(
        "background:%1;color:white;border-radius:8px;"
        "padding:2px 8px;font-size:10px;font-weight:700;")
        .arg(statusColor.name()));
}

void PatientCard::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton)
        emit clicked(m_patient.id);
    QWidget::mousePressEvent(ev);
}

void PatientCard::enterEvent(QEnterEvent* ev) {
    setStyleSheet(R"(
        PatientCard { background:#EBF8FF;border-radius:10px;border:1.5px solid #3182CE; }
    )");
    QWidget::enterEvent(ev);
}

void PatientCard::leaveEvent(QEvent* ev) {
    setStyleSheet(R"(
        PatientCard { background:#FFFFFF;border-radius:10px;border:1px solid #E2E8F0; }
    )");
    QWidget::leaveEvent(ev);
}

} // namespace HMS
