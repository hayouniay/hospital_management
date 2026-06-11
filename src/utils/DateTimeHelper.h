#pragma once
#include <QString>
#include <QDateTime>
#include <QDate>

namespace HMS {

class DateTimeHelper {
public:
    // Human-readable relative time: "2 hours ago", "yesterday", "3 days ago"
    static QString relativeTime(const QDateTime& dt);
    // Duration in minutes → "1h 30m"
    static QString formatDuration(int minutes);
    // Age from date of birth
    static int ageFromDOB(const QDate& dob);
    // Is the date today?
    static bool isToday(const QDate& d);
    // Is the date in the past?
    static bool isPast(const QDate& d);
    // Format for display: "Mon, Jan 6 2025"
    static QString displayDate(const QDate& d);
    static QString displayDateTime(const QDateTime& dt);
};

} // namespace HMS
