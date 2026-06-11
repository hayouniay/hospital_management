#include "RegistrationView.h"
#include "core/HospitalController.h"
#include "core/RegistrationController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>

namespace HMS {

RegistrationView::RegistrationView(QWidget* parent) : QWidget(parent) { setupUi(); }

void RegistrationView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search patients...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn       = new QPushButton("+ Admit Patient", this);
    m_dischargeBtn = new QPushButton("🏠 Discharge", this);
    m_deleteBtn    = new QPushButton("🗑 Delete", this);
    m_dischargeBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_dischargeBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    tb->addWidget(m_searchEdit); tb->addStretch();
    tb->addWidget(m_dischargeBtn);
    tb->addWidget(m_deleteBtn);
    tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "ID","Patient","Type","Department","Doctor","Ward","Bed","Admitted","Status"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &RegistrationView::onSearch);
    connect(m_addBtn,       &QPushButton::clicked, this, &RegistrationView::onAdd);
    connect(m_dischargeBtn, &QPushButton::clicked, this, &RegistrationView::onDischarge);
    connect(m_deleteBtn,    &QPushButton::clicked, this, &RegistrationView::onDelete);
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [this]() {
            bool sel = m_table->currentRow() >= 0;
            m_dischargeBtn->setEnabled(sel);
            m_deleteBtn->setEnabled(sel);
        });
}

void RegistrationView::refresh() { loadData(); }

void RegistrationView::loadData() {
    auto r = HospitalController::instance().registrations().findAll();
    if (r) { m_registrations = *r; populateTable(m_registrations); }
}

void RegistrationView::populateTable(const std::vector<Registration>& regs) {
    m_table->setRowCount(0);
    for (const auto& reg : regs) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(reg.id)));
        m_table->setItem(row, 1, new QTableWidgetItem(reg.patientName));
        m_table->setItem(row, 2, new QTableWidgetItem(reg.type));
        m_table->setItem(row, 3, new QTableWidgetItem(reg.departmentName));
        m_table->setItem(row, 4, new QTableWidgetItem(reg.doctorName));
        m_table->setItem(row, 5, new QTableWidgetItem(reg.ward));
        m_table->setItem(row, 6, new QTableWidgetItem(reg.bedNumber));
        m_table->setItem(row, 7, new QTableWidgetItem(reg.admissionDate.toString("yyyy-MM-dd hh:mm")));

        auto* statusItem = new QTableWidgetItem(reg.status);
        if      (reg.status == "admitted")   statusItem->setForeground(QColor("#38A169"));
        else if (reg.status == "discharged") statusItem->setForeground(QColor("#718096"));
        else if (reg.status == "transferred")statusItem->setForeground(QColor("#D69E2E"));
        m_table->setItem(row, 8, statusItem);
        m_table->item(row, 0)->setData(Qt::UserRole, reg.id);
    }
    m_table->resizeColumnsToContents();
}

void RegistrationView::onAdd() { showDialog(); }

void RegistrationView::onDischarge() {
    qint64 id = selectedId();
    if (id <= 0) return;
    if (QMessageBox::question(this,"Discharge","Mark this patient as discharged?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        auto r = HospitalController::instance().registrations().discharge(id);
        if (r) refresh();
        else QMessageBox::critical(this,"Error",r.error());
    }
}

void RegistrationView::onDelete() {
    qint64 id = selectedId();
    if (id <= 0) return;
    if (QMessageBox::question(this,"Confirm","Delete this registration record?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        auto r = HospitalController::instance().registrations().remove(id);
        if (r) refresh();
        else QMessageBox::critical(this,"Error",r.error());
    }
}

void RegistrationView::onSearch(const QString& /*text*/) { loadData(); }

qint64 RegistrationView::selectedId() const {
    int row = m_table->currentRow();
    if (row < 0) return -1;
    auto* item = m_table->item(row, 0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void RegistrationView::showDialog(std::optional<Registration> /*reg*/) {
    QDialog dlg(this);
    dlg.setWindowTitle("Admit Patient");
    dlg.setMinimumWidth(520);
    auto* layout = new QVBoxLayout(&dlg);

    auto* group = new QGroupBox("Admission Details", &dlg);
    auto* form = new QFormLayout(group);

    // Patient
    auto* patientCombo = new QComboBox(&dlg);
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients)
        for (const auto& p : *patients)
            patientCombo->addItem(p.patientUid + " — " + p.fullName(), p.id);

    // Doctor
    auto* doctorCombo = new QComboBox(&dlg);
    doctorCombo->addItem("-- None --", QVariant((qint64)0));
    auto doctors = HospitalController::instance().doctors().findAll(200);
    if (doctors)
        for (const auto& d : *doctors)
            doctorCombo->addItem("Dr. " + d.fullName(), d.id);

    // Department
    auto* deptCombo = new QComboBox(&dlg);
    deptCombo->addItem("-- None --", QVariant((qint64)0));
    auto depts = HospitalController::instance().doctors().allDepartments();
    if (depts)
        for (const auto& d : *depts)
            deptCombo->addItem(d.name, d.id);

    auto* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"outpatient","inpatient","emergency","icu"});

    auto* wardEdit    = new QLineEdit(&dlg);
    wardEdit->setPlaceholderText("e.g. Ward A");
    auto* bedEdit     = new QLineEdit(&dlg);
    bedEdit->setPlaceholderText("e.g. B-12");
    auto* reasonEdit  = new QTextEdit(&dlg);
    reasonEdit->setMaximumHeight(70);
    reasonEdit->setPlaceholderText("Reason for admission...");

    form->addRow("Patient *",     patientCombo);
    form->addRow("Admitting Doctor", doctorCombo);
    form->addRow("Department",    deptCombo);
    form->addRow("Type",          typeCombo);
    form->addRow("Ward",          wardEdit);
    form->addRow("Bed Number",    bedEdit);
    form->addRow("Reason",        reasonEdit);
    layout->addWidget(group);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton("🏥 Admit",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (patientCombo->count() == 0) {
            QMessageBox::warning(&dlg,"Validation","No patients available. Please register a patient first.");
            return;
        }
        Registration reg;
        reg.patientId       = patientCombo->currentData().toLongLong();
        reg.admittingDoctor = doctorCombo->currentData().toLongLong();
        reg.departmentId    = deptCombo->currentData().toLongLong();
        reg.type            = typeCombo->currentText();
        reg.ward            = wardEdit->text().trimmed();
        reg.bedNumber       = bedEdit->text().trimmed();
        reg.reason          = reasonEdit->toPlainText();
        reg.status          = "admitted";

        auto r = HospitalController::instance().registrations().create(reg);
        if (r) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",r.error());
    });

    dlg.exec();
}

} // namespace HMS
