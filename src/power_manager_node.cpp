#include <ctime>

#include <ros/ros.h>
#include "power_manager.h"
#include "turtle3_cpu_power/RTCTime.h"
#include "turtle3_cpu_power/GetRTCAlarm.h"
#include "turtle3_cpu_power/GetRTCTime.h"
#include "turtle3_cpu_power/SetRTCAlarm.h"
#include "turtle3_cpu_power/Suspend.h"
#include "turtle3_cpu_power/SuspendUntil.h"
#include "turtle3_cpu_power/SuspendFor.h"

class Turtle3PowerManager : private PowerManager
{
public:
    Turtle3PowerManager(ros::NodeHandle nh, const char *path_to_rtc) : nh_{nh}, PowerManager(path_to_rtc)
    {
        ros::ServiceServer srv_get_rtc_time_ = nh_.advertiseService("get_rtc_time", &Turtle3PowerManager::getRTCTimeCallback, this);
        ros::ServiceServer srv_get_rtc_alrm_ = nh_.advertiseService("get_rtc_wake", &Turtle3PowerManager::getRTCAlarmCallback, this);
        ros::ServiceServer srv_set_rtc_alrm_ = nh_.advertiseService("set_rtc_wake", &Turtle3PowerManager::setRTCAlarmCallback, this);
        ros::ServiceServer srv_suspend_ = nh_.advertiseService("suspend", &Turtle3PowerManager::suspendCallback, this);
        ros::ServiceServer srv_suspend_until_ = nh_.advertiseService("suspend_until", &Turtle3PowerManager::suspendUntilCallback, this);
        ros::ServiceServer srv_suspend_for_ = nh_.advertiseService("suspend_for", &Turtle3PowerManager::suspendForCallback, this);

        ros::spin();
    }

private:
    ros::NodeHandle nh_;
    ros::ServiceServer srv_get_rtc_alrm_;
    ros::ServiceServer srv_get_rtc_time_;
    ros::ServiceServer srv_set_rtc_alrm_;
    ros::ServiceServer srv_suspend_;
    ros::ServiceServer srv_suspend_until_;
    ros::ServiceServer srv_suspend_for_;

    bool
    getRTCAlarmCallback(turtle3_cpu_power::GetRTCAlarm::Request &req,
                        turtle3_cpu_power::GetRTCAlarm::Response &res)
    {
        struct rtc_time wake_time;
        try
        {
            getRTCWakeAlarm(&wake_time);
        }
        catch (const std::exception &e)
        {
            ROS_ERROR("Failed to get RTC wake time");
            ROS_ERROR_STREAM(e.what());
            return false;
        }

        res.wake_time.sec = wake_time.tm_sec;
        res.wake_time.min = wake_time.tm_min;
        res.wake_time.hour = wake_time.tm_hour;
        // these below might not be used if RTC doesnt supportalarms > 24h
        res.wake_time.mday = wake_time.tm_mday;
        res.wake_time.mon = wake_time.tm_mon;
        res.wake_time.year = wake_time.tm_year;
    }

    bool getRTCTimeCallback(turtle3_cpu_power::GetRTCTime::Request &req,
                            turtle3_cpu_power::GetRTCTime::Response &res)
    {
        struct rtc_time wake_time;
        try
        {
            getRTCTime(&wake_time);
        }
        catch (const std::exception &e)
        {
            ROS_ERROR("Failed to get RTC time");
            ROS_ERROR_STREAM(e.what());
            return false;
        }

        res.wake_time.sec = wake_time.tm_sec;
        res.wake_time.min = wake_time.tm_min;
        res.wake_time.hour = wake_time.tm_hour;
        // these below might not be used if RTC doesnt supportalarms > 24h
        res.wake_time.mday = wake_time.tm_mday;
        res.wake_time.mon = wake_time.tm_mon;
        res.wake_time.year = wake_time.tm_year;
    }

    bool setRTCAlarmCallback(turtle3_cpu_power::SetRTCAlarm::Request &req,
                             turtle3_cpu_power::SetRTCAlarm::Response &res)
    {
        struct rtc_time wake_time;
        wake_time.tm_sec = req.wake_time.sec;
        wake_time.tm_min = req.wake_time.min;
        wake_time.tm_hour = req.wake_time.hour;
        // these below might not be used if RTC doesnt support alarms > 24h
        wake_time.tm_mday = req.wake_time.mday;
        wake_time.tm_mon = req.wake_time.mon;
        wake_time.tm_year = req.wake_time.year;
        // these are just unused
        wake_time.tm_wday = -1;
        wake_time.tm_yday = -1;
        wake_time.tm_isdst = -1;

        try
        {
            setRTCWakeAlarm(&wake_time);
        }
        catch (const std::exception &e)
        {
            ROS_WARN_STREAM(e.what());
        }

        return true;
    }

    bool suspendUntilCallback(turtle3_cpu_power::SuspendUntil::Request &req,
                              turtle3_cpu_power::SuspendUntil::Response &res)
    {
        struct rtc_time wake_time;
        wake_time.tm_sec = req.wake_time.sec;
        wake_time.tm_min = req.wake_time.min;
        wake_time.tm_hour = req.wake_time.hour;
        // these below might not be used if RTC doesnt support alarms > 24h
        wake_time.tm_mday = req.wake_time.mday;
        wake_time.tm_mon = req.wake_time.mon;
        wake_time.tm_year = req.wake_time.year;
        // these are just unused
        wake_time.tm_wday = -1;
        wake_time.tm_yday = -1;
        wake_time.tm_isdst = -1;

        try
        {
            setRTCWakeAlarm(&wake_time);
        }
        catch (const std::exception &e)
        {
            ROS_WARN_STREAM(e.what());
        }

        switch (req.suspend_state)
        {
        case 0:
            ROS_INFO("Suspending to RAM");
            try
            {
                suspend2RAM();
            }
            catch (const std::exception &e)
            {
                ROS_WARN("Suspending failed!");
                ROS_ERROR_STREAM(e.what());
                return false;
            }

            break;
        default:
            ROS_WARN("The state %d is not associated with any function, maybe it hasn't been developed?", req.suspend_state);
            break;
        }

        return true;
    }

    bool suspendCallback(turtle3_cpu_power::Suspend::Request &req,
                         turtle3_cpu_power::Suspend::Response &res)
    {
        switch (req.suspend_state)
        {
        case 0:
            ROS_INFO("Suspending to RAM");
            try
            {
                suspend2RAM();
            }
            catch (const std::exception &e)
            {
                ROS_WARN("Suspending failed!");
                ROS_ERROR_STREAM(e.what());
                return false;
            }
            break;
        default:
            ROS_WARN("The state %d is not associated with any function, maybe it hasn't been developed?", req.suspend_state);
            break;
        }

        return true;
    }

    bool suspendForCallback(turtle3_cpu_power::SuspendFor::Request &req,
                            turtle3_cpu_power::SuspendFor::Response &res)
    {
        // using /sys-utils/rtcwake.c as reference
        std::tm target_time;
        struct rtc_time rtc_time;

        try
        {
            getRTCTime(&rtc_time);
        }
        catch (const std::exception &e)
        {
            ROS_ERROR("Failed to get RTC time");
            ROS_ERROR_STREAM(e.what());
            return false;
        }

        target_time.tm_sec = rtc_time.tm_sec + req.duration.sec;
        target_time.tm_min = rtc_time.tm_min + req.duration.min;
        target_time.tm_hour = rtc_time.tm_hour + req.duration.hour;
        target_time.tm_mday = rtc_time.tm_mday + req.duration.mday; // not supported in some RTCs
        target_time.tm_mon = rtc_time.tm_mon + req.duration.mon;    // not supported in some RTCs
        target_time.tm_year = rtc_time.tm_year + req.duration.year; // not supported in some RTCs
        target_time.tm_isdst = rtc_time.tm_isdst;

        // make it a valid structure in GMT time
        std::time_t tmp = std::mktime(&target_time);
        target_time = *std::gmtime(&tmp);

        // convert back to the structure expected by the RTC
        rtc_time.tm_sec = target_time.tm_sec;
        rtc_time.tm_min = target_time.tm_min;
        rtc_time.tm_hour = target_time.tm_hour;
        rtc_time.tm_mday = target_time.tm_mday;
        rtc_time.tm_mon = target_time.tm_mon;
        rtc_time.tm_year = target_time.tm_year;
        rtc_time.tm_wday = -1;
        rtc_time.tm_yday = -1;
        rtc_time.tm_isdst = -1;

        try
        {
            setRTCWakeAlarm(&rtc_time);
        }
        catch (const std::exception &e)
        {
            ROS_WARN_STREAM(e.what());
        }

        switch (req.suspend_state)
        {
        case 0:
            ROS_INFO("Suspending to RAM");
            try
            {
                suspend2RAM();
            }
            catch (const std::exception &e)
            {
                ROS_WARN("Suspending failed!");
                ROS_ERROR_STREAM(e.what());
                return false;
            }

            break;
        default:
            ROS_WARN("The state %d is not associated with any function, maybe it hasn't been developed?", req.suspend_state);
            break;
        }

        return true;
    }
};

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "turtle3_cpu_power");
    ros::NodeHandle nh;
    try
    {
        Turtle3PowerManager node(nh, "/dev/rtc");
    }
    catch (const std::exception &e)
    {
        ROS_ERROR_STREAM(e.what());
        ROS_ERROR("Check if current user has permissions to access the following interfaces:\n"
                  "/sys/power/mem_sleep rw\n"
                  "/sys/power/state     rw\n"
                  "/dev/rtc*            rw\n");
    }

    return 0;
}