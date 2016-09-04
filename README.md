#Note:
1.请将coach_ws文件夹放到home目录下；   
2.作为coach其环境变量中的AGENT=0,需要修改brashrc,重启生效,或者每次在终端中输入export AGENT=0;UPATE：现在将AGENT环境变量的设置放在了roslaunch文件中，所以现在不需要手动设置AGENT了。   
3.新电脑第一次用裁判盒时，需要修改一个权限：   
```
﻿sudo gedit /etc/security/limits.conf   
  *               -       rtprio          100   
```
4.结束整个coach时，需要关闭ui和ctrl+c；   
5.要修改RTBD的comm中的argv[1]参数，无线就是wlan0，有线就是eth0，当然如果插外置网卡再改；   
6.当连接裁判盒时，所有按钮（空格可以STOP）都disabled；   
7.为了减少不必要的数据传输负担，对RTDB的comm做了小的修改，只接收和发送必要的信息；   
8.编译时如果提示找不到Eigen，把worldmodel的CMakeList中的两处Eigen3改成Eigen（因为最新的ros没有Eigen）；   

# RUN
1. run coach for real robots   
` $ roslaunch nubot_coach nubot_coach.launch` 
2. run coach for simulation   
	(1) coach for cyan: 	` $ roslaunch nubot_coach nubot_coach_cyan.launch `   
	(2) coach for magenta: 	` $ roslaunch nubot_coach nubot_coach_magenta.launch `   

# Build Error Fix:
1. When catkin_make, if it shows "fatal error: Eigen/Eigen: No such file or directory"   
**Solution 1**: look at /usr/include/eigen3/Eigen, if this folder exists, it means you have already installed Eigen; Input this command:    
` $ sudo ln -s /usr/include/eigen3/Eigen /usr/include/Eigen   `   
**Solution 2**: 把worldmodel的CMakeList中的两处Eigen3改成Eigen（因为最新的ros没有Eigen）；   
2. if you only changed the configuration parts of some files mention above and git should not record these changes, then you should run 'configure' file like this:   
` $ sudo chmod +x configure`   
` $ ./configure `   
   
   
**(You could ignore the following part!)**
> **NOTE:** if used with RTDB, then you should change absolute path in rtdb files:

> 1. rtdb/parser/CMakeLists.txt:

>    (1) SET(RTDB_USER_H_FILE   /home/nubot8/nubot_ws/src/nubot/world_model/include/nubot/rtdb/rtdb_user.h)

>    (2) SET(RTDB_INI_FILE      /home/nubot8/nubot_ws/src/nubot/world_model/config/rtdb.ini )

>    (3) SET(RTDB_Config_FILE   /home/nubot8/nubot_ws/src/nubot/world_model/config/rtdb.config )

> 2. rtdb/rtdb_api.cpp:    
>    std::string ini_config_file="/home/nubot8/nubot_ws/src/nubot/world_model/config/rtdb.ini";
> 3. rtdb/comm.cpp: 'wlano' or 'wlan1'
> 4. coach_info_pub.cpp: DB_get(AGENT), this AGENT depends your coach's agent number
