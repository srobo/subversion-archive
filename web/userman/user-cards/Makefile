users.pdf: users.tex
	pdflatex users.tex

users.tex: make-cards.py
	./make-cards.py ../users-assigned.tex > users.tex

