#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models/Doctor.h"
#include <optional>
#include <vector>

namespace HMS {

class DoctorView : public QWidget {
    Q_OBJECT
public:
    explicit DoctorView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadDoctors(const QString& search = "");
    void populateTable(const std::vector<Doctor>& doctors);
    void showDialog(std::optional<Doctor> doctor = std::nullopt);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};

    std::vector<Doctor> m_doctors;
};

} // namespace HMS
