#include "utils/PDFExporter.h"
#include "core/MedicalRecordController.h"
#include "MedicalRecordView.h"
#include "core/HospitalController.h"
#include "core/MedicalRecordController.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>

namespace HMS {

MedicalRecordView::MedicalRecordView(QWidget* parent) : QWidget(parent) { setupUi(); }

void MedicalRecordView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search by patient, diagnosis, prescription...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn    = new QPushButton("+ New Record", this);
    m_viewBtn   = new QPushButton("👁 View", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_viewBtn->setObjectName("secondaryBtn");
    m_exportBtn = new QPushButton("📄 Export Report", this);
    m_exportBtn->setObjectName("secondaryBtn");
    m_exportBtn->setEnabled(false);
    m_editBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_viewBtn->setEnabled(false);
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    tb->addWidget(m_searchEdit); tb->addStretch();
    tb->addWidget(m_exportBtn); tb->addWidget(m_viewBtn); tb->addWidget(m_editBtn);
    tb->addWidget(m_deleteBtn); tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "ID","Patient","Doctor","Diagnosis","Prescription","Follow-Up","Date"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &MedicalRecordView::onSearch);
    connect(m_addBtn,    &QPushButton::clicked, this, &MedicalRecordView::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &MedicalRecordView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MedicalRecordView::onDelete);
    connect(m_viewBtn,   &QPushButton::clicked, this, [this]() { onEdit(); });
    connect(m_exportBtn, &QPushButton::clicked, this, &MedicalRecordView::onExportReport);
    connect(m_table, &QTableWidget::doubleClicked, this, [this]() { onEdit(); });
    connect(m_table, &QTableWidget::customContextMenuRequested, this,
        [this](const QPoint& pos) {
            if (m_table->currentRow() < 0) return;
            QMenu menu(this);
            menu.addAction("✏  Edit",   this, [this](){ onEdit(); });
            menu.addSeparator();
            menu.addAction("🗑  Delete", this, [this](){ onDelete(); });
            menu.exec(m_table->viewport()->mapToGlobal(pos));
        });
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [this]() {
            bool sel = m_table->currentRow() >= 0;
            m_editBtn->setEnabled(sel);
            m_deleteBtn->setEnabled(sel);
            m_viewBtn->setEnabled(sel);
            m_exportBtn->setEnabled(sel);
        });
}

void MedicalRecordView::refresh() { loadData(m_searchEdit->text()); }

void MedicalRecordView::loadData(const QString& search) {
    auto r = search.isEmpty() ? HospitalController::instance().medicalRecords().findAll(300) : HospitalController::instance().medicalRecords().search(search);
    if (r) { m_records = *r; populateTable(m_records); }
}

void MedicalRecordView::populateTable(const std::vector<MedicalRecord>& records) {
    m_table->setRowCount(0);
    for (const auto& rec : records) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row,0,new QTableWidgetItem(QString::number(rec.id)));
        m_table->setItem(row,1,new QTableWidgetItem(rec.patientName));
        m_table->setItem(row,2,new QTableWidgetItem(rec.doctorName));
        // Truncate long text for table display
        QString diag = rec.diagnosis.length()>50 ? rec.diagnosis.left(47)+"..." : rec.diagnosis;
        m_table->setItem(row,3,new QTableWidgetItem(diag));
        QString presc = rec.prescription.length()>40 ? rec.prescription.left(37)+"..." : rec.prescription;
        m_table->setItem(row,4,new QTableWidgetItem(presc));
        m_table->setItem(row,5,new QTableWidgetItem(
            rec.followUpDate.isValid() ? rec.followUpDate.toString("yyyy-MM-dd") : "-"));
        m_table->setItem(row,6,new QTableWidgetItem(rec.createdAt.toString("yyyy-MM-dd")));
        m_table->item(row,0)->setData(Qt::UserRole, rec.id);
    }
    m_table->resizeColumnsToContents();
}

void MedicalRecordView::onAdd()  { showDialog(); }
void MedicalRecordView::onEdit() {
    qint64 id = selectedId();
    if (id<=0) return;
    auto r = HospitalController::instance().medicalRecords().findById(id);
    if (r) showDialog(*r);
}
void MedicalRecordView::onDelete() {
    qint64 id = selectedId();
    if (id<=0) return;
    if (QMessageBox::question(this,"Confirm","Delete this medical record?",
            QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        auto r = HospitalController::instance().medicalRecords().remove(id);
        if (r) refresh(); else QMessageBox::critical(this,"Error",r.error());
    }
}
void MedicalRecordView::onSearch(const QString& text) { loadData(text); }

qint64 MedicalRecordView::selectedId() const {
    int row = m_table->currentRow();
    if (row<0) return -1;
    auto* item = m_table->item(row,0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void MedicalRecordView::showDialog(std::optional<MedicalRecord> rec) {
    bool isEdit = rec.has_value();
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "Edit Medical Record" : "New Medical Record");
    dlg.setMinimumWidth(580);
    dlg.setMinimumHeight(600);

    auto* scroll = new QScrollArea(&dlg);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);

    // Patient & Doctor
    auto* headerGroup = new QGroupBox("Patient & Doctor", content);
    auto* hf = new QFormLayout(headerGroup);
    auto* patientCombo = new QComboBox(content);
    auto* doctorCombo  = new QComboBox(content);
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients) for (const auto& p : *patients)
        patientCombo->addItem(p.patientUid+" — "+p.fullName(), p.id);
    auto doctors = HospitalController::instance().doctors().findAll(200);
    if (doctors) for (const auto& d : *doctors)
        doctorCombo->addItem("Dr. "+d.fullName()+" ("+d.specialization+")", d.id);
    hf->addRow("Patient *", patientCombo);
    hf->addRow("Doctor *",  doctorCombo);

    // Clinical
    auto* clinGroup = new QGroupBox("Clinical Information", content);
    auto* cf = new QFormLayout(clinGroup);
    auto* diagEdit    = new QTextEdit(content); diagEdit->setMaximumHeight(80);
    diagEdit->setPlaceholderText("Primary diagnosis...");
    auto* sympEdit    = new QTextEdit(content); sympEdit->setMaximumHeight(70);
    sympEdit->setPlaceholderText("Symptoms and observations...");
    auto* prescEdit   = new QTextEdit(content); prescEdit->setMaximumHeight(80);
    prescEdit->setPlaceholderText("Medications and dosages...");
    auto* labEdit     = new QTextEdit(content); labEdit->setMaximumHeight(70);
    labEdit->setPlaceholderText("Lab results and test findings...");
    auto* notesEdit   = new QTextEdit(content); notesEdit->setMaximumHeight(60);
    notesEdit->setPlaceholderText("Additional notes...");
    auto* followUpEdit = new QDateEdit(content);
    followUpEdit->setCalendarPopup(true);
    followUpEdit->setDate(QDate::currentDate().addDays(14));
    followUpEdit->setDisplayFormat("yyyy-MM-dd");
    followUpEdit->setSpecialValueText("None");
    followUpEdit->setMinimumDate(QDate(2000,1,1));

    cf->addRow("Diagnosis *",    diagEdit);
    cf->addRow("Symptoms",       sympEdit);
    cf->addRow("Prescription",   prescEdit);
    cf->addRow("Lab Results",    labEdit);
    cf->addRow("Notes",          notesEdit);
    cf->addRow("Follow-up Date", followUpEdit);

    layout->addWidget(headerGroup);
    layout->addWidget(clinGroup);

    if (isEdit) {
        auto& r = *rec;
        int pi = patientCombo->findData(r.patientId);
        if (pi>=0) patientCombo->setCurrentIndex(pi);
        int di = doctorCombo->findData(r.doctorId);
        if (di>=0) doctorCombo->setCurrentIndex(di);
        diagEdit->setPlainText(r.diagnosis);
        sympEdit->setPlainText(r.symptoms);
        prescEdit->setPlainText(r.prescription);
        labEdit->setPlainText(r.labResults);
        notesEdit->setPlainText(r.notes);
        if (r.followUpDate.isValid()) followUpEdit->setDate(r.followUpDate);
    }

    scroll->setWidget(content);

    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->addWidget(scroll);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit?"💾 Update":"💾 Save Record",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    dlgLayout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (diagEdit->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg,"Validation","Diagnosis is required.");
            return;
        }
        MedicalRecord r = rec.value_or(MedicalRecord{});
        r.patientId   = patientCombo->currentData().toLongLong();
        r.doctorId    = doctorCombo->currentData().toLongLong();
        r.diagnosis   = diagEdit->toPlainText().trimmed();
        r.symptoms    = sympEdit->toPlainText();
        r.prescription= prescEdit->toPlainText();
        r.labResults  = labEdit->toPlainText();
        r.notes       = notesEdit->toPlainText();
        r.followUpDate= followUpEdit->date();

        auto result = isEdit ? HospitalController::instance().medicalRecords().update(r) : HospitalController::instance().medicalRecords().create(r);
        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",result.error());
    });
    dlg.exec();
}


void MedicalRecordView::onExportReport() {
    qint64 id = selectedId();
    if (id <= 0) return;

    auto recResult = HospitalController::instance().medicalRecords().findById(id);
    if (!recResult) return;

    qint64 patientId = recResult->patientId;
    auto patResult = HospitalController::instance().patients().findById(patientId);
    if (!patResult) {
        QMessageBox::critical(this, "Error", "Could not load patient data.");
        return;
    }

    // Load ALL records for this patient for the full report
    auto allRecords = HospitalController::instance().medicalRecords().findByPatient(patientId);
    if (!allRecords) { QMessageBox::critical(this, "Error", allRecords.error()); return; }

    auto result = PDFExporter::exportConsultationReportDialog(*patResult, *allRecords, this);
    if (!result && result.error() != "Cancelled") {
        QMessageBox::critical(this, "Export Failed", result.error());
    } else if (result) {
        QMessageBox::information(this, "Export Complete",
            "Consultation report saved and opened:\n" + *result);
    }
}
} // namespace HMS
