%: %.xml
	@rm -f -- "$@"
	xsltproc -o "$@" -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<

%.xml: %.asc
	@rm -f -- "$@"
	asciidoc -d manpage -b docbook -o "$@" $<

re.8: re.8.asc
rerc.5: rerc.5.asc

man: re.8 rerc.5
install: man
	install -D -m 755 re $(DESTDIR)/usr/bin/re
	install -D -m 644 rerc.template $(DESTDIR)/etc/rerc
	install -D -m 644 re.8  $(DESTDIR)/usr/share/man/man8/re.8 
	install -D -m 644 rerc.5  $(DESTDIR)/usr/share/man/man5/rerc.5
        
