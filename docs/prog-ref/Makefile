ref.pdf: ref.tex
	pdflatex ref.tex
	pdflatex ref.tex

html: ref/index.html

ref/index.html: ref.tex
	latex2html -split 2 ref.tex

.PHONY: clean

clean::
	-rm -f ref.pdf


