#include "AppSettings.h"

namespace HMS {

AppSettings& AppSettings::instance() {
    static AppSettings inst;
    return inst;
}

AppSettings::AppSettings() : m_s("MediCare", "HospitalMS") {}

QString AppSettings::hospitalName()    const { return m_s.value("hospital/name", "MediCare Hospital").toString(); }
void    AppSettings::setHospitalName(const QString& v) { m_s.setValue("hospital/name", v); emit hospitalInfoChanged(); }
QString AppSettings::hospitalAddress() const { return m_s.value("hospital/address", "").toString(); }
void    AppSettings::setHospitalAddress(const QString& v) { m_s.setValue("hospital/address", v); emit hospitalInfoChanged(); }
QString AppSettings::hospitalPhone()   const { return m_s.value("hospital/phone", "").toString(); }
void    AppSettings::setHospitalPhone(const QString& v) { m_s.setValue("hospital/phone", v); emit hospitalInfoChanged(); }
QString AppSettings::hospitalEmail()   const { return m_s.value("hospital/email", "").toString(); }
void    AppSettings::setHospitalEmail(const QString& v) { m_s.setValue("hospital/email", v); emit hospitalInfoChanged(); }
QString AppSettings::hospitalLogoPath()const { return m_s.value("hospital/logo", "").toString(); }
void    AppSettings::setHospitalLogoPath(const QString& v) { m_s.setValue("hospital/logo", v); emit hospitalInfoChanged(); }

bool    AppSettings::darkMode()        const { return m_s.value("ui/darkMode", false).toBool(); }
void    AppSettings::setDarkMode(bool on) { m_s.setValue("ui/darkMode", on); emit darkModeChanged(on); }

int     AppSettings::sessionTimeoutMinutes() const { return m_s.value("security/sessionTimeout", 30).toInt(); }
void    AppSettings::setSessionTimeoutMinutes(int m) { m_s.setValue("security/sessionTimeout", m); }
QString AppSettings::lastUser()        const { return m_s.value("security/lastUser", "admin").toString(); }
void    AppSettings::setLastUser(const QString& u) { m_s.setValue("security/lastUser", u); }

bool    AppSettings::sidebarCollapsed() const { return m_s.value("ui/sidebarCollapsed", false).toBool(); }
void    AppSettings::setSidebarCollapsed(bool v) { m_s.setValue("ui/sidebarCollapsed", v); }

void    AppSettings::sync() { m_s.sync(); }

} // namespace HMS
