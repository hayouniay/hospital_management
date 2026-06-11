#include "AuditLogView.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

namespace HMS {

AuditLogView::AuditLogView(QWidget* parent) : QWidget(parent) { setupUi(); }

void AuditLogView::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24,16,24,24);
    layout->setSpacing(12);

    // Filter toolbar
    auto* tb = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍  Search audit log...");
    m_searchEdit->setFixedHeight(34);

    m_tableFilter = new QComboBox(this);
    m_tableFilter->addItem("All Tables");
    m_tableFilter->addItems({"patients","doctors","appointments",
                              "registrations","billings","medical_records","departments"});
    m_tableFilter->setFixedHeight(34);

    m_actionFilter = new QComboBox(this);
    m_actionFilter->addItem("All Actions");
    m_actionFilter->addItems({"INSERT","UPDATE","DELETE"});
    m_actionFilter->setFixedHeight(34);

    m_fromDate = new QDateEdit(this);
    m_fromDate->setDate(QDate::currentDate().addMonths(-1));
    m_fromDate->setDisplayFormat("yyyy-MM-dd");
    m_fromDate->setCalendarPopup(true);
    m_fromDate->setFixedHeight(34);

    m_toDate = new QDateEdit(this);
    m_toDate->setDate(QDate::currentDate());
    m_toDate->setDisplayFormat("yyyy-MM-dd");
    m_toDate->setCalendarPopup(true);
    m_toDate->setFixedHeight(34);

    auto* refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setObjectName("secondaryBtn");

    tb->addWidget(m_searchEdit);
    tb->addWidget(new QLabel("Table:", this));
    tb->addWidget(m_tableFilter);
    tb->addWidget(new QLabel("Action:", this));
    tb->addWidget(m_actionFilter);
    tb->addWidget(new QLabel("From:", this));
    tb->addWidget(m_fromDate);
    tb->addWidget(new QLabel("To:", this));
    tb->addWidget(m_toDate);
    tb->addWidget(refreshBtn);
    layout->addLayout(tb);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Time","Table","Record ID","Action","User","Old Value","New Value"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);

    connect(refreshBtn, &QPushButton::clicked, this, &AuditLogView::loadData);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AuditLogView::loadData);
    connect(m_tableFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuditLogView::loadData);
    connect(m_actionFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuditLogView::loadData);
}

void AuditLogView::refresh() { loadData(); }

void AuditLogView::loadData() {
    auto& db = DatabaseManager::instance();

    QString sql = R"(
        SELECT created_at, table_name, record_id, action, user, old_data, new_data
        FROM audit_log
        WHERE date(created_at) BETWEEN ? AND ?
    )";
    std::vector<QVariant> params = {
        m_fromDate->date().toString(Qt::ISODate),
        m_toDate->date().toString(Qt::ISODate)
    };

    if (m_tableFilter->currentIndex() > 0) {
        sql += " AND table_name = ?";
        params.push_back(m_tableFilter->currentText());
    }
    if (m_actionFilter->currentIndex() > 0) {
        sql += " AND action = ?";
        params.push_back(m_actionFilter->currentText());
    }

    const QString search = m_searchEdit->text().trimmed();
    if (!search.isEmpty()) {
        sql += " AND (table_name LIKE ? OR user LIKE ? OR old_data LIKE ? OR new_data LIKE ?)";
        const QString like = "%" + search + "%";
        params.insert(params.end(), {like, like, like, like});
    }

    sql += " ORDER BY created_at DESC LIMIT 500";

    auto r = db.execute(sql, params);
    if (!r) return;

    m_table->setRowCount(0);
    while ((*r).next()) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row,0,new QTableWidgetItem((*r).value(0).toString()));
        m_table->setItem(row,1,new QTableWidgetItem((*r).value(1).toString()));
        m_table->setItem(row,2,new QTableWidgetItem((*r).value(2).toString()));

        const QString action = (*r).value(3).toString();
        auto* actionItem = new QTableWidgetItem(action);
        if      (action == "INSERT") actionItem->setForeground(QColor("#38A169"));
        else if (action == "UPDATE") actionItem->setForeground(QColor("#D69E2E"));
        else if (action == "DELETE") actionItem->setForeground(QColor("#E53E3E"));
        m_table->setItem(row,3,actionItem);

        m_table->setItem(row,4,new QTableWidgetItem((*r).value(4).toString()));
        m_table->setItem(row,5,new QTableWidgetItem((*r).value(5).toString()));
        m_table->setItem(row,6,new QTableWidgetItem((*r).value(6).toString()));
    }
    m_table->resizeColumnToContents(0);
    m_table->resizeColumnToContents(1);
    m_table->resizeColumnToContents(3);
}

}
