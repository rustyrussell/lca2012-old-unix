#! /usr/bin/make
OBJDUMP=../matts-binutils/bin/pdp11-aout-objdump

annotation:
	@echo Extra text for V6 cat:
	@$(OBJDUMP) -d ../cat/unix-cat | ./annotate-asm.sh | tail -n1
	@echo Extra text for V6 cat '(C equivalent):'
	@$(OBJDUMP) -d ../cat/unix-cat-rewritten-in-c | ./annotate-asm.sh | tail -n1
	@echo Extra text for V6 grep:
	@$(OBJDUMP) -d ../grep/unix-grep | ./annotate-asm.sh | tail -n1
	@echo Extra text for V6 ls:
	@$(OBJDUMP) -d ../ls/unix-ls | ./annotate-asm.sh | tail -n1
