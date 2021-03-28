# Set linestyle 1 to blue (#0060ad)
set style line 1 \
    linecolor rgb '#0060ad' \
    linetype 1 linewidth 2 \
    pointtype 7 pointsize 1.5
set style line 2 \
    lc rgb 'black' pt 7
plot 'salida.txt' index 0 with linespoints ls 1, \
     ''                   index 1 with points ls 2 \
     notitle