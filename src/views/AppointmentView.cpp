#include "AppointmentView.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>

namespace HMS {

AppointmentView::AppointmentView(QWidget* parent) : QWidget(parent) { setupUi(); }

void AppointmentView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Filter by patient, doctor or date...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn    = new QPushButton("+ New Appointment", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_statusBtn = new QPushButton("⚡ Status", this);
    m_editBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_statusBtn->setObjectName("secondaryBtn");
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    m_statusBtn->setEnabled(false);

    tb->addWidget(m_searchEdit);
    tb->addStretch();
    tb->addWidget(m_statusBtn);
    tb->addWidget(m_editBtn);
    tb->addWidget(m_deleteBtn);
    tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "ID","Patient","Doctor","Department","Date","Time","Type","Duration","Status"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &AppointmentView::onSearch);
    connect(m_addBtn,    &QPushButton::clicked, this, &AppointmentView::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &AppointmentView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &AppointmentView::onDelete);
    connect(m_statusBtn, &QPushButton::clicked, this, &AppointmentView::onUpdateStatus);
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
            m_statusBtn->setEnabled(sel);
        });
}

void AppointmentView::refresh() { loadAppointments(m_searchEdit->text()); }

void AppointmentView::loadAppointments(const QString& /*search*/) {
    auto r = HospitalController::instance().appointments().findAll(200);
    if (r) { m_appointments = *r; populateTable(m_appointments); }
}

void AppointmentView::populateTable(const std::vector<Appointment>& appts) {
    m_table->setRowCount(0);
    for (const auto& a : appts) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(a.id)));
        m_table->setItem(row, 1, new QTableWidgetItem(a.patientName));
        m_table->setItem(row, 2, new QTableWidgetItem(a.doctorName));
        m_table->setItem(row, 3, new QTableWidgetItem(a.departmentName));
        m_table->setItem(row, 4, new QTableWidgetItem(a.appointmentDate.toString("yyyy-MM-dd")));
        m_table->setItem(row, 5, new QTableWidgetItem(a.appointmentTime.toString("HH:mm")));
        m_table->setItem(row, 6, new QTableWidgetItem(a.type));
        m_table->setItem(row, 7, new QTableWidgetItem(QString::number(a.durationMin) + " min"));

        auto* statusItem = new QTableWidgetItem(a.status);
        if      (a.status == "completed")  statusItem->setForeground(QColor("#38A169"));
        else if (a.status == "scheduled")  statusItem->setForeground(QColor("#3182CE"));
        else if (a.status == "confirmed")  statusItem->setForeground(QColor("#805AD5"));
        else if (a.status == "cancelled")  statusItem->setForeground(QColor("#E53E3E"));
        else if (a.status == "in-progress")statusItem->setForeground(QColor("#D69E2E"));
        m_table->setItem(row, 8, statusItem);
        m_table->item(row, 0)->setData(Qt::UserRole, a.id);
    }
    m_table->resizeColumnsToContents();
}

void AppointmentView::onAdd()  { showDialog(); }
void AppointmentView::onEdit() {
    qint64 id = selectedId();
    if (id <= 0) return;
    auto r = HospitalController::instance().appointments().findById(id);
    if (r) showDialog(*r);
}
void AppointmentView::onDelete() {
    qint64 id = selectedId();
    if (id <= 0) return;
    if (QMessageBox::question(this,"Confirm","Delete this appointment?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        auto r = HospitalController::instance().appointments().remove(id);
        if (r) refresh();
        else QMessageBox::critical(this,"Error",r.error());
    }
}

void AppointmentView::onUpdateStatus() {
    qint64 id = selectedId();
    if (id <= 0) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Update Appointment Status");
    dlg.setFixedWidth(300);
    auto* layout = new QVBoxLayout(&dlg);
    auto* combo = new QComboBox(&dlg);
    combo->addItems({"scheduled","confirmed","in-progress","completed","cancelled","no-show"});
    layout->addWidget(new QLabel("Select new status:", &dlg));
    layout->addWidget(combo);
    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* okBtn = new QPushButton("Update",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(okBtn);
    layout->addLayout(btns);
    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(okBtn,&QPushButton::clicked,&dlg,[&](){
        auto r = HospitalController::instance().appointments().updateStatus(id, combo->currentText());
        if (r) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",r.error());
    });
    dlg.exec();
}

void AppointmentView::onSearch(const QString& text) { loadAppointments(text); }

qint64 AppointmentView::selectedId() const {
    int row = m_table->currentRow();
    if (row < 0) return -1;
    auto* item = m_table->item(row, 0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void AppointmentView::showDialog(std::optional<Appointment> appt) {
    bool isEdit = appt.has_value();
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "Edit Appointment" : "New Appointment");
    dlg.setMinimumWidth(520);

    auto* layout = new QVBoxLayout(&dlg);

    auto* mainGroup = new QGroupBox("Appointment Details", &dlg);
    auto* form = new QFormLayout(mainGroup);

    // Patient combo
    auto* patientCombo = new QComboBox(&dlg);
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients) {
        for (const auto& p : *patients)
            patientCombo->addItem(p.patientUid + " — " + p.fullName(), p.id);
    }

    // Doctor combo
    auto* doctorCombo = new QComboBox(&dlg);
    auto doctors = HospitalController::instance().doctors().findAll(200);
    if (doctors) {
        for (const auto& d : *doctors)
            doctorCombo->addItem("Dr. " + d.fullName() + " (" + d.specialization + ")", d.id);
    }

    // Department combo
    auto* deptCombo = new QComboBox(&dlg);
    deptCombo->addItem("-- None --", QVariant((qint64)0));
    auto depts = HospitalController::instance().doctors().allDepartments();
    if (depts) {
        for (const auto& d : *depts)
            deptCombo->addItem(d.name, d.id);
    }

    auto* dateEdit = new QDateEdit(&dlg);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setDisplayFormat("yyyy-MM-dd");

    auto* timeEdit = new QTimeEdit(&dlg);
    timeEdit->setTime(QTime(9,0));
    timeEdit->setDisplayFormat("HH:mm");

    auto* durationSpin = new QSpinBox(&dlg);
    durationSpin->setRange(5, 480);
    durationSpin->setValue(30);
    durationSpin->setSuffix(" min");

    auto* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"consultation","follow-up","emergency","surgery","lab"});

    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({"scheduled","confirmed","in-progress","completed","cancelled","no-show"});

    auto* notesEdit = new QTextEdit(&dlg);
    notesEdit->setMaximumHeight(80);
    notesEdit->setPlaceholderText("Optional notes...");

    form->addRow("Patient *",    patientCombo);
    form->addRow("Doctor *",     doctorCombo);
    form->addRow("Department",   deptCombo);
    form->addRow("Date *",       dateEdit);
    form->addRow("Time *",       timeEdit);
    form->addRow("Duration",     durationSpin);
    form->addRow("Type",         typeCombo);
    form->addRow("Status",       statusCombo);
    form->addRow("Notes",        notesEdit);
    layout->addWidget(mainGroup);

    // Populate if editing
    if (isEdit) {
        auto& a = *appt;
        int pi = patientCombo->findData(a.patientId);
        if (pi >= 0) patientCombo->setCurrentIndex(pi);
        int di = doctorCombo->findData(a.doctorId);
        if (di >= 0) doctorCombo->setCurrentIndex(di);
        int depi = deptCombo->findData(a.departmentId);
        if (depi >= 0) deptCombo->setCurrentIndex(depi);
        dateEdit->setDate(a.appointmentDate);
        timeEdit->setTime(a.appointmentTime);
        durationSpin->setValue(a.durationMin);
        typeCombo->setCurrentText(a.type);
        statusCombo->setCurrentText(a.status);
        notesEdit->setPlainText(a.notes);
    }

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit ? "💾 Update" : "💾 Book",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (patientCombo->count()==0 || doctorCombo->count()==0) {
            QMessageBox::warning(&dlg,"Validation","Please add patients and doctors first.");
            return;
        }
        Appointment a = appt.value_or(Appointment{});
        a.patientId       = patientCombo->currentData().toLongLong();
        a.doctorId        = doctorCombo->currentData().toLongLong();
        a.departmentId    = deptCombo->currentData().toLongLong();
        a.appointmentDate = dateEdit->date();
        a.appointmentTime = timeEdit->time();
        a.durationMin     = durationSpin->value();
        a.type            = typeCombo->currentText();
        a.status          = statusCombo->currentText();
        a.notes           = notesEdit->toPlainText();

        auto result = isEdit
            ? HospitalController::instance().appointments().update(a)
            : HospitalController::instance().appointments().create(a);

        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",result.error());
    });

    dlg.exec();
}

} // namespace HMS
