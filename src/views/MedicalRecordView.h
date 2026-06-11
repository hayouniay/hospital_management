#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models/MedicalRecord.h"
#include <optional>
#include <vector>

namespace HMS {

class MedicalRecordView : public QWidget {
    Q_OBJECT
public:
    explicit MedicalRecordView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onExportReport();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadData(const QString& search = "");
    void populateTable(const std::vector<MedicalRecord>& records);
    void showDialog(std::optional<MedicalRecord> rec = std::nullopt);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};
    QPushButton*  m_viewBtn{nullptr};
    QPushButton*  m_exportBtn{nullptr};

    std::vector<MedicalRecord> m_records;
};

} // namespace HMS
