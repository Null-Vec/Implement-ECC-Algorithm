#include<stdio.h>
#include<stdlib.h>
#include <string.h>

typedef long long unsigned int u64;
typedef unsigned int u32;
typedef unsigned char u8;

//The chosen prime "0xE92E40AD6F281C8A082AFDC49E1372659455BEC8CEEA043A614C835B7FE9EFF5"
const u64 p[9]		= {0xe92e40, 0x15ade503, 0x122820ab, 0x1ee24f09, 0x17265945, 0xb7d919d, 0x1a810e98, 0xa641adb, 0x1fe9eff5};
const u64 mu[10]	= {0x23, 0x435b5a4, 0x1777172, 0x696b212, 0x12508c04, 0x115a5eea, 0x1812b7cb, 0x171d776b, 0x1d398149, 0x1ae00018};
const u64 g[9]		= {0, 0, 0, 0, 0, 0, 0, 0, 2};

const u64 p_2[9]	= {0xe92e40, 0x15ade503, 0x122820ab, 0x1ee24f09, 0x17265945, 0xb7d919d, 0x1a810e98, 0xa641adb, 0x1fe9eff3}; // (m-2) {Required for mod_inv}

//Multiplication of 10 word(29bit) long integers
void mult10(const u64* x, const u64* y, u64* z){
	z[18] = x[9]*y[9];
	z[17] = x[8]*y[9] + x[9]*y[8];
	z[16] = x[7]*y[9] + x[8]*y[8] + x[9]*y[7];
	z[15] = x[6]*y[9] + x[7]*y[8] + x[8]*y[7] + x[9]*y[6];
	z[14] = x[5]*y[9] + x[6]*y[8] + x[7]*y[7] + x[8]*y[6] + x[9]*y[5];
	z[13] = x[4]*y[9] + x[5]*y[8] + x[6]*y[7] + x[7]*y[6] + x[8]*y[5] + x[9]*y[4];
	z[12] = x[3]*y[9] + x[4]*y[8] + x[5]*y[7] + x[6]*y[6] + x[7]*y[5] + x[8]*y[4] + x[9]*y[3];
	z[11] = x[2]*y[9] + x[3]*y[8] + x[4]*y[7] + x[5]*y[6] + x[6]*y[5] + x[7]*y[4] + x[8]*y[3] + x[9]*y[2];
	z[10] = x[1]*y[9] + x[2]*y[8] + x[3]*y[7] + x[4]*y[6] + x[5]*y[5] + x[6]*y[4] + x[7]*y[3] + x[8]*y[2] + x[9]*y[1];
	z[9]  = x[0]*y[9] + x[1]*y[8] + x[2]*y[7] + x[3]*y[6] + x[4]*y[5] + x[5]*y[4] + x[6]*y[3] + x[7]*y[2] + x[8]*y[1] + x[9]*y[0];
	z[8]  = x[0]*y[8] + x[1]*y[7] + x[2]*y[6] + x[3]*y[5] + x[4]*y[4] + x[5]*y[3] + x[6]*y[2] + x[7]*y[1] + x[8]*y[0];
	z[7]  = x[0]*y[7] + x[1]*y[6] + x[2]*y[5] + x[3]*y[4] + x[4]*y[3] + x[5]*y[2] + x[6]*y[1] + x[7]*y[0];
	z[6]  = x[0]*y[6] + x[1]*y[5] + x[2]*y[4] + x[3]*y[3] + x[4]*y[2] + x[5]*y[1] + x[6]*y[0];
	z[5]  = x[0]*y[5] + x[1]*y[4] + x[2]*y[3] + x[3]*y[2] + x[4]*y[1] + x[5]*y[0];
	z[4]  = x[0]*y[4] + x[1]*y[3] + x[2]*y[2] + x[3]*y[1] + x[4]*y[0];
	z[3]  = x[0]*y[3] + x[1]*y[2] + x[2]*y[1] + x[3]*y[0];
	z[2]  = x[0]*y[2] + x[1]*y[1] + x[2]*y[0];
	z[1]  = x[0]*y[1] + x[1]*y[0];
	z[0]  = x[0]*y[0];
	
	u64 carry = z[18] >> 29; z[18] &= 0x1fffffff;
	z[17] += carry; carry = z[17] >> 29; z[17] &= 0x1fffffff;
	z[16] += carry; carry = z[16] >> 29; z[16] &= 0x1fffffff;
	z[15] += carry; carry = z[15] >> 29; z[15] &= 0x1fffffff;
	z[14] += carry; carry = z[14] >> 29; z[14] &= 0x1fffffff;
	z[13] += carry; carry = z[13] >> 29; z[13] &= 0x1fffffff;
	z[12] += carry; carry = z[12] >> 29; z[12] &= 0x1fffffff;
	z[11] += carry; carry = z[11] >> 29; z[11] &= 0x1fffffff;
	z[10] += carry; carry = z[10] >> 29; z[10] &= 0x1fffffff;
	z[9]  += carry; carry = z[9]  >> 29; z[9]  &= 0x1fffffff;
	z[8]  += carry; carry = z[8]  >> 29; z[8]  &= 0x1fffffff;
	z[7]  += carry; carry = z[7]  >> 29; z[7]  &= 0x1fffffff;
	z[6]  += carry; carry = z[6]  >> 29; z[6]  &= 0x1fffffff;
	z[5]  += carry; carry = z[5]  >> 29; z[5]  &= 0x1fffffff;
	z[4]  += carry; carry = z[4]  >> 29; z[4]  &= 0x1fffffff;
	z[3]  += carry; carry = z[3]  >> 29; z[3]  &= 0x1fffffff;
	z[2]  += carry; carry = z[2]  >> 29; z[2]  &= 0x1fffffff;
	z[1]  += carry; carry = z[1]  >> 29; z[1]  &= 0x1fffffff;
	z[0]  += carry;
}

//Multiplication of 9 word(29bit) long integers
void mult9(const u64* x, const u64* y, u64* z){
	z[17] = x[8]*y[8];
	z[16] = x[7]*y[8] + x[8]*y[7];
	z[15] = x[6]*y[8] + x[7]*y[7] + x[8]*y[6];
	z[14] = x[5]*y[8] + x[6]*y[7] + x[7]*y[6] + x[8]*y[5];
	z[13] = x[4]*y[8] + x[5]*y[7] + x[6]*y[6] + x[7]*y[5] + x[8]*y[4];
	z[12] = x[3]*y[8] + x[4]*y[7] + x[5]*y[6] + x[6]*y[5] + x[7]*y[4] + x[8]*y[3];
	z[11] = x[2]*y[8] + x[3]*y[7] + x[4]*y[6] + x[5]*y[5] + x[6]*y[4] + x[7]*y[3] + x[8]*y[2];
	z[10] = x[1]*y[8] + x[2]*y[7] + x[3]*y[6] + x[4]*y[5] + x[5]*y[4] + x[6]*y[3] + x[7]*y[2] + x[8]*y[1];
	z[9]  = x[0]*y[8] + x[1]*y[7] + x[2]*y[6] + x[3]*y[5] + x[4]*y[4] + x[5]*y[3] + x[6]*y[2] + x[7]*y[1] + x[8]*y[0];
	z[8]  = x[0]*y[7] + x[1]*y[6] + x[2]*y[5] + x[3]*y[4] + x[4]*y[3] + x[5]*y[2] + x[6]*y[1] + x[7]*y[0];
	z[7]  = x[0]*y[6] + x[1]*y[5] + x[2]*y[4] + x[3]*y[3] + x[4]*y[2] + x[5]*y[1] + x[6]*y[0];
	z[6]  = x[0]*y[5] + x[1]*y[4] + x[2]*y[3] + x[3]*y[2] + x[4]*y[1] + x[5]*y[0];
	z[5]  = x[0]*y[4] + x[1]*y[3] + x[2]*y[2] + x[3]*y[1] + x[4]*y[0];
	z[4]  = x[0]*y[3] + x[1]*y[2] + x[2]*y[1] + x[3]*y[0];
	z[3]  = x[0]*y[2] + x[1]*y[1] + x[2]*y[0];
	z[2]  = x[0]*y[1] + x[1]*y[0];
	z[1]  = x[0]*y[0];
	z[0]  = 0;
	
				u64 carry = z[17] >> 29; z[17] &= 0x1fffffff;
	z[16] += carry; carry = z[16] >> 29; z[16] &= 0x1fffffff;
	z[15] += carry; carry = z[15] >> 29; z[15] &= 0x1fffffff;
	z[14] += carry; carry = z[14] >> 29; z[14] &= 0x1fffffff;
	z[13] += carry; carry = z[13] >> 29; z[13] &= 0x1fffffff;
	z[12] += carry; carry = z[12] >> 29; z[12] &= 0x1fffffff;
	z[11] += carry; carry = z[11] >> 29; z[11] &= 0x1fffffff;
	z[10] += carry; carry = z[10] >> 29; z[10] &= 0x1fffffff;
	z[9]  += carry; carry = z[9]  >> 29; z[9]  &= 0x1fffffff;
	z[8]  += carry; carry = z[8]  >> 29; z[8]  &= 0x1fffffff;
	z[7]  += carry; carry = z[7]  >> 29; z[7]  &= 0x1fffffff;
	z[6]  += carry; carry = z[6]  >> 29; z[6]  &= 0x1fffffff;
	z[5]  += carry; carry = z[5]  >> 29; z[5]  &= 0x1fffffff;
	z[4]  += carry; carry = z[4]  >> 29; z[4]  &= 0x1fffffff;
	z[3]  += carry; carry = z[3]  >> 29; z[3]  &= 0x1fffffff;
	z[2]  += carry; carry = z[2]  >> 29; z[2]  &= 0x1fffffff;
	z[1]  += carry; carry = z[1]  >> 29; z[1]  &= 0x1fffffff;
	z[0]   = carry;
}


//Subtraction using 2's complement
u8 sub(const u64* a, const u64* b, u64* result){
	u8 carry = 1;
	result[8] = a[8] + (b[8]^0x1fffffff) + carry; carry = result[8] >> 29; result[8] &= 0x1fffffff;
	result[7] = a[7] + (b[7]^0x1fffffff) + carry; carry = result[7] >> 29; result[7] &= 0x1fffffff;
	result[6] = a[6] + (b[6]^0x1fffffff) + carry; carry = result[6] >> 29; result[6] &= 0x1fffffff;
	result[5] = a[5] + (b[5]^0x1fffffff) + carry; carry = result[5] >> 29; result[5] &= 0x1fffffff;
	result[4] = a[4] + (b[4]^0x1fffffff) + carry; carry = result[4] >> 29; result[4] &= 0x1fffffff;
	result[3] = a[3] + (b[3]^0x1fffffff) + carry; carry = result[3] >> 29; result[3] &= 0x1fffffff;
	result[2] = a[2] + (b[2]^0x1fffffff) + carry; carry = result[2] >> 29; result[2] &= 0x1fffffff;
	result[1] = a[1] + (b[1]^0x1fffffff) + carry; carry = result[1] >> 29; result[1] &= 0x1fffffff;
	result[0] = a[0] + (b[0]^0x1fffffff) + carry; carry = result[0] >> 29; result[0] &= 0x1fffffff;
	
	return carry;
}

//Addition modulo "p"
void add(const u64* a, const u64* b, u64* result){
	u8 carry = 0;
	result[8] = a[8] + b[8] + carry; carry = result[8] >> 29; result[8] &= 0x1fffffff;
	result[7] = a[7] + b[7] + carry; carry = result[7] >> 29; result[7] &= 0x1fffffff;
	result[6] = a[6] + b[6] + carry; carry = result[6] >> 29; result[6] &= 0x1fffffff;
	result[5] = a[5] + b[5] + carry; carry = result[5] >> 29; result[5] &= 0x1fffffff;
	result[4] = a[4] + b[4] + carry; carry = result[4] >> 29; result[4] &= 0x1fffffff;
	result[3] = a[3] + b[3] + carry; carry = result[3] >> 29; result[3] &= 0x1fffffff;
	result[2] = a[2] + b[2] + carry; carry = result[2] >> 29; result[2] &= 0x1fffffff;
	result[1] = a[1] + b[1] + carry; carry = result[1] >> 29; result[1] &= 0x1fffffff;
	result[0] = a[0] + b[0] + carry; carry = result[0] >> 29; result[0] &= 0x1fffffff;
	
	u64 temp[9] = {0};
	u64 mask = -sub(result,p,temp);
    result[8] = (temp[8] & mask) ^ (result[8] & ~mask);
	result[7] = (temp[7] & mask) ^ (result[7] & ~mask);
	result[6] = (temp[6] & mask) ^ (result[6] & ~mask);
	result[5] = (temp[5] & mask) ^ (result[5] & ~mask);
	result[4] = (temp[4] & mask) ^ (result[4] & ~mask);
	result[3] = (temp[3] & mask) ^ (result[3] & ~mask);
	result[2] = (temp[2] & mask) ^ (result[2] & ~mask);
	result[1] = (temp[1] & mask) ^ (result[1] & ~mask);
	result[0] = (temp[0] & mask) ^ (result[0] & ~mask);
}

//Subtraction modulo "p"
void mod_sub(const u64* a, const u64* b, u64* result){
	u64 temp[9];
	u64 mask = -sub(a,b,result);
	u8 carry = 0;
	temp[8] = result[8] + p[8] + carry; carry = temp[8] >> 29; temp[8] &= 0x1fffffff;
	temp[7] = result[7] + p[7] + carry; carry = temp[7] >> 29; temp[7] &= 0x1fffffff;
	temp[6] = result[6] + p[6] + carry; carry = temp[6] >> 29; temp[6] &= 0x1fffffff;
	temp[5] = result[5] + p[5] + carry; carry = temp[5] >> 29; temp[5] &= 0x1fffffff;
	temp[4] = result[4] + p[4] + carry; carry = temp[4] >> 29; temp[4] &= 0x1fffffff;
	temp[3] = result[3] + p[3] + carry; carry = temp[3] >> 29; temp[3] &= 0x1fffffff;
	temp[2] = result[2] + p[2] + carry; carry = temp[2] >> 29; temp[2] &= 0x1fffffff;
	temp[1] = result[1] + p[1] + carry; carry = temp[1] >> 29; temp[1] &= 0x1fffffff;
	temp[0] = result[0] + p[0] + carry; carry = temp[0] >> 29; temp[0] &= 0x1fffffff;

	result[8] = (temp[8] & ~mask) ^ (result[8] & mask);
	result[7] = (temp[7] & ~mask) ^ (result[7] & mask);
	result[6] = (temp[6] & ~mask) ^ (result[6] & mask);
	result[5] = (temp[5] & ~mask) ^ (result[5] & mask);
	result[4] = (temp[4] & ~mask) ^ (result[4] & mask);
	result[3] = (temp[3] & ~mask) ^ (result[3] & mask);
	result[2] = (temp[2] & ~mask) ^ (result[2] & mask);
	result[1] = (temp[1] & ~mask) ^ (result[1] & mask);
	result[0] = (temp[0] & ~mask) ^ (result[0] & mask);
}

// Barrett Modular Reduction
void barrett(const u64* a, u64* result){
	u64 temp[9],q2[19],q3[18],r[9];
    u64 mask;

	mult10(a, mu, q2);
	mult9(q2, p, q3);
	sub(a + 9, q3 + 9, result);

    // Perform the first subtraction unconditionally and calculate the mask
    mask = -sub(result, p, temp);
    for (u8 i = 0; i < 9; i++)
        result[i] = (temp[i] & mask) | (result[i] & ~mask); 

	//Repeat to ensure reduction
    mask = -sub(result, p, temp);
    for (u8 i = 0; i < 9; i++)
    	result[i] = (temp[i] & mask) | (result[i] & ~mask);
}

//Square and reduction modulo "p"
void square(const u64* x, u64* result){
	u64 z[18];
	z[17] = x[8]*x[8];
	z[16] = (x[7]*x[8]) << 1;
	z[15] = ((x[6]*x[8]) << 1) + x[7]*x[7];
	z[14] = ((x[5]*x[8]) << 1) + ((x[6]*x[7]) << 1);
	z[13] = ((x[4]*x[8]) << 1) + ((x[5]*x[7]) << 1) + x[6]*x[6];
	z[12] = ((x[3]*x[8]) << 1) + ((x[4]*x[7]) << 1) + ((x[5]*x[6]) << 1);
	z[11] = ((x[2]*x[8]) << 1) + ((x[3]*x[7]) << 1) + ((x[4]*x[6]) << 1) + x[5]*x[5];
	z[10] = ((x[1]*x[8]) << 1) + ((x[2]*x[7]) << 1) + ((x[3]*x[6]) << 1) + ((x[4]*x[5]) << 1);
	z[9]  = ((x[0]*x[8]) << 1) + ((x[1]*x[7]) << 1) + ((x[2]*x[6]) << 1) + ((x[3]*x[5]) << 1) + x[4]*x[4];
	z[8]  = ((x[0]*x[7]) << 1) + ((x[1]*x[6]) << 1) + ((x[2]*x[5]) << 1) + ((x[3]*x[4]) << 1);
	z[7]  = ((x[0]*x[6]) << 1) + ((x[1]*x[5]) << 1) + ((x[2]*x[4]) << 1) + x[3]*x[3];
	z[6]  = ((x[0]*x[5]) << 1) + ((x[1]*x[4]) << 1) + ((x[2]*x[3]) << 1);
	z[5]  = ((x[0]*x[4]) << 1) + ((x[1]*x[3]) << 1) + x[2]*x[2];
	z[4]  = ((x[0]*x[3]) << 1) + ((x[1]*x[2]) << 1);
	z[3]  = ((x[0]*x[2]) << 1) + x[1]*x[1];
	z[2]  = ((x[0]*x[1]) << 1);
	z[1]  = x[0]*x[0];
	z[0]  = 0;
	
	u64 carry = z[17] >> 29; z[17] &= 0x1fffffff;
	z[16] += carry; carry = z[16] >> 29; z[16] &= 0x1fffffff;
	z[15] += carry; carry = z[15] >> 29; z[15] &= 0x1fffffff;
	z[14] += carry; carry = z[14] >> 29; z[14] &= 0x1fffffff;
	z[13] += carry; carry = z[13] >> 29; z[13] &= 0x1fffffff;
	z[12] += carry; carry = z[12] >> 29; z[12] &= 0x1fffffff;
	z[11] += carry; carry = z[11] >> 29; z[11] &= 0x1fffffff;
	z[10] += carry; carry = z[10] >> 29; z[10] &= 0x1fffffff;
	z[9]  += carry; carry = z[9]  >> 29; z[9]  &= 0x1fffffff;
	z[8]  += carry; carry = z[8]  >> 29; z[8]  &= 0x1fffffff;
	z[7]  += carry; carry = z[7]  >> 29; z[7]  &= 0x1fffffff;
	z[6]  += carry; carry = z[6]  >> 29; z[6]  &= 0x1fffffff;
	z[5]  += carry; carry = z[5]  >> 29; z[5]  &= 0x1fffffff;
	z[4]  += carry; carry = z[4]  >> 29; z[4]  &= 0x1fffffff;
	z[3]  += carry; carry = z[3]  >> 29; z[3]  &= 0x1fffffff;
	z[2]  += carry; carry = z[2]  >> 29; z[2]  &= 0x1fffffff;
	z[1]  += carry; carry = z[1]  >> 29; z[1]  &= 0x1fffffff;
	z[0]   = carry;

	barrett(z,result);
}

// Multiplication along with reduction
void mult(const u64* inputA, const u64* inputB, u64* result){
    u64 temp[18];
    mult9(inputA, inputB, temp);
    barrett(temp,result);
}

// Modular Exponentiation (Right to left)
void modexp_rtl(const u64* b, const u64* exp, u64* result){
    u64 temp[18], result_copy[9], base[9];
	memset(result, 0, 72); memcpy(base, b, 72);
	result[8] = 1;
    // Fixed maximum number of iterations (assume exp has a fixed bit size, 256 bits)
    for (int i = 0; i < 256; i++) {
        // Prepare to check the corresponding significant bit of exp
        u8 p = i%29;
		u8 sb = (exp[8- (i/29)] >> p) & 0x1;
        u64 mask = -sb; // mask is 0xFFFFFFFFFFFFFFFF if lsb is 1, otherwise 0x0
		
        // Perform multiplication and reduction unconditionally
        mult(result, base, result_copy);

        // Apply the result of multiplication conditionally based on the least significant bit of exp
        for (int j = 0; j < 9; j++)
            result[j] = (result_copy[j] & mask) | (result[j] & ~mask); // Update result conditionally

        // Perform the squaring and modular reduction unconditionally
        square(base, base);
    }
}

//Modular Exponentiation (Left to right)
void modexp_ltr(const u64* base, const u64* exp, u64* result){
    u64 temp[18], result_copy[9];
	memset(result, 0, 72);
	result[8] = 1;
    for (int i = 255; i>=0; i--) {
        u8 p = i%29;
		u8 sb = (exp[8- (i/29)] >> p) & 0x1;
        u64 mask = -sb;
		square(result, result);
        mult(result, base, result_copy);//can be replaced with times2 for base=2
        for (int j = 0; j < 9; j++)
            result[j] = (result_copy[j] & mask) | (result[j] & ~mask);
    }
}

//Inverse in F_p
void mod_inv(const u64* a, u64* result){
	modexp_rtl(a,p_2,result);
}


//Doubling modulo "p"
void times2(const u64* a, u64* result){
	u8 carry = 0;
	for (int i=8; i>=0; i--){
		result[i] = (a[i] << 1) ^ carry;
		carry = result[i] >> 29;
		result[i] &= 0x1fffffff;
	}
	u64 temp[9] = {0};
	u64 mask = -sub(result,p,temp);
	for (int j = 0; j < 9; j++)
        result[j] = (temp[j] & mask) | (result[j] & ~mask);
}

//Tripling modulo "p"
void times3(const u64* a, u64* result){
	u8 carry = 0;
	for (int i=8; i>=0; i--){
		result[i] = a[i] + (a[i] << 1) + carry;
		carry = result[i] >> 29;
		result[i] &= 0x1fffffff;
	}
	u64 temp[9] = {0};
	u64 mask = -sub(result,p,temp);
	for (int j = 0; j < 9; j++)
        result[j] = (temp[j] & mask) | (result[j] & ~mask);
	mask = -sub(result,p,temp);
	for (int j = 0; j < 9; j++)
        result[j] = (temp[j] & mask) | (result[j] & ~mask);
}