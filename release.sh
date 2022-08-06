#!/bin/bash
rm -rf release
mkdir -p release

cp -rf StructureSynth *.{hpp,cpp,txt,json} LICENSE release/

mv release score-addon-structuresynth
7z a score-addon-structuresynth.zip score-addon-structuresynth
