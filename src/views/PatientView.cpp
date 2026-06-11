#include "PatientView.h"
#include "core/HospitalController.h"
#include "core/MedicalRecordController.h"
#include "utils/PDFExporter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>

namespace HMS {

PatientView::PatientView(QWidget* parent) : QWidget(parent) { setupUi(); }

void PatientView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    // Toolbar
    auto* toolbar = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search by name, ID, phone, email...");
    m_searchEdit->setFixedHeight(36);

    m_addBtn    = new QPushButton("+ Add Patient", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_qrBtn     = new QPushButton("📷 QR Code", this);
    m_reportBtn = new QPushButton("📄 Export Report", this);

    m_deleteBtn->setObjectName("dangerBtn");
    m_editBtn->setObjectName("secondaryBtn");
    m_qrBtn->setObjectName("secondaryBtn");
    m_reportBtn->setObjectName("secondaryBtn");

    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    m_qrBtn->setEnabled(false);
    m_reportBtn->setEnabled(false);

    toolbar->addWidget(m_searchEdit);
    toolbar->addStretch();
    toolbar->addWidget(m_reportBtn);
    toolbar->addWidget(m_qrBtn);
    toolbar->addWidget(m_editBtn);
    toolbar->addWidget(m_deleteBtn);
    toolbar->addWidget(m_addBtn);
    layout->addLayout(toolbar);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "Patient ID","Name","Age","Gender","Blood Type",
        "Phone","Email","Status","Registered"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &PatientView::onSearchChanged);
    connect(m_addBtn,     &QPushButton::clicked,   this, &PatientView::onAddPatient);
    connect(m_editBtn,    &QPushButton::clicked,   this, &PatientView::onEditPatient);
    connect(m_deleteBtn,  &QPushButton::clicked,   this, &PatientView::onDeletePatient);
    connect(m_qrBtn,      &QPushButton::clicked,   this, &PatientView::onViewQR);
    connect(m_reportBtn,  &QPushButton::clicked,   this, &PatientView::onExportReport);
    connect(m_table, &QTableWidget::doubleClicked, this, [this]() { onEditPatient(); });
    connect(m_table, &QTableWidget::customContextMenuRequested, this,
        [this](const QPoint& pos) {
            if (m_table->currentRow() < 0) return;
            QMenu menu(this);
            menu.addAction("✏  Edit Patient",    this, &PatientView::onEditPatient);
            menu.addAction("📄  Export Report",  this, &PatientView::onExportReport);
            menu.addAction("📷  View QR Code",   this, &PatientView::onViewQR);
            menu.addSeparator();
            menu.addAction("🗑  Delete Patient", this, &PatientView::onDeletePatient);
            menu.exec(m_table->viewport()->mapToGlobal(pos));
        });
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [this]() {
            bool sel = m_table->currentRow() >= 0;
            m_editBtn->setEnabled(sel);
            m_deleteBtn->setEnabled(sel);
            m_qrBtn->setEnabled(sel);
            m_reportBtn->setEnabled(sel);
        });
}

void PatientView::refresh() { loadPatients(m_searchEdit->text()); }

void PatientView::loadPatients(const QString& search) {
    auto& ctrl = HospitalController::instance().patients();
    auto r = search.isEmpty() ? ctrl.findAll(200) : ctrl.search(search);
    if (r) { m_patients = *r; populateTable(m_patients); }
}

void PatientView::populateTable(const std::vector<Patient>& patients) {
    m_table->setRowCount(0);
    for (const auto& p : patients) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row,0,new QTableWidgetItem(p.patientUid));
        m_table->setItem(row,1,new QTableWidgetItem(p.fullName()));
        m_table->setItem(row,2,new QTableWidgetItem(QString::number(p.age())));
        m_table->setItem(row,3,new QTableWidgetItem(p.gender));
        m_table->setItem(row,4,new QTableWidgetItem(p.bloodType));
        m_table->setItem(row,5,new QTableWidgetItem(p.phone));
        m_table->setItem(row,6,new QTableWidgetItem(p.email));
        auto* statusItem = new QTableWidgetItem(p.status);
        if      (p.status=="active")    statusItem->setForeground(QColor("#38A169"));
        else if (p.status=="discharged")statusItem->setForeground(QColor("#718096"));
        else                            statusItem->setForeground(QColor("#E53E3E"));
        m_table->setItem(row,7,statusItem);
        m_table->setItem(row,8,new QTableWidgetItem(p.createdAt.toString("yyyy-MM-dd")));
        m_table->item(row,0)->setData(Qt::UserRole, p.id);
    }
    m_table->resizeColumnsToContents();
}

void PatientView::onSearchChanged(const QString& text) { loadPatients(text); }
void PatientView::onAddPatient()  { showPatientDialog(); }
void PatientView::onEditPatient() {
    qint64 id = selectedPatientId();
    if (id<=0) return;
    auto r = HospitalController::instance().patients().findById(id);
    if (r) showPatientDialog(*r);
}
void PatientView::onDeletePatient() {
    qint64 id = selectedPatientId();
    if (id<=0) return;
    if (QMessageBox::question(this,"Confirm Delete",
            "Are you sure you want to delete this patient?\nThis cannot be undone.",
            QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        auto r = HospitalController::instance().patients().remove(id);
        if (r) refresh();
        else QMessageBox::critical(this,"Error",r.error());
    }
}

void PatientView::onViewQR() {
    qint64 id = selectedPatientId();
    if (id<=0) return;
    auto result = HospitalController::instance().patients().findById(id);
    if (!result) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Patient QR Code — " + result->patientUid);
    dlg.setFixedSize(320,280);
    auto* layout = new QVBoxLayout(&dlg);

    auto* titleLbl = new QLabel(result->fullName(), &dlg);
    titleLbl->setAlignment(Qt::AlignCenter);
    titleLbl->setStyleSheet("font-size:14px;font-weight:700;color:#2B6CB0;");
    layout->addWidget(titleLbl);

    auto* idLbl = new QLabel("ID: " + result->patientUid, &dlg);
    idLbl->setAlignment(Qt::AlignCenter);
    idLbl->setStyleSheet("font-size:11px;color:#718096;");
    layout->addWidget(idLbl);

    auto* qrBox = new QLabel(&dlg);
    qrBox->setFixedSize(180,180);
    qrBox->setAlignment(Qt::AlignCenter);
    qrBox->setStyleSheet("border:2px solid #3182CE;border-radius:8px;background:#F7FAFC;padding:8px;");
    qrBox->setText("█▀▀▀█ ▄ ▄ █▀▀▀█\n█ ▄ █ ▄█▄ █ ▄ █\n█▄▄▄█ ▄ █ █▄▄▄█\n      ▄▄▄▄ ▄\n▄█▄▄▄ █▀▀ ▄█▄\n█ █▀█ ▄ ▄ █ █\n█▄▄▄█ █▀█ █▄▄\n\n[QR: " + result->patientUid+"]");
    qrBox->setWordWrap(true);
    auto* ql = new QHBoxLayout;
    ql->addStretch();
    ql->addWidget(qrBox);
    ql->addStretch();
    layout->addLayout(ql);

    auto* closeBtn = new QPushButton("Close",&dlg);
    connect(closeBtn,&QPushButton::clicked,&dlg,&QDialog::accept);
    layout->addWidget(closeBtn);
    dlg.exec();
}

// ── Export consultation PDF ───────────────────────────────────────────────────
void PatientView::onExportReport() {
    qint64 id = selectedPatientId();
    if (id<=0) return;

    auto patResult = HospitalController::instance().patients().findById(id);
    if (!patResult) { QMessageBox::critical(this,"Error","Patient not found."); return; }

    auto recResult = HospitalController::instance().medicalRecords().findByPatient(id);
    if (!recResult) { QMessageBox::critical(this,"Error",recResult.error()); return; }

    auto& patient = *patResult;
    auto& records = *recResult;

    if (records.empty()) {
        auto reply = QMessageBox::question(this, "No Records",
            "No consultation records found for this patient.\n"
            "Export a report with just patient info?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;
    }

    auto result = PDFExporter::exportConsultationReportDialog(patient, records, this);
    if (!result && result.error() != "Cancelled") {
        QMessageBox::critical(this, "Export Failed", result.error());
    } else if (result) {
        QMessageBox::information(this, "Export Complete",
            "Consultation report saved and opened:\n" + *result);
    }
}

qint64 PatientView::selectedPatientId() const {
    int row = m_table->currentRow();
    if (row<0) return -1;
    auto* item = m_table->item(row,0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void PatientView::showPatientDialog(std::optional<Patient> patient) {
    QDialog dlg(this);
    bool isEdit = patient.has_value();
    dlg.setWindowTitle(isEdit ? "Edit Patient" : "Register New Patient");
    dlg.setMinimumWidth(560);

    auto* mainLayout = new QVBoxLayout(&dlg);
    auto* scroll = new QScrollArea(&dlg);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* form = new QWidget;
    auto* formLayout = new QFormLayout(form);
    formLayout->setSpacing(10);

    auto* personalGroup = new QGroupBox("Personal Information",form);
    auto* pf = new QFormLayout(personalGroup);
    auto* firstNameEdit = new QLineEdit(form);
    auto* lastNameEdit  = new QLineEdit(form);
    auto* dobEdit       = new QDateEdit(form);
    dobEdit->setCalendarPopup(true);
    dobEdit->setDate(QDate::currentDate().addYears(-30));
    dobEdit->setDisplayFormat("yyyy-MM-dd");
    auto* genderCombo   = new QComboBox(form);
    genderCombo->addItems({"Male","Female","Other"});
    auto* bloodTypeCombo = new QComboBox(form);
    bloodTypeCombo->addItems({"A+","A-","B+","B-","AB+","AB-","O+","O-","Unknown"});
    pf->addRow("First Name *", firstNameEdit);
    pf->addRow("Last Name",    lastNameEdit);
    pf->addRow("Date of Birth",dobEdit);
    pf->addRow("Gender",       genderCombo);
    pf->addRow("Blood Type",   bloodTypeCombo);

    auto* contactGroup = new QGroupBox("Contact Information",form);
    auto* cf = new QFormLayout(contactGroup);
    auto* phoneEdit   = new QLineEdit(form);
    auto* emailEdit   = new QLineEdit(form);
    auto* addressEdit = new QLineEdit(form);
    auto* ecEdit      = new QLineEdit(form);
    auto* epEdit      = new QLineEdit(form);
    cf->addRow("Phone",             phoneEdit);
    cf->addRow("Email",             emailEdit);
    cf->addRow("Address",           addressEdit);
    cf->addRow("Emergency Contact", ecEdit);
    cf->addRow("Emergency Phone",   epEdit);

    auto* insGroup    = new QGroupBox("Insurance & Status",form);
    auto* inf         = new QFormLayout(insGroup);
    auto* insIdEdit   = new QLineEdit(form);
    auto* statusCombo = new QComboBox(form);
    statusCombo->addItems({"active","discharged","deceased"});
    inf->addRow("Insurance ID", insIdEdit);
    inf->addRow("Status",       statusCombo);

    formLayout->addRow(personalGroup);
    formLayout->addRow(contactGroup);
    formLayout->addRow(insGroup);

    if (isEdit) {
        auto& p = *patient;
        firstNameEdit->setText(p.firstName); lastNameEdit->setText(p.lastName);
        dobEdit->setDate(p.dateOfBirth); genderCombo->setCurrentText(p.gender);
        bloodTypeCombo->setCurrentText(p.bloodType); phoneEdit->setText(p.phone);
        emailEdit->setText(p.email); addressEdit->setText(p.address);
        ecEdit->setText(p.emergencyContact); epEdit->setText(p.emergencyPhone);
        insIdEdit->setText(p.insuranceId); statusCombo->setCurrentText(p.status);
    }

    scroll->setWidget(form);
    mainLayout->addWidget(scroll);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit?"💾 Update":"💾 Register",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    mainLayout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (firstNameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg,"Validation","First name is required.");
            return;
        }
        Patient p = patient.value_or(Patient{});
        p.firstName       = firstNameEdit->text().trimmed();
        p.lastName        = lastNameEdit->text().trimmed();
        p.dateOfBirth     = dobEdit->date();
        p.gender          = genderCombo->currentText();
        p.bloodType       = bloodTypeCombo->currentText();
        p.phone           = phoneEdit->text().trimmed();
        p.email           = emailEdit->text().trimmed();
        p.address         = addressEdit->text().trimmed();
        p.emergencyContact= ecEdit->text().trimmed();
        p.emergencyPhone  = epEdit->text().trimmed();
        p.insuranceId     = insIdEdit->text().trimmed();
        p.status          = statusCombo->currentText();

        auto& ctrl = HospitalController::instance().patients();
        auto result = isEdit ? ctrl.update(p) : ctrl.create(p);
        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",result.error());
    });
    dlg.exec();
}

void PatientView::applyRolesToUi() {
    bool canModify = AuthController::instance().hasRole(UserRole::Admin)
                  || AuthController::instance().hasRole(UserRole::Receptionist);
    m_addBtn->setVisible(canModify);
    m_deleteBtn->setVisible(canModify);
}

} // namespace HMS
