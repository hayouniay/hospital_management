#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>

namespace HMS {

class AuditLogView : public QWidget {
    Q_OBJECT
public:
    explicit AuditLogView(QWidget* parent = nullptr);
    void refresh();

private:
    void setupUi();
    void loadData();

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QComboBox*    m_tableFilter{nullptr};
    QComboBox*    m_actionFilter{nullptr};
    QDateEdit*    m_fromDate{nullptr};
    QDateEdit*    m_toDate{nullptr};
};

} // namespace HMS
