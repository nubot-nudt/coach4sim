#ifndef PACKET2REFBOX_H
#define PACKET2REFBOX_H

#include "robot2coach.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#define PI 3.1415926
using namespace std;
namespace nubot {

typedef struct
{
    quint8  robotId;
    DPoint2s  pose;
    short   orien;
    DPoint2s  velocity;
    DPoint2s  targetPose;
    QString intention;
    float   batteryLevel;
    bool    hasBall;
} robotStructure;
typedef std::vector<robotStructure> robotList;

typedef struct
{
    DPoint2s position;
    DPoint2s velocity;
    float  confidence;
} ballStructure;
typedef std::vector<ballStructure> ballList;

typedef struct
{
    DPoint2s position;
    DPoint2s velocity;
    float    radius;
    float    confidence;
} obstacleStructure;
typedef std::vector<obstacleStructure> obstacleList;

class Nubotpacket
{
public:
    robotList     robots_;       //机器人的信息
    ballList      balls_;        //球的信息
    obstacleList  obstacles_;    //障碍物信息
    QString type_;
    QString teamName_;
    QString globalIntention_;
    double age_;
};

class Packet2refbox
{
public:
    Packet2refbox();
    ~Packet2refbox();

    int getSize();

    void getSerialized(QString &packet);

    //Team level setters
    void setType(const QString type);
    void setTeamIntention(const QString intention);

    //Robot level setters
    void setRobotPose(const quint8 robotId, const DPoint2s pose, const short orien);
    void setRobotTargetPose(const quint8 robotId, const DPoint2s targetpose);
    void setRobotVelocity(const quint8 robotId, const DPoint2s velocity);
    void setRobotIntention(const quint8 robotId, const QString intention);
    void setRobotBatteryLevel(const quint8 robotId, const float level);
    void setRobotBallPossession(const quint8 robotId, const bool hasBall);

    //Ball setters
    void addBall(const DPoint2s position, const DPoint2s velocity, const float confidence);

    //Obstacle setters
    void addObstacle(const DPoint2s position, const DPoint2s velocity);

    //Global setters
    void setAgeMilliseconds(const double age=90);

public:
    Nubotpacket mPacket_;
    QJsonObject *jsonObject_;

    //Robot functions
    void isRobotPresent(const quint8 robotId, bool &isPresent, int &index);
    void addRobot(const quint8 robotId);

    //JSON functions
    void cleanupJSONObject();
    void generateJSON();
    void addRobotsJSON(QJsonArray &array);
    void addBallsJSON(QJsonArray &array);
    void addObstaclesJSON(QJsonArray &array);
};
}

#endif // PACKET2REFBOX_H
