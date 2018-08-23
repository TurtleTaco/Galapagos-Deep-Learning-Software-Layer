#include <iomanip>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main()
{
    // int ar[] = {20, 30, 40, 50, 60, 70, 80, 90};
    // const int siz_ar = sizeof(ar) / sizeof(int);

    // // for (int i = 0; i < siz_ar; ++i)
    // //     cout << ar[i] << " ";
    // // cout << endl;

    // for (int i = 0; i < siz_ar; ++i)
    //     cout << hex << setfill('0') << setw(2) << ar[i] << " ";
    // cout << endl;

    float test;
    unsigned char current_float[4] = {0, 6, 0, 6};
    memcpy(&test, current_float, sizeof(float));
    printf("%d", (int)test);

    // float f;
    // unsigned char b[] = {'b3', 'b2', 'b1', 'b0'};
    // memcpy(&f, &b, sizeof(f));
    // printf("%d", (int)f);

    return 0;
}