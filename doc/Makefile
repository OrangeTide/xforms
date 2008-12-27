TEXI := $(wildcard *.texi)

all:
	make info
	make html
	make pdf

info: $(TEXI)
	makeinfo xforms.texi

html: $(TEXI) xforms.init
	cp images/*.png .
	texi2html --init-file xforms.init xforms.texi
	if [ ! -e xforms ]; then  \
		mkdir xforms;         \
	fi
	ln -s xforms.html index.html
	mv *.html xforms
	mv *.png xforms

pdf: $(TEXI)
	texi2pdf -c xforms.texi

clean:
	rm -rf *.html *.pdf *.log *~ xforms xforms.info* *.dvi
