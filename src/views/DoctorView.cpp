#include "DoctorView.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>

namespace HMS {

DoctorView::DoctorView(QWidget* parent) : QWidget(parent) { setupUi(); }

void DoctorView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    // Toolbar
    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search by name, specialization, ID...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn    = new QPushButton("+ Add Doctor", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_editBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    tb->addWidget(m_searchEdit);
    tb->addStretch();
    tb->addWidget(m_editBtn);
    tb->addWidget(m_deleteBtn);
    tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "Employee ID","Name","Specialization","Department","Phone","Email","Status","Since"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &DoctorView::onSearch);
    connect(m_addBtn,    &QPushButton::clicked, this, &DoctorView::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &DoctorView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DoctorView::onDelete);
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
        });
}

void DoctorView::refresh() { loadDoctors(m_searchEdit->text()); }

void DoctorView::loadDoctors(const QString& search) {
    auto& ctrl = HospitalController::instance().doctors();
    auto result = search.isEmpty() ? ctrl.findAll() : ctrl.search(search);
    if (result) { m_doctors = *result; populateTable(m_doctors); }
}

void DoctorView::populateTable(const std::vector<Doctor>& doctors) {
    m_table->setRowCount(0);

    // Pre-fetch departments for name lookup
    auto& ctrl = HospitalController::instance().doctors();
    auto depts = ctrl.allDepartments();

    for (const auto& d : doctors) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Department name lookup
        QString deptName = "-";
        if (depts && d.departmentId > 0) {
            for (const auto& dep : *depts) {
                if (dep.id == d.departmentId) { deptName = dep.name; break; }
            }
        }

        m_table->setItem(row, 0, new QTableWidgetItem(d.employeeId));
        m_table->setItem(row, 1, new QTableWidgetItem(d.fullName()));
        m_table->setItem(row, 2, new QTableWidgetItem(d.specialization));
        m_table->setItem(row, 3, new QTableWidgetItem(deptName));
        m_table->setItem(row, 4, new QTableWidgetItem(d.phone));
        m_table->setItem(row, 5, new QTableWidgetItem(d.email));

        auto* statusItem = new QTableWidgetItem(d.status);
        if      (d.status == "active")   statusItem->setForeground(QColor("#38A169"));
        else if (d.status == "on_leave") statusItem->setForeground(QColor("#D69E2E"));
        else                             statusItem->setForeground(QColor("#E53E3E"));
        m_table->setItem(row, 6, statusItem);
        m_table->setItem(row, 7, new QTableWidgetItem(d.createdAt.toString("yyyy-MM-dd")));
        m_table->item(row, 0)->setData(Qt::UserRole, d.id);
    }
    m_table->resizeColumnsToContents();
}

void DoctorView::onAdd()  { showDialog(); }
void DoctorView::onEdit() {
    qint64 id = selectedId();
    if (id <= 0) return;
    auto r = HospitalController::instance().doctors().findById(id);
    if (r) showDialog(*r);
}

void DoctorView::onDelete() {
    qint64 id = selectedId();
    if (id <= 0) return;
    if (QMessageBox::question(this, "Confirm", "Delete this doctor?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        auto r = HospitalController::instance().doctors().remove(id);
        if (r) refresh();
        else QMessageBox::critical(this, "Error", r.error());
    }
}

void DoctorView::onSearch(const QString& text) { loadDoctors(text); }

qint64 DoctorView::selectedId() const {
    int row = m_table->currentRow();
    if (row < 0) return -1;
    auto* item = m_table->item(row, 0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void DoctorView::showDialog(std::optional<Doctor> doctor) {
    bool isEdit = doctor.has_value();
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "Edit Doctor" : "Add New Doctor");
    dlg.setMinimumWidth(520);

    auto* layout = new QVBoxLayout(&dlg);

    // Personal
    auto* personalGroup = new QGroupBox("Personal Information", &dlg);
    auto* pf = new QFormLayout(personalGroup);
    auto* firstNameEdit = new QLineEdit(&dlg);
    auto* lastNameEdit  = new QLineEdit(&dlg);
    auto* specEdit      = new QLineEdit(&dlg);
    auto* licenseEdit   = new QLineEdit(&dlg);
    pf->addRow("First Name *",    firstNameEdit);
    pf->addRow("Last Name",       lastNameEdit);
    pf->addRow("Specialization *",specEdit);
    pf->addRow("License Number",  licenseEdit);

    // Contact
    auto* contactGroup = new QGroupBox("Contact", &dlg);
    auto* cf = new QFormLayout(contactGroup);
    auto* phoneEdit = new QLineEdit(&dlg);
    auto* emailEdit = new QLineEdit(&dlg);
    cf->addRow("Phone", phoneEdit);
    cf->addRow("Email", emailEdit);

    // Assignment
    auto* assignGroup = new QGroupBox("Assignment & Status", &dlg);
    auto* af = new QFormLayout(assignGroup);
    auto* deptCombo   = new QComboBox(&dlg);
    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({"active","inactive","on_leave"});

    // Populate departments
    deptCombo->addItem("-- None --", QVariant((qint64)0));
    auto& ctrl = HospitalController::instance().doctors();
    auto depts = ctrl.allDepartments();
    if (depts) {
        for (const auto& d : *depts)
            deptCombo->addItem(d.name, d.id);
    }

    af->addRow("Department", deptCombo);
    af->addRow("Status",     statusCombo);

    layout->addWidget(personalGroup);
    layout->addWidget(contactGroup);
    layout->addWidget(assignGroup);

    // Populate if editing
    if (isEdit) {
        auto& d = *doctor;
        firstNameEdit->setText(d.firstName);
        lastNameEdit->setText(d.lastName);
        specEdit->setText(d.specialization);
        licenseEdit->setText(d.licenseNumber);
        phoneEdit->setText(d.phone);
        emailEdit->setText(d.email);
        statusCombo->setCurrentText(d.status);
        int idx = deptCombo->findData(d.departmentId);
        if (idx >= 0) deptCombo->setCurrentIndex(idx);
    }

    // Buttons
    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit ? "💾 Update" : "💾 Save", &dlg);
    btns->addStretch();
    btns->addWidget(cancelBtn);
    btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, &dlg, [&]() {
        if (firstNameEdit->text().trimmed().isEmpty() || specEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", "First name and specialization are required.");
            return;
        }
        Doctor d = doctor.value_or(Doctor{});
        d.firstName     = firstNameEdit->text().trimmed();
        d.lastName      = lastNameEdit->text().trimmed();
        d.specialization= specEdit->text().trimmed();
        d.licenseNumber = licenseEdit->text().trimmed();
        d.phone         = phoneEdit->text().trimmed();
        d.email         = emailEdit->text().trimmed();
        d.departmentId  = deptCombo->currentData().toLongLong();
        d.status        = statusCombo->currentText();

        auto result = isEdit
            ? HospitalController::instance().doctors().update(d)
            : HospitalController::instance().doctors().create(d);

        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg, "Error", result.error());
    });

    dlg.exec();
}

} // namespace HMS
