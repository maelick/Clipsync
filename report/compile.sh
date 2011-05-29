convert img/fig_p2p.png fig_p2p.eps
convert img/umons.png umons.eps
convert img/broadcast.png broadcast.eps
convert img/connections.png connections.eps
convert img/peerhandler.png peerhandler.eps
latex report.tex
bibtex report.aux
latex report.tex
latex report.tex
dvips report.dvi
ps2pdf report.ps
