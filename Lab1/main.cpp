#include <iostream>
#include <mmintrin.h>

using namespace std;

void print(__m64 A, __m64 B) {
    for (int i = 0; i < 4; i++) {
        cout<<(int)A.m64_i16[i]<<" ";
    }
    cout<<endl;
    for (int i = 0; i < 4; i++) {
        cout<<(int)B.m64_i16[i]<<" ";
    }
    cout<<endl;
}

__m64* task (__int8* A, __int8* B, __int8* C, __int16* D) {
    __m64 *A_tmp, *B_tmp, *C_tmp;
    A_tmp = (__m64*)A;
    B_tmp = (__m64*)B;
    C_tmp = (__m64*)C;
    __m64 zero_temp, cmpA, cmpB, cmpC;
    auto D_tmp = new __m64*[2];
    for (int i = 0; i < 2; i++) {
        D_tmp[i] = (__m64*)(D + i * 4);
    }
    zero_temp = _mm_setzero_si64();
    cmpA = _m_pcmpgtb(zero_temp, *A_tmp);
    cmpB = _m_pcmpgtb(zero_temp, *B_tmp);
    cmpC = _m_pcmpgtb(zero_temp, *C_tmp);

    auto hA = new __m64[2];
    hA[0] = _m_punpcklbw(*A_tmp, cmpA);
    hA[1] = _m_punpckhbw(*A_tmp, cmpA);
    auto hB = new __m64[2];
    hB[0] = _m_punpcklbw(*B_tmp, cmpB);
    hB[1] = _m_punpckhbw(*B_tmp, cmpB);
    auto sub_res = new __m64[2];
    for (int i = 0; i < 2; i++) {
        sub_res[i] = _m_psubw(hA[i], hB[i]);
    }
    cout<<"Sub result:"<<endl;
    print(sub_res[0], sub_res[1]);

    auto hC = new __m64[2];
    hC[0] = _m_punpcklbw(*C_tmp, cmpC);
    hC[1] = _m_punpckhbw(*C_tmp, cmpC);

    auto mul_res = new __m64[2];
    for (int i = 0; i < 2; i++) {
        mul_res[i] = _m_pmullw(hC[i], *D_tmp[i]);
    }
    cout<<"Mul result:"<<endl;
    print(mul_res[0], mul_res[1]);

    auto result = new __m64[2];
    for (int i = 0; i < 2; i++) {
        result[i] = _m_paddw(mul_res[i], sub_res[i]);
    }
    return result;
}

int* task2(__int8* A, __int8* B, __int8* C, __int16* D) {
    auto result = new int[8];
    for (int i = 0; i < 8; i++) {
        result[i] = ((int)A[i] - (int)B[i]) + ((int)C[i] * (int)D[i]);
    }
    return result;
}

int main() {
    __int8 A[8] = {-111, 2, 3, -40, 9, 8, 7, 6};
    __int8 B[8] = {113, 61, 71, 81, 3, 4, 2, 1};
    __int8 C[8] = {111, -5, -2, 6, 11, -12, 1, -9};
    __int16 D[8] = {60, -4, 9, -8, -123, 6, -43, -10};
    auto res = task(A, B, C, D);
    cout<<"Task result:"<<endl;
    print(res[0], res[1]);
    cout<<"Task result2:"<<endl;
    auto res2 = task2(A, B, C, D);
    for (int i = 0; i < 8; i++) {
        cout<<res2[i]<<" ";
    }
    return 0;
}


