#! /bin/sh

# Annotate an assembler file with nature of instructions.

# Any register offset => assume 24 bit relative (+3 bytes)
# Any branch => Assume 48 bit offset address (+6 bytes)
# Any literal => Assume 32 bit value (+4 bytes)

ADDITIONAL=0
while read LINE; do
    case "$LINE" in
	*[0-9]\(*)
	    echo "$LINE		# 3"
	    ADDITIONAL=$(($ADDITIONAL + 3))
	    ;;
	*br*|*bhi*|*beq*|*bne*|*bge*|*jsr*|*jmp*)
	    echo "$LINE		# 6"
	    ADDITIONAL=$(($ADDITIONAL + 6))
	    ;;
	*\$*)
	    echo "$LINE		# 4"
	    ADDITIONAL=$(($ADDITIONAL + 4))
	    ;;
	*)
	    echo "$LINE"
	    ;;
    esac
done
echo Total bytes added: $ADDITIONAL
