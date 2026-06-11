#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "models/Billing.h"
#include <optional>
#include <vector>

namespace HMS {

class BillingView : public QWidget {
    Q_OBJECT
public:
    explicit BillingView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onRecordPayment();
    void onExportPDF();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadData(const QString& search = "");
    void populateTable(const std::vector<Billing>& bills);
    void showDialog(std::optional<Billing> bill = std::nullopt);
    void showPaymentDialog(qint64 id);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};
    QPushButton*  m_payBtn{nullptr};
    QPushButton*  m_exportBtn{nullptr};
    QLabel*       m_summaryLabel{nullptr};

    std::vector<Billing> m_bills;
};

} // namespace HMS
