#! /usr/bin/make

survey-processed.csv: survey-split.txt
	echo Option Name,Commonly Uses It,Ever Used It,Knew of It > $@
	while read NAME LINE; do echo $$NAME,$$(echo "$$LINE" | tr ' ' '\n' | grep -c '^1'),$$(echo "$$LINE" | tr ' ' '\n' | grep -c '^,1'),$$(echo "$$LINE" | tr ' ' '\n' | grep -c '^,,1'); done < $< >> $@

survey-split.txt: survey.csv
	sed 's/,/ /' < $< | sed 's/\(1\?,1\?,1\?,\)/\1 /g' | tail -n +3 > $@

clean:
	rm -f survey-split.txt survey-processed.csv
