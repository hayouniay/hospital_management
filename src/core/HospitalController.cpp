#include "HospitalController.h"

namespace HMS {

HospitalController& HospitalController::instance() {
    static HospitalController inst;
    return inst;
}

HospitalController::HospitalController()
    : m_patients(std::make_unique<PatientController>())
    , m_appointments(std::make_unique<AppointmentController>())
    , m_billing(std::make_unique<BillingController>())
    , m_doctors(std::make_unique<DoctorController>())
    , m_departments(std::make_unique<DepartmentController>())
    , m_medicalRecords(std::make_unique<MedicalRecordController>())
    , m_registrations(std::make_unique<RegistrationController>())
{}

std::expected<HospitalController::DashboardStats, QString>
HospitalController::getDashboardStats() {
    DashboardStats stats;
    if (auto r = m_patients->countAll())      stats.totalPatients    = *r;
    if (auto r = m_patients->countActive())   stats.activePatients   = *r;
    if (auto r = m_patients->countAdmitted()) stats.admittedPatients = *r;
    if (auto r = m_appointments->findToday()) stats.todayAppointments = static_cast<int>(r->size());
    if (auto r = m_billing->countByStatus("pending")) stats.pendingBills = *r;
    if (auto r = m_doctors->countActive())    stats.totalDoctors     = *r;
    if (auto r = m_departments->count())      stats.totalDepartments = *r;
    if (auto r = m_billing->totalRevenue(QDate::currentDate(), QDate::currentDate()))
        stats.todayRevenue = *r;
    return stats;
}

} // namespace HMS
