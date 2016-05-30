#ifndef COACH2REFBOX_H
#define COACH2REFBOX_H

#include <packet2Refbox.h>

using namespace std;
namespace nubot {

class Coach2refbox
{
public:
    Coach2refbox();
    ~Coach2refbox();

    void packWorldmodel_(Robot2coach_info *robot2coach_info_);
    void cleanPacket_();

public:
    Packet2refbox nubotpacket_;
    QString actions_[13];
    int  jsonSize;

    //简化上传数据
    DPoint2s _robot_pos[OUR_TEAM];
    DPoint2s _robot_vel[OUR_TEAM];
    DPoint2s _robot_target[OUR_TEAM];
    short _robot_ori[OUR_TEAM];
    DPoint2s _ball_pos[OUR_TEAM];
    DPoint2s _ball_vel[OUR_TEAM];
    DPoint2s _obstacles[MAX_OBSNUMBER_CONST*2];
};
}

#endif // COACH2REFBOX_H
