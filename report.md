# Report: Exercise 2, Data Parallelism – Parallel Mandelbrot
## Patrick Rössler (GRUPPE HAT SICH GEÄNDERT!)


### 1. Describe your approach (technology used, parallelization strategy, scheduling scheme (if any), additional findings, interesting remarks): 1 point.

Als Scheduling wird schedule(dynamic, 1) genutzt, damit Threads jeweils eine Zeile nach der anderen erhalten. Das ist bei Mandelbrot sinnvoll, weil der Rechenaufwand pro Pixel stark variiert (insbesondere an den Fraktalrändern) und statisches Scheduling sonst zu Lastungleichgewicht führen kann.

Für die Auswertung messe ich zuerst die serielle Laufzeit, danach parallele Läufe und berechne den Speedup \(S = T_1 / T_n\).  
Die gemessenen Werte zeigen einen sehr guten Speedup:
- 1 Thread: 5.933 s
- 2 Threads: Speedup 1.97833
- 4 Threads: Speedup 3.88031
- 8 Threads: Speedup 6.79611
- 16 Threads: Speedup 10.6517

Zusätzlich werden automatisch ein Mandelbrot-Bild mandelbrot.png und eine Speedup-Grafik speedup.png erzeugt.  
Durch die Speedup Grafik ist gut zu sehen, dass der Speedup bei höheren Threads nicht mehr linear steigt.

### 2.Provide an example image and either the command line parameters or at least the settings used
for generation (e.g. when you used a UI instead of command line arguments). Generation of the a
correct image: 2 points;

--> Mandelbot.png

mit den Arguments

```
const int WIDTH  = 2048;
const int HEIGHT = 2048;

const double MIN_X = -2.0;
const double MAX_X =  1.0;
const double MIN_Y = -1.0;
const double MAX_Y =  1.0;
```

### 3. Performance measurements and comparison to serial implementation. For benchmarking measure
the pure computation time on a 2048 times 2048 image (without writing to disk or blitting to
screen) and provide a graph showing the speedup quotient Ts/Tn where Ts is the execution time
on a single thread and Tn is the execution time on n threads (n <= number of processor cores): 3
points.

--> speedup.png