TEXI := $(wildcard *.texi)

all: $(TEXI)
	make info
	make html
	make pdf

info: xforms.info

xforms.info: $(TEXI)
	makeinfo xforms.texi

html: xforms.html

xforms.html: $(TEXI) xforms.init
	texi2html --init-file xforms.init xforms.texi
	if [ ! -e xforms ]; then  \
		mkdir xforms;         \
	fi
	ln -s xforms.html index.html
	mv *.html xforms
	cp -r images xforms

pdf: xforms.pdf

xforms.pdf: $(TEXI)
	texi2pdf -c xforms.texi

clean:
	rm -rf *.html *.pdf *.log *~ xforms xforms.info* *.dvi
