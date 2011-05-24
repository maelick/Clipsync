convert img/fig_clientloc.png fig_clientloc.eps
convert img/fig_copypaste.png fig_copypaste.eps
convert img/fig_join.png fig_join.eps
convert img/fig_local.png fig_local.eps
convert img/fig_p2p.png fig_p2p.eps
convert img/umons.png umons.eps
convert img/p2pclient_broadcast.png p2pclient_broadcast.eps
convert img/p2pclient_connections.png p2pclient_connections.eps
convert img/p2pclient_peerhandler1.png p2pclient_peerhandler1.eps
convert img/p2pclient_peerhandler2.png p2pclient_peerhandler2.eps
latex report.tex
#bibtex report.aux
#latex report.tex
latex report.tex
dvips report.dvi
ps2pdf report.ps
