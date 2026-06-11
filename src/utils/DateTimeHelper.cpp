#include "DateTimeHelper.h"

namespace HMS {

QString DateTimeHelper::relativeTime(const QDateTime& dt) {
    const qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 60)          return "just now";
    if (secs < 3600)        return QString("%1 min ago").arg(secs / 60);
    if (secs < 86400)       return QString("%1 hr ago").arg(secs / 3600);
    if (secs < 172800)      return "yesterday";
    if (secs < 2592000)     return QString("%1 days ago").arg(secs / 86400);
    if (secs < 31536000)    return QString("%1 months ago").arg(secs / 2592000);
    return QString("%1 years ago").arg(secs / 31536000);
}

QString DateTimeHelper::formatDuration(int minutes) {
    if (minutes < 60) return QString("%1 min").arg(minutes);
    int h = minutes / 60, m = minutes % 60;
    return m > 0 ? QString("%1h %2m").arg(h).arg(m) : QString("%1h").arg(h);
}

int DateTimeHelper::ageFromDOB(const QDate& dob) {
    return dob.daysTo(QDate::currentDate()) / 365;
}

bool DateTimeHelper::isToday(const QDate& d) {
    return d == QDate::currentDate();
}

bool DateTimeHelper::isPast(const QDate& d) {
    return d < QDate::currentDate();
}

QString DateTimeHelper::displayDate(const QDate& d) {
    return d.toString("ddd, MMM d yyyy");
}

QString DateTimeHelper::displayDateTime(const QDateTime& dt) {
    return dt.toString("MMM d yyyy  hh:mm AP");
}

} // namespace HMS
