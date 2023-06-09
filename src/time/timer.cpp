#include <chrono>
#include <sstream>
#include <string>

#include "zest/logger/logger.h"
#include "zest/time/timer.h"
#include <zest/time/profiler.h>

using namespace std::chrono;
using namespace Zest;

namespace Zest
{

timer globalTimer;

double timer_to_seconds(nanoseconds value)
{
    return double(value.count() / 1000000000.0);
}

double timer_to_ms(nanoseconds value)
{
    return double(value.count() / 1000000.0);
}

/*
std::chrono::sys_time<std::chrono::milliseconds> sys_time_from_iso_8601(const std::string& str)
{
    std::istringstream in{ str };
    std::chrono::sys_time<std::chrono::milliseconds> tp;
    in >> std::chrono::parse("%FT%TZ", tp);
    if (in.fail())
    {
        in.clear();
        in.exceptions(std::ios::failbit);
        in.str(str);
        in >> std::chrono::parse("%FT%T%Ez", tp);
    }
    return tp;
}

DateTime datetime_from_iso_8601(const std::string& str)
{
    auto t = sys_time_from_iso_8601(str);
    return DateTime(duration_cast<seconds>(t.time_since_epoch()));
}

std::string datetime_to_iso_8601_string(DateTime dt)
{
    return std::chrono::format("%y_%m_%d_%H_%M_%S", dt);
}
*/

std::string datetime_to_iso_8601(DateTime tp)
{
    //return std::chrono::format("%FT%TZ", std::chrono::floor<seconds>(tp));
    return "";
}

DateTime datetime_now()
{
    return DateTime(std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
}

DateTime datetime_from_seconds(uint64_t t)
{
    return DateTime(std::chrono::seconds(t));
}

DateTime datetime_from_seconds(std::chrono::seconds s)
{
    return DateTime(s);
}

DateTime datetime_from_timer_start(timer& timer)
{
    return DateTime(seconds(timer_to_epoch_utc_seconds(timer)));
}

// Convert DateTime to string
std::string datetime_to_string(DateTime d, DateTimeFormat format)
{
    /*
    auto dp = std::chrono::floor<std::chrono::days>(d);
    auto ymd = std::chrono::year_month_day{ dp };
    auto tm = make_time(d - dp);
    std::ostringstream str;
    switch (format)
    {
    default:
    case DateTimeFormat::JsonDayMonthYear:
        str << ymd.day() << "/" << ymd.month() << "/" << ymd.year();
        return str.str();
    case DateTimeFormat::YearMonthDay:
        str << ymd.day() << " " << ymd.month() << " " << ymd.year();
        return str.str();
    case DateTimeFormat::YearMonthDayTime:
        str << ymd.day() << " " << ymd.month() << " " << ymd.year() << " - " << tm;
        return str.str();
    case DateTimeFormat::DayMonth:
        str << ymd.day() << " " << ymd.month();
        return str.str();
    case DateTimeFormat::Month:
        str << ymd.month();
        return str.str();
    case DateTimeFormat::Year:
        str << ymd.year();
        return str.str();
    case DateTimeFormat::Day:
        str << ymd.day();
        return str.str();
    case DateTimeFormat::Time:
        str << tm;
        return str.str();
    case DateTimeFormat::Value:
        str << d.time_since_epoch().count();
        return str.str();
    }
    */
    return "";
}

} // namespace Zest
