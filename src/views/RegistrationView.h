#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models/Registration.h"
#include <optional>
#include <vector>

namespace HMS {

class RegistrationView : public QWidget {
    Q_OBJECT
public:
    explicit RegistrationView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onDischarge();
    void onDelete();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadData();
    void populateTable(const std::vector<Registration>& regs);
    void showDialog(std::optional<Registration> reg = std::nullopt);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_dischargeBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};

    std::vector<Registration> m_registrations;
};

} // namespace HMS
