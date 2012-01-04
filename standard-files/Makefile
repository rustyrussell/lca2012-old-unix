# Makefile to wrap simple binary tests as noted in standard-tests.txt
#
# Targets:	pdp11	-- Run pdp11 tests through apout
#		x88 	-- Run x86 tests through qemu-i386
#
# Variables:  	APOUT=<dir containing apout & anbin>
#		ROOT=<unix root dir>
#		or
#		QEMU=<dir containing qemu-i386 and analyse>
#
# If APOUT variable not set, assumed to be using qemu-i386
#
#
# Example:
# make pdp11 APOUT=/home/matt/Documents/LCA/2012/binsize/pdp11/apout2.3beta1 ROOT=/home/matt/Documents/LCA/2012/binsize/pdp11/TUHS/PDP-11/Trees/V6/
#
# This script does not remove existing /tmp/binuse (or /tmp/binsize, for qemu)
# data; ensure these dirs are clean first.
#

ifneq ($(APOUT), )
# We're running PDP/11 stuff
INTERP=$(APOUT)/apout
APOUT_ROOT=$(ROOT)
IS_V6=$(shell echo $(ROOT) | grep -c V6 )
ifeq ($(IS_V6), '1')
	GREPPATH=/usr/bin
else
	GREPPATH=/bin
endif
export APOUT_ROOT
else
# We're running Linux x86 stuff
INTERP=$(QEMU)/qemu-i386
GREPPATH=/bin
endif

# The return value from apout seems to be random... :P  hence the || true.

help:
	@echo " Run this with a target 'pdp11' or 'x86' and supply appropriate vars,"
	@echo " APOUT & ROOT (for pdp11) or QEMU (for x86)."

basetest:	ls cat grep

ls:
	$(INTERP) $(ROOT)/bin/ls -ld	|| true
	$(INTERP) $(ROOT)/bin/ls -asg dir2 m*    || true
	$(INTERP) $(ROOT)/bin/ls -lur . nonexistentfile    || true
	$(INTERP) $(ROOT)/bin/ls -tui    || true
	$(INTERP) $(ROOT)/bin/ls -f    || true

ls-gnu-extra:
	$(INTERP) $(ROOT)/bin/ls -oAhFSL --color --full-time    || true
	$(INTERP) $(ROOT)/bin/ls -1R /dev    || true
	$(INTERP) $(ROOT)/bin/ls -l --time-style=full-iso    || true
	$(INTERP) $(ROOT)/bin/ls -l --time-style=long-iso    || true
	$(INTERP) $(ROOT)/bin/ls -l --time-style=iso    || true
	$(INTERP) $(ROOT)/bin/ls -l --time-style=locale    || true
	$(INTERP) $(ROOT)/bin/ls -l --time-style=='+%s'    || true
	$(INTERP) $(ROOT)/bin/ls --help    || true
	$(INTERP) $(ROOT)/bin/ls --version    || true
	$(INTERP) $(ROOT)/bin/ls -ltcnkD    || true
	$(INTERP) $(ROOT)/bin/ls -C    || true
	$(INTERP) $(ROOT)/bin/ls -lG --group-directories-first --time=ctime    || true
	$(INTERP) $(ROOT)/bin/ls -H my_filelink    || true
	$(INTERP) $(ROOT)/bin/ls -mp --sort=time -w 40    || true
	$(INTERP) $(ROOT)/bin/ls -o --show-control-chars    || true
	$(INTERP) $(ROOT)/bin/ls --format=horizontal    || true
	$(INTERP) $(ROOT)/bin/ls -lh --si --quoting-style=c    || true
	$(INTERP) $(ROOT)/bin/ls -l --block-size 256 --quoting-style=shell-always    || true

cat:
	$(INTERP) $(ROOT)/bin/cat mbox    || true
	$(INTERP) $(ROOT)/bin/cat nonexistentfile    || true
	$(INTERP) $(ROOT)/bin/cat mbox make_devices.sh    || true
	echo | $(INTERP) $(ROOT)/bin/cat mbox make_devices.sh -    || true
	$(INTERP) $(ROOT)/bin/cat make_devices.sh - <&-    || true

grep:
	$(INTERP) $(ROOT)$(GREPPATH)/grep foo nonexistentfile  || true
	$(INTERP) $(ROOT)$(GREPPATH)/grep -vn nod make_devices.sh  || true
	$(INTERP) $(ROOT)$(GREPPATH)/grep -c nod make_devices.sh  || true
	$(INTERP) $(ROOT)$(GREPPATH)/grep "thing.*thing[^f][abc]*$$" mbox  || true
	$(INTERP) $(ROOT)$(GREPPATH)/grep "affinity$$" mbox  || true
	$(INTERP) $(ROOT)$(GREPPATH)/grep "spa.*din[^z][a-z, ]*" mbox  || true

pdp11:	tmpdirs_pdp11 basetest
	@echo "\n\nAfter running ls, cat and grep with various options:\n"
	@echo -n " ls: "
	@$(APOUT)/anbin /tmp/binuse/ls.*
	@echo -n " cat: "
	@$(APOUT)/anbin /tmp/binuse/cat.*
	@echo -n " grep: "
	@$(APOUT)/anbin /tmp/binuse/grep.*

x86-analyse:
	@echo "\n\nAfter running with various options:\n"
	./analyse_qemu_output.sh $(QEMU)/analyse /tmp/binsize

x86:	tmpdirs_qemu basetest x86-analyse

x86-extra:	tmpdirs_qemu ls-gnu-extra x86-analyse


tmpdirs_pdp11:
	mkdir -p /tmp/binuse

tmpdirs_qemu:
	mkdir -p /tmp/binsize
