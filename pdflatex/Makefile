all: ims.pdf

ims.pdf: ims.tex
	pdflatex ims.tex
	bibtex ims.aux
	pdflatex ims.tex
	pdflatex ims.tex

