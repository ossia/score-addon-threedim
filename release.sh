#!/bin/bash
rm -rf release
mkdir -p release

cp -rf Threedim 3rdparty *.{hpp,cpp,txt,json} LICENSE release/

mv release score-addon-threedim
7z a score-addon-threedim.zip score-addon-threedim
