@echo off
awk '{print FNR "\t" $0}' %1
