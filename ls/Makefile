CFLAGS=-Os
LDFLAGS=-s

ls-ported-forward: ls-ported-forward.c
clean:
	rm -f ls-ported-forward

speed-test: testdir ls-ported-forward
	@echo "GNU ls:"
	@cd testdir; unset LS_COLORS; echo `for i in \`seq 3\`; do /usr/bin/time -f %e /bin/ls 2>&1 > /dev/null; done`
	@echo "GNU ls -l:"
	@cd testdir; unset LS_COLORS; echo `for i in \`seq 3\`; do /usr/bin/time -f %e /bin/ls -l 2>&1 > /dev/null; done`
	@echo "GNU ls (LANG=C):"
	@cd testdir; unset LS_COLORS; echo `for i in \`seq 3\`; do LANG=C /usr/bin/time -f %e /bin/ls 2>&1 > /dev/null; done`
	@echo "UNIX ls:"
	@cd testdir; echo `for i in \`seq 3\`; do /usr/bin/time -f %e ../ls-ported-forward 2>&1 > /dev/null; done`
	@echo "UNIX ls -l:"
	@cd testdir; echo `for i in \`seq 3\`; do /usr/bin/time -f %e ../ls-ported-forward -l 2>&1 > /dev/null; done`

testdir:
	mkdir testdir && cd testdir && seq 500000 | xargs touch
