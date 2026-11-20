/* mandelbrot.c -- draws the Mandelbrot set as text, with the math library (math.h) */

#include <math.h>
#include <stdio.h>

#define WIDTH 64
#define HEIGHT 24
#define MAX_ITER 60

/* the number of iterations before |z| leaves the circle of radius 2, or MAX_ITER */
static int escape(double cr, double ci)
{
    double zr = 0;
    double zi = 0;
    int    n;

    for (n = 0; n < MAX_ITER; n++)
    {
        double zr2 = zr * zr;
        double zi2 = zi * zi;

        if (zr2 + zi2 > 4)
        {
            break;
        }
        zi = 2 * zr * zi + ci;
        zr = zr2 - zi2 + cr;
    }
    return n;
}

int main(void)
{
    const char* shades = " .:-=+*#%@";
    int         row;
    int         col;

    for (row = 0; row < HEIGHT; row++)
    {
        double ci = -1.2 + 2.4 * row / (HEIGHT - 1);

        for (col = 0; col < WIDTH; col++)
        {
            double cr = -2.2 + 3.0 * col / (WIDTH - 1);
            int    n = escape(cr, ci);

            putchar(n == MAX_ITER ? '@' : shades[n % 9]);
        }
        putchar('\n');
    }
    printf("%d columns, %d rows, %d iterations at most\n", WIDTH, HEIGHT, MAX_ITER);
    return 0;
}
