#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "models/Appointment.h"
#include <optional>
#include <vector>

namespace HMS {

class AppointmentView : public QWidget {
    Q_OBJECT
public:
    explicit AppointmentView(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onUpdateStatus();
    void onSearch(const QString& text);

private:
    void setupUi();
    void loadAppointments(const QString& search = "");
    void populateTable(const std::vector<Appointment>& appts);
    void showDialog(std::optional<Appointment> appt = std::nullopt);
    qint64 selectedId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};
    QPushButton*  m_statusBtn{nullptr};

    std::vector<Appointment> m_appointments;
};

} // namespace HMS
