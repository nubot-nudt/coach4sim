#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "nubot/world_model/world_model.h"
using namespace nubot;
nubot::World_Model::World_Model(int argc,char** argv,const char * name)
{
    ros::init(argc,argv,name);
    const char * environment;
   /** 读取机器人编号，coach为编号０*/
    if((environment = getenv("AGENT"))==NULL)
    {
        ROS_ERROR("this agent number is not coach, please change the bashrc to AGENT=0");
        return ;
    }
    nh = boost::make_shared<ros::NodeHandle>();
    /** RTDB通信模块的初始化，开辟内存空间*/
    if(DB_init() != 0)
    {
        ROS_WARN("RTDB没有成功初始化内存空间");
        return;
    }

    coach_info_sub_ = nh->subscribe("nubot_coach/coachinfo", 1, &World_Model::updateCoach_info,this);
    worldmodelinfo_pub_ =  nh->advertise<nubot_common::WorldModelInfo>("worldmodel/worldmodelinfo",30);
    worldmodel_publish_timer_ = nh->createTimer(ros::Duration(0.03),&World_Model::publish,this);
    teammatesinfo_.resize(OUR_TEAM); //开辟内存空间；
    /** 当前仅仅传输五个机器人信息*/
    world_model_info_.robotinfo.resize(OUR_TEAM);
    receive_coach_count_ = ros::Time::now();;
}
nubot::World_Model::~World_Model()
{
    DB_free();           //RTDB通信模块的释放开辟内存空间
}

/** 发布从队友的得到的信息*/
void
nubot::World_Model::publish(const ros::TimerEvent &)
{
    /** 更新RTBD信息 */
#ifdef SIMULATION
    DB_get(1,TEAMMATESINFO_SIM, &teammatesinfo_sim_);                //认为Agent1为仿真的我方机器人，后面要改就改
    for(int i=0;i<OUR_TEAM;i++)
        teammatesinfo_[i]=teammatesinfo_sim_.teammatesinfo_sim[i];
#else
    for(std::size_t i = 0 ; i< OUR_TEAM ; i++)
    {

        int ltime = DB_get(i+1, TEAMMATESINFO, &teammatesinfo_[i]);
        //接收到的队友信息，并记录其间隔时间，可能表示信息无效；
        teammatesinfo_[i].robot_info_.setlifetime(ltime);
        teammatesinfo_[i].ball_info_.setlifetime(ltime);
    }
#endif

    /** 发布机器人信息 */
    world_model_info_.robotinfo.clear();
    world_model_info_.robotinfo.resize(OUR_TEAM);
    for(std::size_t i = 0 ; i< OUR_TEAM ; i++)
    {
        Robot & robot_info = teammatesinfo_[i].robot_info_;
        world_model_info_.robotinfo[i].AgentID = robot_info.getID();
        world_model_info_.robotinfo[i].pos.x      = robot_info.getLocation().x_;
        world_model_info_.robotinfo[i].pos.y      = robot_info.getLocation().y_;
        world_model_info_.robotinfo[i].heading.theta = robot_info.getHead().radian_;
        world_model_info_.robotinfo[i].vtrans.x   = robot_info.getVelocity().x_;
        world_model_info_.robotinfo[i].vtrans.y   = robot_info.getVelocity().y_;
        world_model_info_.robotinfo[i].isstuck    = robot_info.isStuck();
        world_model_info_.robotinfo[i].iskick     = robot_info.isKickoff();
        world_model_info_.robotinfo[i].isvalid    = robot_info.isValid();
        world_model_info_.robotinfo[i].vrot       = robot_info.getW();
        world_model_info_.robotinfo[i].current_role = robot_info.getCurrentRole();
        world_model_info_.robotinfo[i].current_action= robot_info.getCurrentAction();;
        world_model_info_.robotinfo[i].role_time =  robot_info.getRolePreserveTime();
        world_model_info_.robotinfo[i].target.x = robot_info.getTarget().x_;
        world_model_info_.robotinfo[i].target.y = robot_info.getTarget().y_;
        world_model_info_.robotinfo[i].isdribble = robot_info.getDribbleState();
        if(robot_info.getlifetime()>5000)
             world_model_info_.robotinfo[i].isvalid = false;    //如果太长时间没有受到信息更新，则判断下场，避免程序突然挂掉后还显示
    }

    /** 发布障碍物的信息(未测试)*/
    world_model_info_.obstacleinfo.pos.clear();
    world_model_info_.obstacleinfo.pos.resize(MAX_OBSNUMBER_CONST);
    for(std::size_t i = 0 ; i< OUR_TEAM ; i++)
        if(teammatesinfo_[i].robot_info_.isValid())         //选择第一个上场机器人看到的障碍物显示
        {
            for(int j=0; j<MAX_OBSNUMBER_CONST; j++)
            {
                world_model_info_.obstacleinfo.pos[j].x=teammatesinfo_[i].obs_info_[j].getLocation().x_;
                world_model_info_.obstacleinfo.pos[j].y=teammatesinfo_[i].obs_info_[j].getLocation().y_;
            }
            break;
        }

    /** 发布球的信息 */
    world_model_info_.ballinfo.clear();
    world_model_info_.ballinfo.resize(OUR_TEAM);
    for(std::size_t i = 0 ; i< OUR_TEAM ; i++)
    {
        BallObject ball_info;
        ball_info = teammatesinfo_[i].ball_info_;
        world_model_info_.ballinfo[i].pos.x =  ball_info.getGlobalLocation().x_;
        world_model_info_.ballinfo[i].pos.y =  ball_info.getGlobalLocation().y_;
        world_model_info_.ballinfo[i].real_pos.angle = ball_info.getRealLocation().angle_.radian_;
        world_model_info_.ballinfo[i].real_pos.radius = ball_info.getRealLocation().radius_;
        world_model_info_.ballinfo[i].velocity.x = ball_info.getVelocity().x_;
        world_model_info_.ballinfo[i].velocity.y = ball_info.getVelocity().y_;
        world_model_info_.ballinfo[i].velocity_known = ball_info.isVelocityKnown();
        world_model_info_.ballinfo[i].pos_known      = ball_info.isLocationKnown();
        if(ball_info.getlifetime()>5000)
            world_model_info_.ballinfo[i].pos_known = false;    //如果太长时间没有受到信息更新，则判断下场，避免程序突然挂掉后还显示
    }

    /**  发布传球信息*/
    world_model_info_.pass_cmd.catch_id = -1;
    world_model_info_.pass_cmd.pass_id = -1;
    world_model_info_.pass_cmd.is_dynamic_pass  = false;
    world_model_info_.pass_cmd.is_static_pass  = false;
    world_model_info_.pass_cmd.is_passout = false;
    world_model_info_.pass_cmd.is_valid = false;
    for(int i = 0 ; i < OUR_TEAM; i++)
    {
        Teammatesinfo & teammates = teammatesinfo_[i];
        if(teammates.pass_cmds_.isvalid && (teammates.pass_cmds_.is_static_pass||teammates.pass_cmds_.is_dynamic_pass||teammates.pass_cmds_.is_passout))
        {
            world_model_info_.pass_cmd.catch_id = teammates.pass_cmds_.catchrobot_id;
            world_model_info_.pass_cmd.pass_id  = teammates.pass_cmds_.passrobot_id;
            world_model_info_.pass_cmd.is_dynamic_pass  = teammates.pass_cmds_.is_dynamic_pass;
            world_model_info_.pass_cmd.is_static_pass   = teammates.pass_cmds_.is_static_pass;
            world_model_info_.pass_cmd.is_passout  = teammates.pass_cmds_.is_passout;
            world_model_info_.pass_cmd.is_valid    = teammates.pass_cmds_.isvalid;
            world_model_info_.pass_cmd.pass_pt.x   = teammates.pass_cmds_.pass_pt.x_;
            world_model_info_.pass_cmd.pass_pt.y   = teammates.pass_cmds_.pass_pt.y_;
            world_model_info_.pass_cmd.catch_pt.x  = teammates.pass_cmds_.catch_pt.x_;
            world_model_info_.pass_cmd.catch_pt.y  = teammates.pass_cmds_.catch_pt.y_;
        }
    }
    worldmodelinfo_pub_.publish(world_model_info_);
}

void World_Model::updateCoach_info(const nubot_common::CoachInfo &_coach_msg)
{
    coach_info_.MatchMode=_coach_msg.MatchMode;
    coach_info_.MatchType=_coach_msg.MatchType;
    if(DB_put(MESSAGEFROMCOACHINFO, &coach_info_) == -1)
    {
        DB_free();
        ROS_ERROR("RTDB error, please restart");
        return;
    }
}

