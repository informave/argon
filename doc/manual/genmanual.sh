#!/bin/sh



MANUALDIR=$1/manual

#awk '{ print "//APIREF.050 "$0; }' ${GUIDEDIR}/guide.txt > ./apiref.txx;
#COM='//' ../../pipadoc  `find ../../ -type f -regex ".*\(hh\|cc\)"` ${GUIDEDIR}/*.txt  ./*.txx ${GUIDEDIR}/guide.conf > guide.txt
#a2x -v -f xhtml guide.txt
ln -s ${MANUALDIR}/manual.txt manual.txt;
asciidoc -v --doctype book -b xhtml11 -a toc -a toclevels=3 -a numbered manual.txt;
#asciidoc -v --doctype book -b docbook -a toc -a toclevels=3 -a numbered guide.txt;

