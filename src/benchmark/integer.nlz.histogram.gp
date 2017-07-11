#!/usr/bin/gnuplot
# -*- mode: sh -*-

outdir = 'out/integer.nlz'

# plot \
#   outdir."/hist.base.txt" w l, \
#   outdir."/hist.shift.txt" w l, \
#   outdir."/hist.shift4.txt" w l, \
#   outdir."/hist.shift8.txt" w l, \
#   outdir."/hist.bsec.txt" w l, \
#   outdir."/hist.bsec2.txt" w l, \
#   outdir."/hist.bsec3.txt" w l, \
#   outdir."/hist.builtin.txt" w l, \
#   outdir."/hist.bctz.txt" w l, \
#   outdir."/hist.asmbsr.txt" w l, \
#   outdir."/hist.frexp.txt" w l, \
#   outdir."/hist.double.txt" w l, \
#   outdir."/hist.float.txt" w l, \
#   outdir."/hist.kazatsuyu.txt" w l, \
#   outdir."/hist.debruijn.txt" w l, \
#   outdir."/hist.debruijn2.txt" w l

set terminal postscript eps color size 4,4
#set size 1.0,2.0/(1+sqrt(5))
set size 1.0,1.0/sqrt(2)
set output outdir.'/benchmark.eps'

set xtics rotate by -90 offset first 0.0,0.0
set ylabe '(t - t_{base})/ (t_{shift1} - t_{base})'
#unset log y; set yrange [0.0:5.0]
set log y; set yrange [0.01:5.0]
set key samplen 1 font ",10"
set xrange [-0.5:21.5]
plot \
  '< printf "%f %f\n" 11.5 0.01 11.5 10.0' with filledcurves y1 lc rgb '#dddddd' title '', \
  '< printf "%f %f\n"  8.5 0.01  8.5 10.0' with filledcurves y1 lc rgb '#ffffff' title '', \
  '< printf "%f %f\n"  5.5 0.01  5.5 10.0' with filledcurves y1 lc rgb '#dddddd' title '', \
  '< printf "%f %f\n"  2.5 0.01  2.5 10.0' with filledcurves y1 lc rgb '#ffffff' title '', \
  outdir.'/graph.xtic.txt'           using ($0+0.00):(-1):xtic(1) notitle, \
  outdir.'/graph.pad.icc.txt'        using ($0-0.25):2 ps 0.8 pt 8 lc rgb '#0000FF' title '', \
  outdir.'/graph.pad.gcc.txt'        using ($0-0.25):2 ps 1.2 pt 8 lc rgb '#FF0000' title '', \
  outdir.'/graph.pad.clang.txt'      using ($0-0.25):2 ps 1.0 pt 8 lc rgb '#008800' title '', \
  outdir.'/graph.vaio.msc.txt'       using ($0+0.00):2 ps 0.8 pt 4 lc rgb '#FF00FF' title '', \
  outdir.'/graph.vaio.gcc.txt'       using ($0+0.00):2 ps 1.2 pt 4 lc rgb '#FF0000' title '', \
  outdir.'/graph.vaio.clang.txt'     using ($0+0.00):2 ps 1.0 pt 4 lc rgb '#008800' title '', \
  outdir.'/graph.laguerre.icc.txt'   using ($0+0.25):2 ps 0.8 pt 6 lc rgb '#0000FF' title '', \
  outdir.'/graph.laguerre.gcc.txt'   using ($0+0.25):2 ps 1.2 pt 6 lc rgb '#FF0000' title '', \
  outdir.'/graph.laguerre.clang.txt' using ($0+0.25):2 ps 1.0 pt 6 lc rgb '#008800' title '', \
  NaN w p pt 8 lc rgb '#000000' title 'Linux 32bit (Core Duo T2300 1.66GHz)', \
  NaN w p pt 4 lc rgb '#000000' title 'Win10 64bit (Core i5-6200U 2.3GHz)', \
  NaN w p pt 6 lc rgb '#000000' title 'Linux 64bit (Xeon E5-2670 2.6GHz)', \
  NaN w p pt 5 ps 1.2 lc rgb '#FF0000' title 'gcc', \
  NaN w p pt 5 ps 1.0 lc rgb '#008800' title 'clang', \
  NaN w p pt 5 ps 0.8 lc rgb '#0000FF' title 'icc', \
  NaN w p pt 5 ps 0.8 lc rgb '#FF00FF' title 'msc'
