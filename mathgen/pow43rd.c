
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    unsigned int i, j;
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

    printf("------\n");
    for (i=0;i < 20000;i++)
        printf("%u ^ 1/3 = %.20f\n",i,pow(i,1.0 / 3));

    printf("------\n");
    for (j=0;j < 10;j++) {
        unsigned int base;
        unsigned int basen;
        unsigned int basenn;
        unsigned int c;

        base = 1;
        for (c=0;c < 3;c++)
            base *= j;

        basen = 1;
        for (c=0;c < 3;c++)
            basen *= j+1;

        basenn = 1;
        for (c=0;c < 3;c++)
            basenn *= j+2;

        printf("j=%u base=%u basen=%u basenn=%u\n",j,base,basen,basenn);
        for (i=base;i <= basen;i++) {
            double x = pow(i,1.0 / 3) - j;

            printf("%u ^ 1/3 - %u = %.20f\n",i,base,x);

            if (x < -(1e-11) || x > (1+(1e-11))) {
                fprintf(stderr,"Precision error. x out of range.\n");
                abort();
            }
        }
    }

    return 0;
}

