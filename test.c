#include"final.h"

int main(){
    u64 secret[9] = {0};
    char my_secret[] = "E000000000000000000000000000000000000000000000000000000000000000";
    parse_to_int(my_secret, secret);

    ECPoint base, p1, p2;
    for(u8 i=0; i<9; i++) {
        p1.x[i] = p2.x[i] = base.x[i] = 0;
        p1.y[i] = p2.y[i] = base.y[i] = 0;
        p1.z[i] = p2.z[i] = base.z[i] = 0;
    }
    base.y[8] = base.z[8] = p1.z[8] = p2.z[8] = 1;

    u64 exp[9] = {0};

    ECPoint final, temp;
    ECPoint* result = &temp;
    //for (int i =0; i<1000; i++)
    //    point_mult(base, secret, &result);
    point_double(base, &temp);
    //point_add(p1, p2, &result);
    PtoA(temp, &final);
    print_point(final);
    return 0;
}