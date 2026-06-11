#pragma once
#include <QWidget>
#include <QLabel>
#include "models/Patient.h"

namespace HMS {

// A compact summary card displayed in search results and patient lists
class PatientCard : public QWidget {
    Q_OBJECT
public:
    explicit PatientCard(const Patient& patient, QWidget* parent = nullptr);
    void setPatient(const Patient& patient);

signals:
    void clicked(qint64 patientId);

protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void enterEvent(QEnterEvent* ev) override;
    void leaveEvent(QEvent* ev) override;

private:
    void setupUi();
    void updateDisplay();

    QLabel* m_avatarLabel{nullptr};
    QLabel* m_nameLabel{nullptr};
    QLabel* m_idLabel{nullptr};
    QLabel* m_ageGenderLabel{nullptr};
    QLabel* m_bloodTypeLabel{nullptr};
    QLabel* m_statusLabel{nullptr};

    Patient m_patient;
};

} // namespace HMS
