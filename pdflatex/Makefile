all: ims.pdf

ims.pdf: ims.tex
	pdflatex ims.tex
	bibtex ims.aux
	pdflatex ims.tex
	pdflatex ims.tex

clean:
	rm ims.bbl ims.log ims.aux ims.blg ims.pdf ims.toc
