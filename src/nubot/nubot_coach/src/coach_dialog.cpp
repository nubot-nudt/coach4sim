#include "coach_dialog.h"

Dialog::Dialog(nubot::Robot2coach_info & robot2coach, nubot::MessageFromCoach & coach2robot, QWidget *parent) :
    QDialog(parent)
{
    robot2coach_info_= & robot2coach;                                 //将指针指向值空间
    coach2robot_info_= & coach2robot;
    json_parse_=new nubot::JSONparse;                                 //用于解析json文件
    coach2refebox_=new nubot::Coach2refbox;                           //生成上传的json文件

    scene_=new QGraphicsScene();                                      //尝试采用新的绘图方法

    QTimer *timer=new QTimer(this);
    tcpSocket_=new QTcpSocket(this);
    QByteArray *buffer = new QByteArray();
    qint32 *s = new qint32(0);
    buffers.insert(tcpSocket_, buffer);
    sizes.insert(tcpSocket_, s);

    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));        //定时函数的槽函数
    //connect(tcpSocket_, SIGNAL(readyRead()), this, SLOT(OnReceive_()));    //tcp/ip通信时用到的槽函数
    connect(tcpSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError_(QAbstractSocket::SocketError)));

    ui=new Ui::Dialog;
    ui->setupUi(this);
    ui->radioButton->setChecked(true);
    ui->display->setScene(scene_);
    timer->start(30);                                                            //定时函数，每30ms

    this->setFixedSize(1110,700);                                                //固定窗口大小

    field_img_init_.load("../../../src/nubot/nubot_coach/source/field.png");     //载入场地图像
    field_img_home_.load("../../../src/nubot/nubot_coach/source/field_home.png");
    robot_img_[0].load("../../../src/nubot/nubot_coach/source/NUM1.png");        //载入机器人图像
    robot_img_[1].load("../../../src/nubot/nubot_coach/source/NUM2.png");
    robot_img_[2].load("../../../src/nubot/nubot_coach/source/NUM3.png");
    robot_img_[3].load("../../../src/nubot/nubot_coach/source/NUM4.png");
    robot_img_[4].load("../../../src/nubot/nubot_coach/source/NUM5.png");
    ball_img_.load("../../../src/nubot/nubot_coach/source/ball.png");
    obs_img_.load("../../../src/nubot/nubot_coach/source/obstacles.png");

    for(int i=0;i<OUR_TEAM;i++)
        robot_img_[i]=robot_img_[i].scaled(30,30);
    ball_img_=ball_img_.scaled(20,20);
    obs_img_=obs_img_.scaled(30,30);

    field_=scene_->addPixmap(field_img_init_);             //载入初始化球场
    scene_->setSceneRect(0,0,700,467);                     //固定显示区域
    ball_=scene_->addPixmap(ball_img_);                    //载入球
    ball_->setPos(900,900);

    for(int i=0;i<OUR_TEAM;i++)
    {
        robot_[i]=scene_->addPixmap(robot_img_[i]);        //载入机器人
        robot_[i]->setPos(900,900);
    }
    for(int i=0;i<MAX_OBSNUMBER_CONST*2;i++)
    {
        obstacle_[i]=scene_->addPixmap(obs_img_);                 //载入障碍物
        obstacle_[i]->setPos(900,900);                            //初始位置放到(900,900),不出现在视野里
    }
    velocity_=scene_->addLine(900,900,901,901,QPen(Qt::red, 5));  //初始化速度曲线位置

    //一系列的标志初始化
    isObs_display_=false;
    isConnect_RefBox_=false;

    isAtHome_=0;
    teamflag_=0;
    score_cyan_=0;
    score_magenta_=0;
    groundflag_=1;

    //ui->change_ground->setStyleSheet("border-image: url(../../../src/nubot/nubot_coach/source/left2right.png)");
    //ui->magenta->setStyleSheet("background-color:rgb(245, 12, 198);border:2px groove gray;border-radius:10px;padding:2px 4px;");
    ui->currentState->setText("STOP ROBOT");                                  //显示当前机器人状态
}

Dialog::~Dialog()
{
    delete ui;                                                                 //不能同时关闭ui线程和ros线程
}

void Dialog::paintEvent(QPaintEvent *event)
{
    //根据选择画图
    restItems_();
    if(display_choice_==0)
    {
        //绘制机器人
        for(int i=0;i<OUR_TEAM;i++)
        {
            if(robot2coach_info_->RobotInfo_[i].isValid())
            {
                robot_[i]->setPos(groundflag_*robot2coach_info_->RobotInfo_[i].getLocation().x_*WIDTH+335,
                                  -groundflag_*robot2coach_info_->RobotInfo_[i].getLocation().y_*HEIGHT+218.5);
                robot_[i]->setTransformOriginPoint(15,15);
                robot_[i]->setRotation((groundflag_-1)*90-robot2coach_info_->RobotInfo_[i].getHead().degree());
            }
        }

        //绘制球
        int num=ballPos_fuse();
        if(num)
            ball_->setPos(groundflag_*robot2coach_info_->BallInfo_[num-1].getGlobalLocation().x_*WIDTH+340,
                          -groundflag_*robot2coach_info_->BallInfo_[num-1].getGlobalLocation().y_*HEIGHT+223.5);

        //绘制融合后的障碍物
        if(isObs_display_)
            for(int i=0;i<OUR_TEAM;i++)
            {
                if(robot2coach_info_->RobotInfo_[i].isValid())     //存在任意一个机器人在场
                {
                    if(robot2coach_info_->Opponents_.size())
                        for(int i=0;i<robot2coach_info_->Opponents_.size();i++)
                            obstacle_[i]->setPos(groundflag_*robot2coach_info_->Opponents_[i].x_*WIDTH+335,
                                                 -groundflag_*robot2coach_info_->Opponents_[i].y_*HEIGHT+218.5);
                    break;
                }
            }
    }
    else
    {
        if(robot2coach_info_->RobotInfo_[display_choice_-1].isValid())
        {
            //绘制机器人
            robot_[display_choice_-1]->setPos(groundflag_*robot2coach_info_->RobotInfo_[display_choice_-1].getLocation().x_*WIDTH+335,
                                              -groundflag_*robot2coach_info_->RobotInfo_[display_choice_-1].getLocation().y_*HEIGHT+218.5);
            robot_[display_choice_-1]->setTransformOriginPoint(15,15);
            robot_[display_choice_-1]->setRotation((groundflag_-1)*90-robot2coach_info_->RobotInfo_[display_choice_-1].getHead().degree());

            //绘制球
            if(robot2coach_info_->BallInfo_[display_choice_-1].isLocationKnown())
            {
                ball_->setPos(groundflag_*robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().x_*WIDTH+340,
                              -groundflag_*robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().y_*HEIGHT+223.5);

                //绘制当前机器人识别的球速
                velocity_->setLine(groundflag_*robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().x_*WIDTH+350,
                                   -groundflag_*robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().y_*HEIGHT+233.5,
                                   groundflag_*(robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().x_*WIDTH+robot2coach_info_->BallInfo_[display_choice_-1].getVelocity().x_)+350,
                                   -groundflag_*(robot2coach_info_->BallInfo_[display_choice_-1].getGlobalLocation().y_*HEIGHT+robot2coach_info_->BallInfo_[display_choice_-1].getVelocity().y_)+233.5);
            }
            //绘制当前机器人识别的障碍物
            if(isObs_display_)
                for(int j=0;j<robot2coach_info_->Obstacles_.size();j++)
                    obstacle_[j]->setPos(groundflag_*robot2coach_info_->Obstacles_[display_choice_-1][j].x_*WIDTH+335,
                                         -groundflag_*robot2coach_info_->Obstacles_[display_choice_-1][j].y_*HEIGHT+218.5);
        }
    }
}

void Dialog::keyPressEvent(QKeyEvent *event)               //保证安全，任何模式下空格都能发stop命令到机器人
{
    if(event->key()==Qt::Key_Space)
    {
         coach2robot_info_->MatchMode=STOPROBOT;
         ui->currentState->setText("STOP ROBOT");
    }
}

void Dialog::closeEvent(QCloseEvent *event)                //结束时关闭Ros进程
{
    //system("rosnode kill /world_model_node");
    //system("rosnode kill /rosout");
    ros::shutdown();
}

void Dialog::timerUpdate()
{
    update();                                             //重绘
    showRobot_info_();                                    //更新显示信息
    showAll_info_();
    if(isConnect_RefBox_)                                 //采用循环降低频率，不用重新写定时函数
    {
        static int count=0;
        if(count==15)
        {
            ui->text_disply->setStyleSheet("");                   //复位颜色
            count=0;
        }
        else
            count++;
    }
    if(isUpload_worldmodel_)                                 //采用循环降低频率，不用重新写定时函数
    {
        static int count1=0;
        if(count1==2)                                        //90ms一次
        {
            coach2refebox_->packWorldmodel_(robot2coach_info_);
            count1=0;
        }
        else
            count1++;
    }
    //qDebug()<<groundflag_<<" "<<teamflag_;
}

//显示的选择
void Dialog::on_radioButton_clicked()
{
    display_choice_=0;
}
void Dialog::on_radioButton_1_clicked()
{
    display_choice_=1;
}
void Dialog::on_radioButton_2_clicked()
{
    display_choice_=2;
}
void Dialog::on_radioButton_3_clicked()
{
    display_choice_=3;
}
void Dialog::on_radioButton_4_clicked()
{
    display_choice_=4;
}
void Dialog::on_radioButton_5_clicked()
{
    display_choice_=5;
}

//分数控制
void Dialog::on_Score1_up_clicked()
{
    score_cyan_=ui->Score1->value();
    score_cyan_++;
    ui->Score1->display(score_cyan_);
}
void Dialog::on_Score1_down_clicked()
{
    score_cyan_=ui->Score1->value();
    if(score_cyan_)
        score_cyan_--;
    ui->Score1->display(score_cyan_);
}
void Dialog::on_Score0_up_clicked()
{
    score_magenta_=ui->Score0->value();
    score_magenta_++;
    ui->Score0->display(score_magenta_);
}
void Dialog::on_Score0_down_clicked()
{
    score_magenta_=ui->Score0->value();
    if(score_magenta_)
        score_magenta_--;
    ui->Score0->display(score_magenta_);
}

//控制面板
void Dialog::on_startButton_clicked()
{
    coach2robot_info_->MatchType=coach2robot_info_->MatchMode;
    coach2robot_info_->MatchMode=STARTROBOT;
    ui->currentState->setText("START ROBOT");
}

void Dialog::on_stopButton_clicked()
{
    coach2robot_info_->MatchMode=STOPROBOT;
    ui->currentState->setText("STOP ROBOT");
}

void Dialog::on_kickoff_clicked()
{
    coach2robot_info_->MatchMode=OUR_KICKOFF;
    ui->currentState->setText("OUR KICKOFF");
}

void Dialog::on_kickoff_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_KICKOFF;
    ui->currentState->setText("OPP KICKOFF");
}

void Dialog::on_penalty_clicked()
{
    coach2robot_info_->MatchMode=OUR_PENALTY;
    ui->currentState->setText("OUR PENALTY");
}

void Dialog::on_penalty_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_PENALTY;
    ui->currentState->setText("OPP PENALTY");
}

void Dialog::on_corner_clicked()
{
    coach2robot_info_->MatchMode=OUR_CORNERKICK;
    ui->currentState->setText("OUR CORNER");
}

void Dialog::on_corner_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_CORNERKICK;
    ui->currentState->setText("OPP CORNER");
}

void Dialog::on_throwin_clicked()
{
    coach2robot_info_->MatchMode=OUR_THROWIN;
    ui->currentState->setText("OUR THROWIN");
}

void Dialog::on_throwin_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_THROWIN;
    ui->currentState->setText("OPP THROWIN");
}

void Dialog::on_freekick_clicked()
{
    coach2robot_info_->MatchMode=OUR_FREEKICK;
    ui->currentState->setText("OUR FREEKICK");
}

void Dialog::on_freekick_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_FREEKICK;
    ui->currentState->setText("OPP FREEKICK");
}

void Dialog::on_goalkick_clicked()
{
    coach2robot_info_->MatchMode=OUR_GOALKICK;
    ui->currentState->setText("OUR GOALKICK");
}

void Dialog::on_goalkick_opp_clicked()
{
    coach2robot_info_->MatchMode=OPP_GOALKICK;
    ui->currentState->setText("OPP GOALKICK");
}

void Dialog::on_dropball_clicked()
{
    coach2robot_info_->MatchMode=DROPBALL;
    ui->currentState->setText("DROPBALL");
}

void Dialog::on_cancel_clicked()
{
    ui->currentState->setText("CANCEL");
}

//障碍物显示控制
void Dialog::on_obstacles_clicked()
{
    if(!isObs_display_)
        isObs_display_=true;
    else if(isObs_display_)
        isObs_display_=false;
}

//队伍选择
void Dialog::on_cyan_clicked()
{
    teamflag_=1;
    ui->cyan->setStyleSheet("background-color:rgb(11, 246, 230);border:2px groove gray;border-radius:10px;padding:2px 4px;");
    ui->magenta->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px;");
}

void Dialog::on_magenta_clicked()
{
    teamflag_=0;
    ui->magenta->setStyleSheet("background-color:rgb(245, 12, 198);border:2px groove gray;border-radius:10px;padding:2px 4px;");
    ui->cyan->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px;");
}

//连接裁判盒
void Dialog::on_connectRefe_clicked()
{
    if(!isConnect_RefBox_)
    {
        QString IP="173.17.1.2";
        quint16 prot=28097;
        tcpSocket_->abort();
        tcpSocket_->connectToHost(IP,prot);

        connect(tcpSocket_, SIGNAL(readyRead()), this, SLOT(OnReceive_()));    //tcp/ip通信时用到的槽函数
        ui->connectRefe->setText("disconnect");
        isConnect_RefBox_=true;
        qDebug()<<"RefBox_Connected";
        disableButton_();
    }
    else
    {
        tcpSocket_->disconnectFromHost();
        disconnect(tcpSocket_, SIGNAL(readyRead()), this, SLOT(OnReceive_()));
        ui->connectRefe->setText("connect");
        isConnect_RefBox_=false;
        qDebug()<<"RefBox_Disconnected";
        enableButton_();
    }
}

void Dialog::on_upload_clicked()
{
    if(!isUpload_worldmodel_)
    {
        ui->upload->setText("STOPUP");
        isUpload_worldmodel_=true;
        qDebug()<<"Upload_worldmodel";
    }
    else
    {
        ui->upload->setText("UPLOAD");
        isUpload_worldmodel_=false;
        qDebug()<<"Stop_upload";
    }
}

void Dialog::on_change_ground_clicked()
{
    if(groundflag_==1)
    {
        ui->change_ground->setStyleSheet("border-image: url(../../../src/nubot/nubot_coach/source/right2left.png)");
        groundflag_=-1;
    }
    else if (groundflag_==-1)
    {
        ui->change_ground->setStyleSheet("border-image: url(../../../src/nubot/nubot_coach/source/left2right.png)");
        groundflag_=1;
    }
}

void Dialog::on_field_home_clicked()
{
    if(!isAtHome_)
    {
        field_->setPixmap(field_img_home_);     //改为奥拓楼球场
        isAtHome_=true;
    }
    else if (isAtHome_)
    {
        field_->setPixmap(field_img_init_);
        isAtHome_=false;
    }
}

//显示机器人策略信息
void Dialog::showRobot_info_()
{
    QString current_roles;
    QString current_actions;

    for(int i=0;i<OUR_TEAM;i++)
    {
        switch (robot2coach_info_->RobotInfo_[i].getCurrentRole())
        {
        case 0: current_roles="GOALIE";break;
        case 1: current_roles="ACTIVE";break;
        case 2: current_roles="PASSIVE";break;
        case 3: current_roles="MIDFIELD";break;
        case 4: current_roles="ASSISTANT";break;
        case 5: current_roles="ACIDPASSIVE";break;
        case 6: current_roles="GAZER";break;
        case 7: current_roles="BLOCK";break;
        case 8: current_roles="NOROLE";break;
        case 9: current_roles="CATCHOFPASS";break;
        case 10: current_roles="PASSOFPASS";break;
        default: break;
        }

        switch (robot2coach_info_->RobotInfo_[i].getCurrentAction())
        {
        case 0: current_actions="Stucked";break;
        case 1: current_actions="Penalty";break;
        case 2: current_actions="CanNotSeeBall";break;
        case 3: current_actions="SeeNotDribbleBall";break;
        case 4: current_actions="TurnForShoot";break;
        case 5: current_actions="AtShootSituation";break;
        case 6: current_actions="TurnToPass";break;
        case 7: current_actions="StaticPass";break;
        case 8: current_actions="AvoidObs";break;
        case 9: current_actions="Catch_Positioned";break;
        case 10: current_actions="Positioned";break;
        case 11: current_actions="Positioned_Static";break;
        case 12: current_actions="No_Action";break;
        default: break;
        }

        infoShow_[i]=QString("Role: %1, Action: %2, isDribble: %3").arg(current_roles).
                                                                    arg(current_actions).
                                                                    arg(robot2coach_info_->RobotInfo_[i].getDribbleState());
    }
    ui->infoShow_1->setText(infoShow_[0]);
    ui->infoShow_2->setText(infoShow_[1]);
    ui->infoShow_3->setText(infoShow_[2]);
    ui->infoShow_4->setText(infoShow_[3]);
    ui->infoShow_5->setText(infoShow_[4]);
}

//用于显示所有机器人和球的位置和速度信息
void Dialog::showAll_info_()
{
    for(int i=0;i<OUR_TEAM;i++)
    {
        if(robot2coach_info_->RobotInfo_[i].isValid())
            allShow_[i]=QString("NUM.%1: ROBOT_POS: (%2, %3); ROBOT_ORI: %4; BALL_GLO: (%5, %6); BALL_VEC: (%7, %8)").arg(i+1).
                    arg(robot2coach_info_->RobotInfo_[i].getLocation().x_,0,'g',5).arg(robot2coach_info_->RobotInfo_[i].getLocation().y_,0,'g',5).
                    arg(robot2coach_info_->RobotInfo_[i].getHead().degree(),0,'g',3).arg(robot2coach_info_->BallInfo_[i].getGlobalLocation().x_,0,'g',5).
                    arg(robot2coach_info_->BallInfo_[i].getGlobalLocation().y_,0,'g',5).arg(robot2coach_info_->BallInfo_[i].getVelocity().x_,0,'g',5).
                    arg(robot2coach_info_->BallInfo_[i].getVelocity().y_,0,'g',5);
        else
            allShow_[i]=QString("NUM.%1: ROBOT_POS: (0, 0); ROBOT_ORI: 0; BALL_GLO: (0, 0); BALL_VEC: (0, 0)").arg(i+1);
    }
    allShow_combine_=allShow_[0]+"\n"+allShow_[1]+"\n"+allShow_[2]+"\n"+allShow_[3]+"\n"+allShow_[4];
    ui->all_show->setText(allShow_combine_);
}

//在多个机器人都看到球的情况下做融合，仅用于绘图
int Dialog::ballPos_fuse()
{
    QVector<nubot::DPoint> ball_pos;
    QVector<nubot::DPoint> robot_pos;
    QVector<int> robot_num;
    float robot2ball_dis=20000;                        //机器人都看到球的最短距离
    int num=0;

    for(int i=0;i<OUR_TEAM;i++)                                                   //如果机器人看到了球就将其压到堆栈中
        if(robot2coach_info_->BallInfo_[i].isLocationKnown()&&robot2coach_info_->RobotInfo_[i].isValid())
        {
            ball_pos.push_back(robot2coach_info_->BallInfo_[i].getGlobalLocation());
            robot_pos.push_back(robot2coach_info_->RobotInfo_[i].getLocation());
            robot_num.push_back(i);
        }
    if(ball_pos.size())
    {
        for(int i=0;i<ball_pos.size();i++)
            if(ball_pos[i].distance(robot_pos[i])<robot2ball_dis)
            {
                robot2ball_dis=ball_pos[i].distance(robot_pos[i]);
                num=robot_num[i];
            }

        return (num+1);
    }
    else
        return num;

}

//根据xml解析结果发布coach指令
void Dialog::RefBox_Info()
{
    char chafinal=json_parse_->chafinal_;
   // qDebug()<<chafinal;
   // score_cyan_=json_parse_->score_cyan_;
   // score_magenta_=json_parse_->score_magenta_;
    switch (chafinal)
    {
    case COMM_START:
       // coach2robot_info_->MatchType=coach2robot_info_->MatchMode;
        coach2robot_info_->MatchMode=STARTROBOT;
        ui->currentState->setText("START ROBOT");
        break;
    case COMM_FIRST_HALF:
//        coach2robot_info_->MatchType=coach2robot_info_->MatchMode;
        coach2robot_info_->MatchMode=STARTROBOT;
        ui->currentState->setText("FIRST HALF");
        break;
    case COMM_SECOND_HALF:
      //  coach2robot_info_->MatchType=coach2robot_info_->MatchMode;
        coach2robot_info_->MatchMode=STARTROBOT;
        ui->currentState->setText("SECOND HALF");
        break;
     case COMM_STOP:
        coach2robot_info_->MatchMode=STOPROBOT;
        ui->currentState->setText("STOP ROBOT");
        break;
     case COMM_HALT:
        coach2robot_info_->MatchMode=STOPROBOT;
        ui->currentState->setText("Halt");
        break;
     case COMM_READY:
        ui->currentState->setText("READY");
        break;
     case COMM_DROPPED_BALL:
        coach2robot_info_->MatchType=DROPBALL;
        coach2robot_info_->MatchMode=DROPBALL;
        ui->currentState->setText("DROPBALL");
        break;
     case COMM_GOAL_MAGENTA:
        score_magenta_++;
        //coach2robot_info_->MatchMode=STOPROBOT;
        ui->Score0->display(score_magenta_);
        break;
     case COMM_GOAL_CYAN:
        score_cyan_++;
        //coach2robot_info_->MatchMode=STOPROBOT;
        ui->Score1->display(score_cyan_);
        break;
     case COMM_SUBGOAL_MAGENTA:
        score_magenta_--;
        //coach2robot_info_->MatchMode=STOPROBOT;
        ui->Score0->display(score_magenta_);
        break;
     case COMM_SUBGOAL_CYAN:
        score_cyan_--;
        //coach2robot_info_->MatchMode=STOPROBOT;
        ui->Score1->display(score_cyan_);
        break;
     case COMM_KICKOFF_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OPP_KICKOFF;
            coach2robot_info_->MatchMode=OPP_KICKOFF;
            ui->currentState->setText("OPP KICKOFF");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_KICKOFF;
            coach2robot_info_->MatchMode=OUR_KICKOFF;
            ui->currentState->setText("OUR KICKOFF");
        }
        break;
     case COMM_KICKOFF_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_KICKOFF;
            coach2robot_info_->MatchMode=OUR_KICKOFF;
            ui->currentState->setText("OUR KICKOFF");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_KICKOFF;
            coach2robot_info_->MatchMode=OPP_KICKOFF;
            ui->currentState->setText("OPP KICKOFF");
        }
        break;
     case COMM_FREEKICK_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OPP_FREEKICK;
            coach2robot_info_->MatchMode=OPP_FREEKICK;
            ui->currentState->setText("OPP FREEKICK");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_FREEKICK;
            coach2robot_info_->MatchMode=OUR_FREEKICK;
            ui->currentState->setText("OUR FREEKICK");
        }
        break;
    case COMM_FREEKICK_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_FREEKICK;
            coach2robot_info_->MatchMode=OUR_FREEKICK;
            ui->currentState->setText("OUR FREEKICK");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_FREEKICK;
            coach2robot_info_->MatchMode=OPP_FREEKICK;
            ui->currentState->setText("OPP FREEKICK");
        }
        break;
    case COMM_GOALKICK_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType = OPP_GOALKICK;
            coach2robot_info_->MatchMode = OPP_GOALKICK;
            ui->currentState->setText("OPP GOALKICK");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_GOALKICK;
            coach2robot_info_->MatchMode=OUR_GOALKICK;
            ui->currentState->setText("OUR GOALKICK");
        }
        break;
    case COMM_GOALKICK_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_GOALKICK;
            coach2robot_info_->MatchMode=OUR_GOALKICK;
            ui->currentState->setText("OUR GOALKICK");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_GOALKICK;
            coach2robot_info_->MatchMode=OPP_GOALKICK;
            ui->currentState->setText("OPP GOALKICK");
        }
        break;
    case COMM_THROWIN_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OPP_THROWIN;
            coach2robot_info_->MatchMode=OPP_THROWIN;
            ui->currentState->setText("OPP THROWIN");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_THROWIN;
            coach2robot_info_->MatchMode=OUR_THROWIN;
            ui->currentState->setText("OUR THROWIN");
        }
        break;
    case COMM_THROWIN_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_THROWIN;
            coach2robot_info_->MatchMode=OUR_THROWIN;
            ui->currentState->setText("OUR THROWIN");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_THROWIN;
            coach2robot_info_->MatchMode=OPP_THROWIN;
            ui->currentState->setText("OPP THROWIN");
        }
        break;
    case COMM_CORNER_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OPP_CORNERKICK;
            coach2robot_info_->MatchMode=OPP_CORNERKICK;
            ui->currentState->setText("OPP CORNER");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_CORNERKICK;
            coach2robot_info_->MatchMode=OUR_CORNERKICK;
            ui->currentState->setText("OUR CORNER");
        }
        break;
    case COMM_CORNER_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_CORNERKICK;
            coach2robot_info_->MatchMode=OUR_CORNERKICK;
            ui->currentState->setText("OUR CORNER");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_CORNERKICK;
            coach2robot_info_->MatchMode=OPP_CORNERKICK;
            ui->currentState->setText("OPP CORNER");
        }
        break;
    case COMM_PENALTY_MAGENTA:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OPP_PENALTY;
            coach2robot_info_->MatchMode=OPP_PENALTY;
            ui->currentState->setText("OPP PENALTY");
        }
        else
        {
            coach2robot_info_->MatchType=OUR_PENALTY;
            coach2robot_info_->MatchMode=OUR_PENALTY;
            ui->currentState->setText("OUR PENALTY");
        }
        break;
    case COMM_PENALTY_CYAN:
        if(teamflag_)
        {
            coach2robot_info_->MatchType=OUR_PENALTY;
            coach2robot_info_->MatchMode=OUR_PENALTY;
            ui->currentState->setText("OUR PENALTY");
        }
        else
        {
            coach2robot_info_->MatchType=OPP_PENALTY;
            coach2robot_info_->MatchMode=OPP_PENALTY;
            ui->currentState->setText("OPP PENALTY");
        }
        break;
    case COMM_CANCEL:
        ui->currentState->setText("CANCEL");
        break;
    case COMM_PARKING:
        coach2robot_info_->MatchMode=PARKINGROBOT;
        ui->currentState->setText("PARKING");
    case COMM_HALF_TIME:
        coach2robot_info_->MatchMode=STOPROBOT;
        ui->currentState->setText("END_PART");
        break;
    case COMM_END_GAME:
        ui->currentState->setText("GOOD_GAME");
        break;
    default:
        break;
    }
   // ui->Score0->display(score_magenta_);
   // ui->Score1->display(score_cyan_);
}

//连接裁判盒后，coach不能主动发布命令
void Dialog::disableButton_()
{
    ui->kickoff_opp->setEnabled(false);
    ui->penalty_opp->setEnabled(false);
    ui->corner_opp->setEnabled(false);
    ui->throwin_opp->setEnabled(false);
    ui->freekick_opp->setEnabled(false);
    ui->goalkick_opp->setEnabled(false);
    ui->cancel->setEnabled(false);
    ui->kickoff->setEnabled(false);
    ui->penalty->setEnabled(false);
    ui->corner->setEnabled(false);
    ui->throwin->setEnabled(false);
    ui->freekick->setEnabled(false);
    ui->goalkick->setEnabled(false);
    ui->dropball->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
}

void Dialog::enableButton_()
{
    ui->kickoff_opp->setEnabled(true);
    ui->penalty_opp->setEnabled(true);
    ui->corner_opp->setEnabled(true);
    ui->throwin_opp->setEnabled(true);
    ui->freekick_opp->setEnabled(true);
    ui->goalkick_opp->setEnabled(true);
    ui->cancel->setEnabled(true);
    ui->kickoff->setEnabled(true);
    ui->penalty->setEnabled(true);
    ui->corner->setEnabled(true);
    ui->throwin->setEnabled(true);
    ui->freekick->setEnabled(true);
    ui->goalkick->setEnabled(true);
    ui->dropball->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
}

//接收jason的响应函数
void Dialog::OnReceive_()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);

    while (socket->bytesAvailable() > 0)
    {
        buffer->clear();
        buffer->append(socket->readAll());
        for(int i = 0 ; i < buffer->size() ;i++)
        {
            json_parse_->chafinal_ = buffer->data()[i];
            RefBox_Info();
            ui->text_disply->setStyleSheet("color:rgb(255,0,0);");
            qDebug()<< json_parse_->chafinal_<<buffer->size();
        }
     /*   if(1)//buffer->contains('\0'))
        {
            qint32 size = buffer->indexOf('\0');
            QByteArray data = buffer->mid(0, size);
            qDebug()<<data;

            buffer->remove(0, size + 1);
            if(json_parse_->parseJSON_(data))
            {
                RefBox_Info();
                ui->text_disply->setStyleSheet("color:rgb(255,0,0);");
                qDebug()<<data;
            }
        }*/
    }
}

//返回链接错误
void Dialog::displayError_(QAbstractSocket::SocketError)
{
    qDebug()<<tcpSocket_->errorString();
    tcpSocket_->close();
}

//延时函数，目前没有用
void Dialog::buttonDelay_()              //每隔450ms按钮复位
{
    static int count=0;
    if(count==100)
    {
        ui->startButton->setDown(false);
        count==0;
    }
    else
        count++;
}

void Dialog::restItems_()
{
    ball_->setPos(900,900);
    for(int i=0;i<OUR_TEAM;i++)
        robot_[i]->setPos(900,900);
    for(int i=0;i<MAX_OBSNUMBER_CONST*2;i++)
        obstacle_[i]->setPos(900,900);                            //初始位置放到(900,900),不出现在视野里
    velocity_->setLine(900,901,900,901);
}
