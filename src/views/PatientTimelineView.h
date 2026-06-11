#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "models/Patient.h"

namespace HMS {

class PatientTimelineView : public QWidget {
    Q_OBJECT
public:
    explicit PatientTimelineView(QWidget* parent = nullptr);
    void setPatient(qint64 patientId);
    void refresh();

private:
    void setupUi();
    void buildTimeline();
    void clearTimeline();

    struct TimelineEvent {
        QDateTime   timestamp;
        QString     type;       // registration, appointment, record, billing
        QString     icon;
        QString     title;
        QString     detail;
        QString     status;
        QColor      color;
    };

    void addEventCard(const TimelineEvent& ev);

    QComboBox*   m_patientCombo{nullptr};
    QScrollArea* m_scroll{nullptr};
    QWidget*     m_timelineWidget{nullptr};
    QVBoxLayout* m_timelineLayout{nullptr};
    QLabel*      m_patientInfoLabel{nullptr};
    QPushButton* m_exportBtn{nullptr};

    qint64 m_patientId{0};
};

} // namespace HMS
