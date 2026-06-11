#pragma once
#include "PatientController.h"
#include "AppointmentController.h"
#include "BillingController.h"
#include "DoctorController.h"
#include "DepartmentController.h"
#include "MedicalRecordController.h"
#include "RegistrationController.h"
#include <QObject>
#include <memory>

namespace HMS {

class HospitalController : public QObject {
    Q_OBJECT
public:
    static HospitalController& instance();

    PatientController&       patients()       { return *m_patients; }
    AppointmentController&   appointments()   { return *m_appointments; }
    BillingController&       billing()        { return *m_billing; }
    DoctorController&        doctors()        { return *m_doctors; }
    DepartmentController&    departments()    { return *m_departments; }
    MedicalRecordController& medicalRecords() { return *m_medicalRecords; }
    RegistrationController&  registrations()  { return *m_registrations; }

    struct DashboardStats {
        int    totalPatients{0};
        int    activePatients{0};
        int    admittedPatients{0};
        int    todayAppointments{0};
        int    pendingBills{0};
        int    totalDoctors{0};
        int    totalDepartments{0};
        double todayRevenue{0.0};
    };
    std::expected<DashboardStats, QString> getDashboardStats();

private:
    HospitalController();
    std::unique_ptr<PatientController>       m_patients;
    std::unique_ptr<AppointmentController>   m_appointments;
    std::unique_ptr<BillingController>       m_billing;
    std::unique_ptr<DoctorController>        m_doctors;
    std::unique_ptr<DepartmentController>    m_departments;
    std::unique_ptr<MedicalRecordController> m_medicalRecords;
    std::unique_ptr<RegistrationController>  m_registrations;
};

} // namespace HMS
