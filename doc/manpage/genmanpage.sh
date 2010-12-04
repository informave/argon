#!/bin/sh



MANUALDIR=$1/manpage

COM='//' ../../pipadoc  `find ../../ -type f -regex ".*\(hh\|cc\)"` ${MANUALDIR}/*.txt ${MANUALDIR}/manpage.conf > argoncli.1.txt
a2x -v -f manpage argoncli.1.txt

