#include "power_manager.h"

PowerManager::PowerManager(const char *path_to_rtc)
{
    rtc_dev_fd_ = open(path_to_rtc, O_RDWR);

    if (rtc_dev_fd_ < 0)
        throw std::runtime_error(strerror(errno));
}

PowerManager::~PowerManager()
{
    close(rtc_dev_fd_);
}

void PowerManager::getRTCTime(struct rtc_time *time)
{
    if (ioctl(rtc_dev_fd_, RTC_RD_TIME, (struct rtc_time *)time) < 0)
        throw std::runtime_error(strerror(errno));
}

void PowerManager::getRTCWakeAlarm(struct rtc_time *wake_time)
{
    if (ioctl(rtc_dev_fd_, RTC_ALM_READ, wake_time) < 0)
    {
        throw std::runtime_error(strerror(errno));
    }
}

void PowerManager::setRTCWakeAlarm(struct rtc_time *wake_time)
{
    struct rtc_wkalrm wake;
    wake.time = *wake_time;
    wake.enabled = 1;

    if (ioctl(rtc_dev_fd_, RTC_WKALM_SET, &wake) < 0)
    {
        char *fallback_reason;
        std::sprintf(fallback_reason, "Fallback required: %s\nDoes this rtc support alarms > 24h?", strerror(errno));

        // fallback to older method
        if (ioctl(rtc_dev_fd_, RTC_ALM_SET, wake_time) < 0)
            throw std::runtime_error(strerror(errno));

        if (ioctl(rtc_dev_fd_, RTC_AIE_ON, 0) < 0)
            throw std::runtime_error(strerror(errno));

        throw std::runtime_error(fallback_reason);
    }
}

void PowerManager::suspend2RAM()
{
    FILE *mem_slp_fp;

    mem_slp_fp = fopen("/sys/power/mem_sleep", "w");
    if (!mem_slp_fp)
        throw std::runtime_error(strerror(errno));

    if (fprintf(mem_slp_fp, "deep") < 0)
        throw std::runtime_error(strerror(errno));

    fclose(mem_slp_fp);

    FILE *state_fp;

    state_fp = fopen("/sys/power/state", "w");
    if (!state_fp)
        throw std::runtime_error(strerror(errno));

    if (fprintf(state_fp, "mem") < 0)
        throw std::runtime_error(strerror(errno));

    fclose(state_fp);
}

inline void PowerManager::rtc2Tm(struct rtc_time *rtc_time, struct tm *tm)
{
    tm->tm_sec = rtc_time->tm_sec;
    tm->tm_min = rtc_time->tm_min;
    tm->tm_hour = rtc_time->tm_hour;
    tm->tm_mday = rtc_time->tm_mday;
    tm->tm_mon = rtc_time->tm_mon;
    tm->tm_year = rtc_time->tm_year;
    tm->tm_isdst = rtc_time->tm_isdst;
}

inline void PowerManager::tm2Rtc(struct tm *tm, struct rtc_time *rtc_time)
{
    rtc_time->tm_sec = tm->tm_sec;
    rtc_time->tm_min = tm->tm_min;
    rtc_time->tm_hour = tm->tm_hour;
    rtc_time->tm_mday = tm->tm_mday;
    rtc_time->tm_mon = tm->tm_mon;
    rtc_time->tm_year = tm->tm_year;
    rtc_time->tm_isdst = tm->tm_isdst;
}