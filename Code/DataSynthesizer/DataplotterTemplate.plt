#!/gnuplot
#
#    
#    	G N U P L O T
#    	Version 5.0 patchlevel 3    last modified 2016-02-21 
#    
#    	Copyright (C) 1986-1993, 1998, 2004, 2007-2016
#    	Thomas Williams, Colin Kelley and many others
#    
#    	gnuplot home:     http://www.gnuplot.info
#    	faq, bugs, etc:   type "help FAQ"
#    	immediate help:   type "help"  (plot window: hit 'h')
# set terminal wxt 0 enhanced
# set output


#STATISTICAL DATA
#print stats has to be done at start of file -> standard options
set terminal wxt
set datafile separator ","
set print "StatData.txt"

#Time to ...
print "Time to HR"
print ""
stats 'Data_all.csv' us 1:2
print ""
print ""
print "Time to EDA"
print ""
stats 'Data_all.csv' us 1:3
print ""
print ""
print "Time to Temp"
print ""
stats 'Data_all.csv' us 1:4
print ""
print ""
print "Time to Sound"
print ""
stats 'Data_all.csv' us 1:5
print ""
print ""
print "Time to Dust"
print ""
stats 'Data_all.csv' us 1:6
print ""
print ""
print "Time to WiFi"
print ""
stats 'Data_all.csv' us 1:7

#HR to ...
print ""
print ""
print "HR to EDA"
print ""
stats 'Data_all.csv' us 2:3
print ""
print ""
print "HR to Temp"
print ""
stats 'Data_all.csv' us 2:4
print ""
print ""
print "HR to Sound"
print ""
stats 'Data_all.csv' us 2:5
print ""
print ""
print "HR to Dust"
print ""
stats 'Data_all.csv' us 2:6
print ""
print ""
print "HR to WiFi"
print ""
stats 'Data_all.csv' us 2:7

#EDA to ...
print ""
print ""
print "EDA to Temp"
print ""
stats 'Data_all.csv' us 3:4
print ""
print ""
print "EDA to Sound"
print ""
stats 'Data_all.csv' us 3:5
print ""
print ""
print "EDA to Dust"
print ""
stats 'Data_all.csv' us 3:6
print ""
print ""
print "EDA to WiFi"
print ""
stats 'Data_all.csv' us 3:7

#Temp to ...
print ""
print ""
print "Temp to Sound"
print ""
stats 'Data_all.csv' us 4:5
print ""
print ""
print "Temp to Dust"
print ""
stats 'Data_all.csv' us 4:6
print ""
print ""
print "Temp to WiFi"
print ""
stats 'Data_all.csv' us 4:7

#Sound to ...
print ""
print ""
print "Sound to Dust"
print ""
stats 'Data_all.csv' us 5:6
print ""
print ""
print "Sound to WiFi"
print ""
stats 'Data_all.csv' us 5:7

#Dust to ...
print ""
print ""
print "Dust to WiFi"
print ""
stats 'Data_all.csv' us 6:7




#PLOTTING OF DATA STARTS HERE
#set separator char
set datafile separator ","

#alternatively plot data to png
set terminal png size 3000,2000 enhanced font "Helvetica,20" 

#set multiplot layout 2,2


#set line/point style and type
set style line 1 lt 1 lw 4 pt 1 ps 4

#read time properly on x axis change accordingly
set style data fsteps
set timefmt "%s"
set format x "%H:%M:%S"
set xdata time
set grid
set key left



set output "alldata.png"
set title "Time to Data Plot"
set xlabel "Date"
#set ylabel "Data"
plot "Data_all.csv" using 1:2 title "HR" with lines, "" using 1:3 title "EDA" with lines, "" using 1:4 title "Temp" with lines, "" using 1:5 title "Sound" with lines, "" using 1:6 title "Dust" with lines, "" using 1:7 title "#Wifi" with lines


#plot 1 HR
set output "HR-EDA.png"
set format x
set title "HR to EDA Plot"
set xlabel "HR"
set ylabel "EDA"
plot "Data_all.csv" using 2:3 title "HR-EDA" ls 1 with points




#plot 2
set output "HR-Temp.png"
set format x
set title "HR to Temp Plot"
set xlabel "HR"
set ylabel "Temp"
plot "Data_all.csv" using 2:4 title "HR-Temp" ls 1 with points




#plot 3
set output "HR-Sound.png"
set format x
set title "HR to Sound Plot"
set xlabel "HR"
set ylabel "Sound"
plot "Data_all.csv" using 2:5 title "HR-Sound" ls 1 with points




#plot 4
set output "HR-Dust.png"
set format x
set title "HR to Dust Plot"
set xlabel "HR"
set ylabel "Dust"
plot "Data_all.csv" using 2:6 title "HR-Dust" ls 1 with points




#plot 5
set output "HR-Wifi.png"
set format x
set title "HR to Wifi Plot"
set xlabel "HR"
set ylabel "Wifi"
plot "Data_all.csv" using 2:7 title "HR-Wifi" ls 1 with points




#plot 6 EDA
set output "EDA-Temp.png"
set format x
set title "EDA to Temp Plot"
set xlabel "EDA"
set ylabel "Temp"
plot "Data_all.csv" using 3:4 title "EDA-Temp" ls 1 with points




#plot 7
set output "EDA-Sound.png"
set format x
set title "EDA to Sound Plot"
set xlabel "EDA"
set ylabel "Sound"
plot "Data_all.csv" using 3:5 title "EDA-Sound" ls 1 with points




#plot 8
set output "EDA-Dust.png"
set format x
set title "EDA to Dust Plot"
set xlabel "EDA"
set ylabel "Dust"
plot "Data_all.csv" using 3:6 title "EDA-Dust" ls 1 with points




#plot 9
set output "EDA-Wifi.png"
set format x
set title "EDA to Wifi Plot"
set xlabel "EDA"
set ylabel "Wifi"
plot "Data_all.csv" using 3:7 title "EDA-Wifi" ls 1 with points




#plot 10 Temp
set output "Temp-Sound.png"
set format x
set title "Temp to Sound Plot"
set xlabel "Temp"
set ylabel "Sound"
plot "Data_all.csv" using 4:5 title "Temp-Sound" ls 1 with points




#plot 11
set output "Temp-Dust.png"
set format x
set title "Temp to Dust Plot"
set xlabel "Temp"
set ylabel "Dust"
plot "Data_all.csv" using 4:6 title "Temp-Dust" ls 1 with points




#plot 12
set output "Temp-Wifi.png"
set format x
set title "Temp to Wifi Plot"
set xlabel "Temp"
set ylabel "Wifi"
plot "Data_all.csv" using 4:7 title "Temp-Wifi" ls 1 with points




#plot 13 Sound
set output "Sound-Dust.png"
set format x
set title "Sound to Dust Plot"
set xlabel "Sound"
set ylabel "Dust"
plot "Data_all.csv" using 5:6 title "Sound-Dust" ls 1 with points




#plot 14
set output "Sound-Wifi.png"
set format x
set title "Sound to Wifi Plot"
set xlabel "Sound"
set ylabel "Wifi"
plot "Data_all.csv" using 5:7 title "Sound-Wifi" ls 1 with points




#plot 15 Dust
set output "Dust-Wifi.png"
set format x
set title "Dust to Wifi Plot"
set xlabel "Dust"
set ylabel "Wifi"
plot "Data_all.csv" using 6:7 title "Dust-Wifi" ls 1 with points





set output "Lon-Lat.png"
set format x
set title "Position Plot"
set xlabel "Lon"
set ylabel "Lat"
plot "Data_all.csv" using 8:9 title "Position" ls 1 with linespoints

#    EOF
