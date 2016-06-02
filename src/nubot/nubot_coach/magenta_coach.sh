#!/bin/bash			

### source the workspace
source ../devel/setup.bash
source devel/setup.bash

### Get parameters and init
declare -i kill_num
magenta_prefix=$(rosparam get /magenta/prefix)

### run
#rosrun world_model world_model_node& & __name:=$(magenta_prefix)_wm &
#let "kill_num=kill_num+1"

rosrun nubot_coach nubot_coach_node ${magenta_prefix} __name:=${magenta_prefix}_coach &
let "kill_num=kill_num+1"

### kill thoes background processes
trap 'kill ${PIDS[*]}' SIGINT
wait

