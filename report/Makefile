# Options
LATEX=latex
BIBTEX=bibtex
DVIPS=dvips
PS2PDF=ps2pdf
CONVERT=convert

# Executables
EXEC=report.pdf presentation.pdf

# Rules
all: $(EXEC)

# Dependencies
report.dvi: fig_p2p.eps umons.eps connections.eps peerhandler.eps broadcast.eps
	$(LATEX) report.tex
	$(BIBTEX) report.aux
presentation.dvi: ccmp1.eps ccmp2.eps cl1p.eps clipboardmshare.eps netclip1.eps netclip2.eps fig_p2p.eps remoteclip.eps umons.eps

# Rules (cont'd)
%.pdf: %.ps
	$(PS2PDF) $<

%.ps: %.dvi
	$(DVIPS) $<

%.dvi: %.tex
	$(LATEX) $<
	$(LATEX) $<

%.eps: img/%.png
	$(CONVERT) $< $@

.PHONY: clean

clean:
	@rm *.dvi *.eps *.log *.toc *.aux *.bbl *.blg *-blx.bib *.nav *.snm *.out
