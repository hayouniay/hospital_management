#include "DepartmentView.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>

namespace HMS {

DepartmentView::DepartmentView(QWidget* parent) : QWidget(parent) { setupUi(); }

void DepartmentView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search departments...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn    = new QPushButton("+ New Department", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_editBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    tb->addWidget(m_searchEdit); tb->addStretch();
    tb->addWidget(m_editBtn); tb->addWidget(m_deleteBtn); tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID","Name","Floor","Capacity","Phone","Status"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &DepartmentView::onSearch);
    connect(m_addBtn,    &QPushButton::clicked, this, &DepartmentView::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &DepartmentView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DepartmentView::onDelete);
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

void DepartmentView::refresh() { loadData(m_searchEdit->text()); }

void DepartmentView::loadData(const QString& search) {
    auto r = search.isEmpty() ? HospitalController::instance().departments().findAll() : HospitalController::instance().departments().search(search);
    if (r) { m_departments = *r; populateTable(m_departments); }
}

void DepartmentView::populateTable(const std::vector<Department>& depts) {
    m_table->setRowCount(0);
    for (const auto& d : depts) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row,0,new QTableWidgetItem(QString::number(d.id)));
        m_table->setItem(row,1,new QTableWidgetItem(d.name));
        m_table->setItem(row,2,new QTableWidgetItem(QString("Floor %1").arg(d.floor)));
        m_table->setItem(row,3,new QTableWidgetItem(QString::number(d.capacity)+" beds"));
        m_table->setItem(row,4,new QTableWidgetItem(d.phone));
        auto* si = new QTableWidgetItem(d.status);
        si->setForeground(d.status=="active" ? QColor("#38A169") : QColor("#E53E3E"));
        m_table->setItem(row,5,si);
        m_table->item(row,0)->setData(Qt::UserRole, d.id);
    }
    m_table->resizeColumnsToContents();
}

void DepartmentView::onAdd()  { showDialog(); }
void DepartmentView::onEdit() {
    qint64 id = selectedId();
    if (id<=0) return;
    auto r = HospitalController::instance().departments().findById(id);
    if (r) showDialog(*r);
}
void DepartmentView::onDelete() {
    qint64 id = selectedId();
    if (id<=0) return;
    if (QMessageBox::question(this,"Confirm","Delete this department?\nAll associated doctors will lose their department assignment.",
            QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        auto r = HospitalController::instance().departments().remove(id);
        if (r) refresh(); else QMessageBox::critical(this,"Error",r.error());
    }
}
void DepartmentView::onSearch(const QString& text) { loadData(text); }

qint64 DepartmentView::selectedId() const {
    int row = m_table->currentRow();
    if (row<0) return -1;
    auto* item = m_table->item(row,0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void DepartmentView::showDialog(std::optional<Department> dept) {
    bool isEdit = dept.has_value();
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "Edit Department" : "New Department");
    dlg.setFixedWidth(420);
    auto* layout = new QVBoxLayout(&dlg);

    auto* group = new QGroupBox("Department Details",&dlg);
    auto* form  = new QFormLayout(group);

    auto* nameEdit   = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("e.g. Cardiology");
    auto* floorSpin  = new QSpinBox(&dlg);
    floorSpin->setRange(0,50); floorSpin->setSuffix(" (floor)");
    auto* capSpin    = new QSpinBox(&dlg);
    capSpin->setRange(1,500); capSpin->setValue(20); capSpin->setSuffix(" beds");
    auto* phoneEdit  = new QLineEdit(&dlg);

    // Head doctor combo
    auto* headCombo  = new QComboBox(&dlg);
    headCombo->addItem("-- None --", QVariant((qint64)0));
    auto doctors = HospitalController::instance().doctors().findAll(200);
    if (doctors) for (const auto& d : *doctors)
        headCombo->addItem("Dr. "+d.fullName()+" ("+d.specialization+")", d.id);

    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({"active","inactive"});

    form->addRow("Name *",       nameEdit);
    form->addRow("Head Doctor",  headCombo);
    form->addRow("Floor",        floorSpin);
    form->addRow("Capacity",     capSpin);
    form->addRow("Phone",        phoneEdit);
    form->addRow("Status",       statusCombo);
    layout->addWidget(group);

    if (isEdit) {
        auto& d = *dept;
        nameEdit->setText(d.name);
        floorSpin->setValue(d.floor);
        capSpin->setValue(d.capacity);
        phoneEdit->setText(d.phone);
        statusCombo->setCurrentText(d.status);
        int hi = headCombo->findData(d.headDoctor);
        if (hi>=0) headCombo->setCurrentIndex(hi);
    }

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit?"💾 Update":"💾 Create",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg,"Validation","Department name is required.");
            return;
        }
        Department d = dept.value_or(Department{});
        d.name      = nameEdit->text().trimmed();
        d.headDoctor= headCombo->currentData().toLongLong();
        d.floor     = floorSpin->value();
        d.capacity  = capSpin->value();
        d.phone     = phoneEdit->text().trimmed();
        d.status    = statusCombo->currentText();

        auto result = isEdit ? HospitalController::instance().departments().update(d) : HospitalController::instance().departments().create(d);
        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",result.error());
    });
    dlg.exec();
}

} // namespace HMS
