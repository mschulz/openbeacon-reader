# Makefile
#
# Makefile for the Arduino NRF24 project
#
# Author: Mike McCauley (mikem@open.com.au)
# Copyright (C) 2011 Mike McCauley
# $Id: Makefile,v 1.1 2012/12/12 11:17:16 mikem Exp mikem $

PROJNAME = NRF24
# Dont forget to also change the version at the top of NRF24.h:
DISTFILE = $(PROJNAME)-1.7.zip

all:	doxygen dist upload

doxygen: 
	doxygen project.cfg

ci:
	(cd ..;ci -l `cat $(PROJNAME)/MANIFEST`)

dist:	
	(cd ..; zip $(PROJNAME)/$(DISTFILE) `cat $(PROJNAME)/MANIFEST`)

upload:
	rsync -avz $(DISTFILE) doc/ server2:/var/www/html/mikem/arduino/$(PROJNAME)
#	scp $(DISTFILE) doc/*.html doc/*.gif doc/*.png doc/*.css doc/*.pdf server2:/var/www/html/mikem/arduino/$(PROJNAME)
