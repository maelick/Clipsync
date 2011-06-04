# Options
LATEX=latex
BIBTEX=bibtex
DVIPS=dvips
PS2PDF=ps2pdf
CONVERT=convert

# Executables
EXEC=report.pdf

# Rules
all: $(EXEC)

# Dependencies
report.dvi: fig_p2p.eps umons.eps connections.eps peerhandler.eps broadcast.eps

# Rules (cont'd)
%.pdf: %.ps
	$(PS2PDF) $<

%.ps: %.dvi
	$(DVIPS) $<

%.dvi: %.tex
	$(LATEX) $<
	$(BIBTEX) $*.aux
	$(LATEX) $<
	$(LATEX) $<

%.eps: img/%.png
	$(CONVERT) $< $@

.PHONY: clean

clean:
	@rm *.dvi *.eps *.log *.toc *.aux *.bbl *.blg *-blx.bib
