
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    unsigned int i;
    double x, chk, dev;

    /* MPEG Layer III scales samples by x' = x^(4/3) so this code is written
     * to generate lookup tables to speed that process up.
     *
     * The sample value is signed but pow() is given the absolute value of the sample */

    /* x' = x ^ (4/3)
     *
     * which is equivalent to:
     *
     * x' = (x ^ 1) * (x ^ (1/3))
     *
     * or
     *
     * x' = x * cube_root(x)
     *
     * cube_root(x) = x ^ (1/3)
     */

    /* prove: x ^ (4/3) = x * ( x ^ (4/3) ) */
    for (i=0;i < 20000;i++) {
        x = i * pow(i,1.0 / 3);
        chk = pow(i,4.0 / 3);
        dev = x - chk;

        if (fabs(dev) > 1e-9) {
            fprintf(stderr,"Precision error. x=%.11f chk=%.11f dev=%.20f\n",x,chk,dev);
            abort();
        }
    }

    return 0;
}

