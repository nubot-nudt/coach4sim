#ifndef __NUBOT_CORE_HPP__
#define __NUBOT_CORE_HPP__


#include "Circle.hpp"
#include "Angle.hpp"
#include "DPoint.hpp"
#include "PPoint.hpp"
#include "Line.hpp"

//#define  SIMULATION
#define  NET_TYPE "eth0"

#ifdef  SIMULATION
const double WIDTHRATIO4FIELD = 1;
const double WIDTH_RATIO= 1 ;
const double ConstDribbleDisFirst  = 50;
const double ConstDribbleDisSecond = 40;
#else
const double WIDTHRATIO4FIELD = 8.0/12.0 ;
const double WIDTH_RATIO= 8.0/12.0 ;
const double ConstDribbleDisFirst  = 40;
const double ConstDribbleDisSecond = 35;
#endif
//using namespace nubot;
const int MAX_OBSNUMBER_CONST = 10;
const int NOT_DATAUPDATE = 1000; //数据没有更新的阈值，比如通信过程中时间大于300ms为更新数据，默认为失效
const int OUR_TEAM = 5 ;        //自己机器人个数
const int OPP_TEAM = 7 ;        //对方的机器人个数
const int ROLENUM = 7;

/** 比赛模式的一些定义，*/

enum MatchMode { STOPROBOT   = 0,
                 OUR_KICKOFF = 1,
                 OPP_KICKOFF = 2,
                 OUR_THROWIN = 3,
                 OPP_THROWIN = 4,
                 OUR_PENALTY = 5,
                 OPP_PENALTY = 6,
                 OUR_GOALKICK = 7 ,
                 OPP_GOALKICK = 8,
                 OUR_CORNERKICK = 9,
                 OPP_CORNERKICK = 10,
                 OUR_FREEKICK = 11,
                 OPP_FREEKICK = 12,
                 DROPBALL     = 13,
                 STARTROBOT   = 15,
                 PARKINGROBOT = 25,
                 TEST = 27
               };

enum TestMode
{
    Test_Stop =0,
    Move_NoBall_NoAvoid = 10,
    Move_NoBall_Avoid   = 11,
    Move_Ball_NoAvoid   = 12,
    Move_Ball_Avoid     = 13,
    Pass_Ball  =2,
    Catch_Ball =3,
    Shoot_Ball =4,
    Location_test =5,
    Circle_test =6
};

struct obs_info
{
    nubot::PPoint polar_pt;
    nubot::DPoint world_pt;
    double HRZ[4];
    double HRH[4*4];
};

#endif 
