#!/bin/bash

tmux new-session -s my_session
tmux split-window -h
tmux select-pane -t 0
tmux split-window -v
tmux select-pane -t 1
tmux split-window -v
tmux select-pane -t 0
tmux split-window -v

tmux send-keys -t 4 './GS list.txt -p 6969 -v' 'Enter'

for i in {0..3}
do
    tmux send-keys -t $i './player -n localhost -p 6969' 'Enter'
done


