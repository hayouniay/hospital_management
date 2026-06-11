#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models/Patient.h"
#include "auth/AuthController.h"
#include <optional>
#include <vector>

namespace HMS {

class PatientView : public QWidget {
    Q_OBJECT
public:
    explicit PatientView(QWidget* parent = nullptr);
    void refresh();

public:
    void applyRolesToUi();

private slots:
    void onAddPatient();
    void onEditPatient();
    void onDeletePatient();
    void onSearchChanged(const QString& text);
    void onViewQR();
    void onExportReport();        // NEW: export consultation PDF

private:
    void setupUi();
    void loadPatients(const QString& search = "");
    void populateTable(const std::vector<Patient>& patients);
    void showPatientDialog(std::optional<Patient> patient = std::nullopt);
    qint64 selectedPatientId() const;

    QTableWidget* m_table{nullptr};
    QLineEdit*    m_searchEdit{nullptr};
    QPushButton*  m_addBtn{nullptr};
    QPushButton*  m_editBtn{nullptr};
    QPushButton*  m_deleteBtn{nullptr};
    QPushButton*  m_qrBtn{nullptr};
    QPushButton*  m_reportBtn{nullptr};  // NEW

    std::vector<Patient> m_patients;
};

} // namespace HMS
