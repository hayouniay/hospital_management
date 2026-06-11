#include "utils/PDFExporter.h"
#include "BillingView.h"
#include "core/HospitalController.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QGroupBox>
#include <QFrame>

namespace HMS {

BillingView::BillingView(QWidget* parent) : QWidget(parent) { setupUi(); }

void BillingView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search by invoice, patient...");
    m_searchEdit->setFixedHeight(36);
    m_addBtn    = new QPushButton("+ New Invoice", this);
    m_payBtn    = new QPushButton("💳 Record Payment", this);
    m_editBtn   = new QPushButton("✏ Edit", this);
    m_deleteBtn = new QPushButton("🗑 Delete", this);
    m_payBtn->setObjectName("secondaryBtn");
    m_editBtn->setObjectName("secondaryBtn");
    m_deleteBtn->setObjectName("dangerBtn");
    m_exportBtn = new QPushButton("📄 Export PDF", this);
    m_exportBtn->setObjectName("secondaryBtn");
    m_exportBtn->setEnabled(false);
    m_payBtn->setEnabled(false);
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    tb->addWidget(m_searchEdit); tb->addStretch();
    tb->addWidget(m_exportBtn); tb->addWidget(m_payBtn); tb->addWidget(m_editBtn);
    tb->addWidget(m_deleteBtn); tb->addWidget(m_addBtn);
    layout->addLayout(tb);

    // Summary bar
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet(R"(
        background:#EBF8FF;border:1px solid #BEE3F8;border-radius:8px;
        padding:8px 16px;color:#2B6CB0;font-weight:600;font-size:13px;)");
    m_summaryLabel->setText("Total: $0  |  Paid: $0  |  Outstanding: $0");
    layout->addWidget(m_summaryLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(9);
    m_table->setHorizontalHeaderLabels({
        "Invoice","Patient","Amount","Discount","Tax","Total","Paid","Balance","Status"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &BillingView::onSearch);
    connect(m_addBtn,    &QPushButton::clicked, this, &BillingView::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &BillingView::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &BillingView::onDelete);
    connect(m_payBtn,    &QPushButton::clicked, this, &BillingView::onRecordPayment);
    connect(m_exportBtn, &QPushButton::clicked, this, &BillingView::onExportPDF);
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
            m_payBtn->setEnabled(sel);
            m_exportBtn->setEnabled(sel);
        });
}

void BillingView::refresh() { loadData(m_searchEdit->text()); }

void BillingView::loadData(const QString& /*search*/) {
    auto r = HospitalController::instance().billing().findAll(300);
    if (r) { m_bills = *r; populateTable(m_bills); }
}

void BillingView::populateTable(const std::vector<Billing>& bills) {
    m_table->setRowCount(0);
    double totalAmt=0, totalPaid=0;

    for (const auto& b : bills) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row,0,new QTableWidgetItem(b.invoiceNumber));
        m_table->setItem(row,1,new QTableWidgetItem(b.patientName));
        m_table->setItem(row,2,new QTableWidgetItem(QString("$%1").arg(b.amount,0,'f',2)));
        m_table->setItem(row,3,new QTableWidgetItem(QString("$%1").arg(b.discount,0,'f',2)));
        m_table->setItem(row,4,new QTableWidgetItem(QString("$%1").arg(b.tax,0,'f',2)));
        m_table->setItem(row,5,new QTableWidgetItem(QString("$%1").arg(b.total,0,'f',2)));
        m_table->setItem(row,6,new QTableWidgetItem(QString("$%1").arg(b.paidAmount,0,'f',2)));

        double bal = b.balance();
        auto* balItem = new QTableWidgetItem(QString("$%1").arg(bal,0,'f',2));
        balItem->setForeground(bal > 0 ? QColor("#E53E3E") : QColor("#38A169"));
        m_table->setItem(row,7,balItem);

        auto* si = new QTableWidgetItem(b.status);
        if      (b.status=="paid")     si->setForeground(QColor("#38A169"));
        else if (b.status=="partial")  si->setForeground(QColor("#D69E2E"));
        else if (b.status=="pending")  si->setForeground(QColor("#3182CE"));
        else if (b.status=="cancelled")si->setForeground(QColor("#E53E3E"));
        m_table->setItem(row,8,si);
        m_table->item(row,0)->setData(Qt::UserRole, b.id);

        totalAmt  += b.total;
        totalPaid += b.paidAmount;
    }
    m_table->resizeColumnsToContents();
    double outstanding = totalAmt - totalPaid;
    m_summaryLabel->setText(QString("Total: $%1  |  Paid: $%2  |  Outstanding: $%3")
        .arg(totalAmt,0,'f',2).arg(totalPaid,0,'f',2).arg(outstanding,0,'f',2));
}

void BillingView::onAdd()  { showDialog(); }
void BillingView::onEdit() {
    qint64 id = selectedId();
    if (id<=0) return;
    auto r = HospitalController::instance().billing().findById(id);
    if (r) showDialog(*r);
}
void BillingView::onDelete() {
    qint64 id = selectedId();
    if (id<=0) return;
    if (QMessageBox::question(this,"Confirm","Delete this invoice?",
            QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        auto r = HospitalController::instance().billing().remove(id);
        if (r) refresh(); else QMessageBox::critical(this,"Error",r.error());
    }
}
void BillingView::onSearch(const QString& text) { loadData(text); }

void BillingView::onRecordPayment() {
    qint64 id = selectedId();
    if (id<=0) return;
    showPaymentDialog(id);
}

qint64 BillingView::selectedId() const {
    int row = m_table->currentRow();
    if (row<0) return -1;
    auto* item = m_table->item(row,0);
    return item ? item->data(Qt::UserRole).toLongLong() : -1;
}

void BillingView::showPaymentDialog(qint64 id) {
    auto billResult = HospitalController::instance().billing().findById(id);
    if (!billResult) return;
    auto& bill = *billResult;

    QDialog dlg(this);
    dlg.setWindowTitle("Record Payment — " + bill.invoiceNumber);
    dlg.setFixedWidth(380);
    auto* layout = new QVBoxLayout(&dlg);

    auto* info = new QLabel(QString("Patient: %1\nTotal: $%2   Paid: $%3   Balance: $%4")
        .arg(bill.patientName)
        .arg(bill.total,0,'f',2)
        .arg(bill.paidAmount,0,'f',2)
        .arg(bill.balance(),0,'f',2), &dlg);
    info->setStyleSheet("background:#F7FAFC;padding:10px;border-radius:6px;border:1px solid #E2E8F0;");
    layout->addWidget(info);

    auto* form = new QFormLayout;
    auto* amountSpin = new QDoubleSpinBox(&dlg);
    amountSpin->setRange(0.01, 999999.99);
    amountSpin->setPrefix("$ ");
    amountSpin->setDecimals(2);
    amountSpin->setValue(bill.balance());

    auto* methodCombo = new QComboBox(&dlg);
    methodCombo->addItems({"cash","card","insurance","online"});

    form->addRow("Amount *",         amountSpin);
    form->addRow("Payment Method *", methodCombo);
    layout->addLayout(form);

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton("✅ Confirm Payment",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        auto r = HospitalController::instance().billing().recordPayment(
            id, amountSpin->value(), methodCombo->currentText());
        if (r) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",r.error());
    });
    dlg.exec();
}

void BillingView::showDialog(std::optional<Billing> bill) {
    bool isEdit = bill.has_value();
    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? "Edit Invoice" : "New Invoice");
    dlg.setMinimumWidth(500);
    auto* layout = new QVBoxLayout(&dlg);

    auto* group = new QGroupBox("Invoice Details",&dlg);
    auto* form = new QFormLayout(group);

    auto* patientCombo = new QComboBox(&dlg);
    auto patients = HospitalController::instance().patients().findAll(500);
    if (patients)
        for (const auto& p : *patients)
            patientCombo->addItem(p.patientUid + " — " + p.fullName(), p.id);

    auto* amountSpin  = new QDoubleSpinBox(&dlg);
    amountSpin->setRange(0,999999); amountSpin->setPrefix("$ "); amountSpin->setDecimals(2);
    auto* discountSpin = new QDoubleSpinBox(&dlg);
    discountSpin->setRange(0,999999); discountSpin->setPrefix("$ "); discountSpin->setDecimals(2);
    auto* taxSpin = new QDoubleSpinBox(&dlg);
    taxSpin->setRange(0,99999); taxSpin->setPrefix("$ "); taxSpin->setDecimals(2);

    auto* methodCombo = new QComboBox(&dlg);
    methodCombo->addItems({"pending","cash","card","insurance","online"});
    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({"pending","partial","paid","cancelled","refunded"});
    auto* dueDateEdit = new QDateEdit(&dlg);
    dueDateEdit->setCalendarPopup(true);
    dueDateEdit->setDate(QDate::currentDate().addDays(30));
    dueDateEdit->setDisplayFormat("yyyy-MM-dd");
    auto* notesEdit = new QTextEdit(&dlg);
    notesEdit->setMaximumHeight(60);

    form->addRow("Patient *",        patientCombo);
    form->addRow("Amount *",         amountSpin);
    form->addRow("Discount",         discountSpin);
    form->addRow("Tax",              taxSpin);
    form->addRow("Payment Method",   methodCombo);
    form->addRow("Status",           statusCombo);
    form->addRow("Due Date",         dueDateEdit);
    form->addRow("Notes",            notesEdit);
    layout->addWidget(group);

    if (isEdit) {
        auto& b = *bill;
        int pi = patientCombo->findData(b.patientId);
        if (pi>=0) patientCombo->setCurrentIndex(pi);
        amountSpin->setValue(b.amount);
        discountSpin->setValue(b.discount);
        taxSpin->setValue(b.tax);
        methodCombo->setCurrentText(b.paymentMethod);
        statusCombo->setCurrentText(b.status);
        dueDateEdit->setDate(b.dueDate.isValid() ? b.dueDate : QDate::currentDate().addDays(30));
        notesEdit->setPlainText(b.notes);
    }

    auto* btns = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancel",&dlg);
    cancelBtn->setObjectName("secondaryBtn");
    auto* saveBtn = new QPushButton(isEdit?"💾 Update":"💾 Create Invoice",&dlg);
    btns->addStretch(); btns->addWidget(cancelBtn); btns->addWidget(saveBtn);
    layout->addLayout(btns);

    connect(cancelBtn,&QPushButton::clicked,&dlg,&QDialog::reject);
    connect(saveBtn,&QPushButton::clicked,&dlg,[&](){
        if (patientCombo->count()==0) {
            QMessageBox::warning(&dlg,"Validation","No patients available.");
            return;
        }
        Billing b = bill.value_or(Billing{});
        b.patientId     = patientCombo->currentData().toLongLong();
        b.amount        = amountSpin->value();
        b.discount      = discountSpin->value();
        b.tax           = taxSpin->value();
        b.total         = b.amount - b.discount + b.tax;
        b.paymentMethod = methodCombo->currentText();
        b.status        = statusCombo->currentText();
        b.dueDate       = dueDateEdit->date();
        b.notes         = notesEdit->toPlainText();

        auto result = isEdit
            ? HospitalController::instance().billing().update(b)
            : HospitalController::instance().billing().create(b);
        if (result) { dlg.accept(); refresh(); }
        else QMessageBox::critical(&dlg,"Error",result.error());
    });
    dlg.exec();
}


void BillingView::onExportPDF() {
    qint64 id = selectedId();
    if (id <= 0) return;

    auto billResult = HospitalController::instance().billing().findById(id);
    if (!billResult) { return; }
    auto& bill = *billResult;

    // Get patient
    auto patResult = HospitalController::instance().patients().findById(bill.patientId);
    if (!patResult) {
        QMessageBox::critical(this, "Error", "Patient not found for this invoice.");
        return;
    }

    auto result = PDFExporter::exportBillPDFDialog(bill, *patResult, this);
    if (!result && result.error() != "Cancelled") {
        QMessageBox::critical(this, "Export Failed", result.error());
    } else if (result) {
        QMessageBox::information(this, "Export Complete",
            "Invoice PDF saved and opened:\n" + *result);
    }
}
} // namespace HMS
