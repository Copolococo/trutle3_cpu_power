#include <cstdio>
#include <ctime>
#include <cerrno>    /* used for error handling */
#include <cstring>   /* nedded as well */
#include <stdexcept> /* cpp error handling */
#include <exception> /* cpp error handling */

#include <fcntl.h>  /* used for open(path) */
#include <unistd.h> /* used for close(fd) */
#include <sys/ioctl.h>

#include <linux/rtc.h> /* ioctl structs and macros for rtc */

using std::fclose;
using std::fopen;
using std::strerror;
using std::tm;

class PowerManager
{
public:
    /**
     * Constructor
     * @param path_to_rtc path to the RTC device i.e.: /dev/rtc0 defaults to /dev/rtc
     */
    PowerManager(const char *path_to_rtc = "/dev/rtc");

    /**
     * Destructor
     */
    ~PowerManager();

    void getRTCTime(struct rtc_time *time);

    void getRTCWakeAlarm(struct rtc_time *wake_time);

    void setRTCWakeAlarm(struct rtc_time *wake_time);

    void suspend2RAM();

    inline void rtc2Tm(struct rtc_time *rtc_time, struct tm *tm);

    inline void tm2Rtc(struct tm *tm, struct rtc_time *rtc_time);

    /**
     * Set RTCWakeAlarm using sysfs
     * equivelent to:
     *      # cat +<secs> > /sys/class/rtc/rtc[0-9]/wakealarm
     * @param secs number of seconds to wait until RTC signal wakes CPU
     */
    // void setRTCWakeAlarm(unsigned int secs);

private:
    int rtc_dev_fd_;

    // FILE *rtc_wake_alarm_fp_;
};