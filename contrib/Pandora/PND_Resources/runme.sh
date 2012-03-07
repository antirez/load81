#!/bin/bash
if [ ! -f "./profile.txt" ]; then
    cp -R defconf/* ./
fi
if [ $# -eq 0 ]
then
   ./picklelauncher
fi
