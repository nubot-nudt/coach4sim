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
};
}

#endif // COACH2REFBOX_H
