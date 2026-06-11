#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models/Department.h"
#include <optional>
#include <vector>

namespace HMS {

class DepartmentView : public QWidget {
    Q_OBJECT
public:
    explicit DepartmentView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadData(const QString& search = "");
    void populateTable(const std::vector<Department>& depts);
    void showDialog(std::optional<Department> dept = std::nullopt);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};

    std::vector<Department> m_departments;
};

} // namespace HMS
