#pragma once
#include <QSettings>
#include <QString>
#include <QObject>

namespace HMS {

class AppSettings : public QObject {
    Q_OBJECT
public:
    static AppSettings& instance();

    // Hospital info (used in PDFs)
    QString hospitalName()    const;
    void    setHospitalName(const QString& v);
    QString hospitalAddress() const;
    void    setHospitalAddress(const QString& v);
    QString hospitalPhone()   const;
    void    setHospitalPhone(const QString& v);
    QString hospitalEmail()   const;
    void    setHospitalEmail(const QString& v);
    QString hospitalLogoPath()const;
    void    setHospitalLogoPath(const QString& v);

    // Theme
    bool    darkMode()        const;
    void    setDarkMode(bool on);

    // Session / security
    int     sessionTimeoutMinutes() const;
    void    setSessionTimeoutMinutes(int m);
    QString lastUser()        const;
    void    setLastUser(const QString& u);

    // UI preferences
    bool    sidebarCollapsed()const;
    void    setSidebarCollapsed(bool v);

    void sync();

signals:
    void darkModeChanged(bool on);
    void hospitalInfoChanged();

private:
    AppSettings();
    QSettings m_s;
};

} // namespace HMS
