#include <coach2refbox.h>

using namespace nubot;


Coach2refbox::Coach2refbox()
{
    actions_[0]="stucked";
    actions_[1]="penalty";
    actions_[2]="can not see ball";
    actions_[3]="see not dribble ball";
    actions_[4]="turn for shoot";
    actions_[5]="at shoot situation";
    actions_[6]="turn to pass";
    actions_[7]="static pass";
    actions_[8]="avoid obstacles";
    actions_[9]="catch positioned";
    actions_[10]="postitoned";
    actions_[11]="positioned static";
    actions_[12]="no action";
}

Coach2refbox::~Coach2refbox()
{

}

void Coach2refbox::packWorldmodel_(Robot2coach_info *robot2coach_info_)
{
    cleanPacket_();

    //Team level setters
    nubotpacket_.setTeamIntention(QString("active"));

    //Robot level setters
    for(int i=0;i<OUR_TEAM;i++)
    {
        if(robot2coach_info_->RobotInfo_[i].isValid())
        {
            nubotpacket_.addRobot(i+1);
            nubotpacket_.setRobotPose(i+1,robot2coach_info_->RobotInfo_[i].getLocation(),robot2coach_info_->RobotInfo_[i].getHead());
            nubotpacket_.setRobotVelocity(i+1,robot2coach_info_->RobotInfo_[i].getVelocity());
            nubotpacket_.setRobotTargetPose(i+1,robot2coach_info_->RobotInfo_[i].getTarget());
            nubotpacket_.setRobotIntention(i+1,actions_[robot2coach_info_->RobotInfo_[i].getCurrentAction()]);
            nubotpacket_.setRobotBatteryLevel(i+1,0.5);
            nubotpacket_.setRobotBallPossession(i+1,robot2coach_info_->RobotInfo_[i].getDribbleState());
        }
    }

    //Ball setters
    for(int i=0;i<OUR_TEAM;i++)
    {
        if(robot2coach_info_->BallInfo_[i].isLocationKnown())
        {
            float confidence=0.001*(1000-(robot2coach_info_->RobotInfo_[i].getLocation().distance(robot2coach_info_->BallInfo_[i].getGlobalLocation())));
            nubotpacket_.addBall(robot2coach_info_->BallInfo_[i].getGlobalLocation(),
                                 robot2coach_info_->BallInfo_[i].getVelocity(),
                                 confidence);
        }
    }

    //Obstacles setters
    for(int i=0;i<OUR_TEAM;i++)
    {
        if(robot2coach_info_->RobotInfo_[i].isValid())
        {
            vector<DPoint> _obstacles=robot2coach_info_->Obstacles_[i];
            for(int i=0;i<_obstacles.size();i++)
            {
                nubotpacket_.addObstacle(_obstacles[i],DPoint(0,0));
            }
        }
    }

    //ageMs setters
    nubotpacket_.setAgeMilliseconds(90);

    //update JSON
    jsonSize=nubotpacket_.getSize();
    QJsonDocument document;
    document.setObject(*nubotpacket_.jsonObject_);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);            //把这个用tcpip传上去
    //qDebug()<<byte_array<<'\0';
}

void Coach2refbox::cleanPacket_()
{
    nubotpacket_.mPacket_.balls_.clear();
    nubotpacket_.mPacket_.robots_.clear();
    nubotpacket_.mPacket_.obstacles_.clear();
}
