#ifndef _NUBOT_World_Model_H_
#define _NUBOT_World_Model_H_

#include "nubot/core/core.hpp"
#include "nubot/world_model/ball.h"
#include "nubot/world_model/robot.h"
#include "nubot/world_model/obstacles.h"
#include "nubot/world_model/teammatesinfo.h"
#include <std_msgs/Header.h>
#include <std_msgs/String.h>
#include <nubot_common/WorldModelInfo.h>
#include <nubot_common/CoachInfo.h>
#include "nubot/rtdb/rtdb_api.h"
#include "nubot/rtdb/rtdb_user.h"
#include "ros/ros.h"
#include <semaphore.h>
#include <fstream>

namespace nubot {
class World_Model
{

public:
    World_Model(int argc,char** argv,const char * name);
    ~World_Model();
    void publish(const ros::TimerEvent &);
    void updateCoach_info(const nubot_common::CoachInfo & _coach_msg);
public:
    std::vector<Teammatesinfo> teammatesinfo_;
    Obstacles obstacles_;
    nubot_common::WorldModelInfo world_model_info_;
    MessageFromCoach coach_info_;

    ros::Subscriber   coach_info_sub_;
    ros::Publisher    worldmodelinfo_pub_;
    ros::Time         receive_coach_count_;
    ros::Timer        worldmodel_publish_timer_;
    boost::shared_ptr<ros::NodeHandle> nh;
};

}


#endif // _NUBOT_World_Model_H_
