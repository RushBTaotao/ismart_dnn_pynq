

// Conv 1x1 PE

#include <stdio.h>
#include <math.h>
#include <ap_fixed.h>
#include "hls_stream.h"
#include "net_hls.h"


FIX_32_12 compute_engine_16(FIX_WT_8_1 w0,  FIX_FM_16_6 b0,
					  FIX_WT_8_1 w1,  FIX_FM_16_6 b1,
					  FIX_WT_8_1 w2,  FIX_FM_16_6 b2,
					  FIX_WT_8_1 w3,  FIX_FM_16_6 b3,
					  FIX_WT_8_1 w4,  FIX_FM_16_6 b4,
					  FIX_WT_8_1 w5,  FIX_FM_16_6 b5,
					  FIX_WT_8_1 w6,  FIX_FM_16_6 b6,
					  FIX_WT_8_1 w7,  FIX_FM_16_6 b7,
					  FIX_WT_8_1 w8,  FIX_FM_16_6 b8,
					  FIX_WT_8_1 w9,  FIX_FM_16_6 b9,
					  FIX_WT_8_1 w10, FIX_FM_16_6 b10,
					  FIX_WT_8_1 w11, FIX_FM_16_6 b11,
					  FIX_WT_8_1 w12, FIX_FM_16_6 b12,
					  FIX_WT_8_1 w13, FIX_FM_16_6 b13,
					  FIX_WT_8_1 w14, FIX_FM_16_6 b14,
					  FIX_WT_8_1 w15, FIX_FM_16_6 b15)
{
	FIX_32_12 mul0, mul1, mul2,  mul3,  mul4,  mul5,  mul6,  mul7;
	FIX_32_12 mul8, mul9, mul10, mul11, mul12, mul13, mul14, mul15;
	FIX_32_12 add0, add1, add2, add3,  add4,  add5,  add6;
	FIX_32_12 add7, add8, add9, add10, add11, add12, add13, add14;

	mul0  = w0  * b0;
	mul1  = w1  * b1;
	mul2  = w2  * b2;
	mul3  = w3  * b3;
	mul4  = w4  * b4;
	mul5  = w5  * b5;
	mul6  = w6  * b6;
	mul7  = w7  * b7;
	mul8  = w8  * b8;
	mul9  = w9  * b9;
	mul10 = w10 * b10;
	mul11 = w11 * b11;
	mul12 = w12 * b12;
	mul13 = w13 * b13;
	mul14 = w14 * b14;
	mul15 = w15 * b15;

  // add tree style ;
	add0 = mul0  + mul1;
	add1 = mul2  + mul3;
	add2 = mul4  + mul5;
	add3 = mul6  + mul7;
	add4 = mul8  + mul9;
	add5 = mul10 + mul11;
	add6 = mul12 + mul13;
	add7 = mul14 + mul15;

	add8  = add0 + add1;
	add9  = add2 + add3;
	add10 = add4 + add5;
	add11 = add6 + add7;

	add12 = add8  + add9;
	add13 = add10 + add11;

	add14 = add12 + add13;

	return add14;

}



FIX_FM_16_6 compute_engine_8(FIX_WT_8_1 w0,  FIX_FM_16_6 b0,
					  FIX_WT_8_1 w1,  FIX_FM_16_6 b1,
					  FIX_WT_8_1 w2,  FIX_FM_16_6 b2,
					  FIX_WT_8_1 w3,  FIX_FM_16_6 b3,
					  FIX_WT_8_1 w4,  FIX_FM_16_6 b4,
					  FIX_WT_8_1 w5,  FIX_FM_16_6 b5,
					  FIX_WT_8_1 w6,  FIX_FM_16_6 b6,
					  FIX_WT_8_1 w7,  FIX_FM_16_6 b7)
{
	FIX_32_16 mul0, mul1, mul2,  mul3,  mul4,  mul5,  mul6,  mul7;
	FIX_32_16 add0, add1, add2, add3,  add4,  add5,  add6;

	mul0  = w0  * b0;
	mul1  = w1  * b1;
	mul2  = w2  * b2;
	mul3  = w3  * b3;
	mul4  = w4  * b4;
	mul5  = w5  * b5;
	mul6  = w6  * b6;
	mul7  = w7  * b7;

	add0 = mul0  + mul1;
	add1 = mul2  + mul3;
	add2 = mul4  + mul5;
	add3 = mul6  + mul7;

	add4 = add0  + add1;
	add5 = add2 + add3;

	add6 = add4 + add5;

	return (FIX_FM_16_6)add6;

}


void CONV_1x1(FIX_FM_16_6 bottom[16][22][42],
			  FIX_FM_16_6 top[16][22][42],  // always buf
			  FIX_WT_8_1 weights[16][16])
{
FIX_WT_8_1 weight_buf[16][16];
FIX_32_12 tmp[16];

#pragma HLS ARRAY_PARTITION variable=bottom cyclic dim=1 factor=16
#pragma HLS ARRAY_PARTITION variable=top cyclic dim=1 factor=16
#pragma HLS ARRAY_PARTITION variable=weight_buf dim=1 factor=16
#pragma HLS ARRAY_PARTITION variable=weight_buf dim=2 factor=16
#pragma HLS ARRAY_PARTITION variable=tmp complete
// implement 8 hardward core
#pragma HLS ALLOCATION instances=compute_engine_16 limit=8 function

	for(int i = 0; i < 16; i++)
		for(int j = 0; j < 16; j++)
			weight_buf[i][j] = weights[i][j];


	for(int h = 1; h <= 20; h++){
		for(int w = 1; w <= 40; w++) {

//			for(int co = 0; co < 16; co+=8) {
#pragma HLS pipeline
			for(int co = 0; co < 16; co+=16) { //not used
				for(int coo = 0; coo < 16; coo++) {
#pragma HLS unroll
                                             //weight_buf: [out channel][in channel]
											 //bottom :[in channel][h][w]
					tmp[coo] = compute_engine_16(
												 weight_buf[co+coo][0],  bottom[0][h][w], 
												 weight_buf[co+coo][1],  bottom[1][h][w],
												 weight_buf[co+coo][2],  bottom[2][h][w],
												 weight_buf[co+coo][3],  bottom[3][h][w],
												 weight_buf[co+coo][4],  bottom[4][h][w],
												 weight_buf[co+coo][5],  bottom[5][h][w],
												 weight_buf[co+coo][6],  bottom[6][h][w],
												 weight_buf[co+coo][7],  bottom[7][h][w],
												 weight_buf[co+coo][8],  bottom[8][h][w],
												 weight_buf[co+coo][9],  bottom[9][h][w],
												 weight_buf[co+coo][10], bottom[10][h][w],
												 weight_buf[co+coo][11], bottom[11][h][w],
												 weight_buf[co+coo][12], bottom[12][h][w],
												 weight_buf[co+coo][13], bottom[13][h][w],
												 weight_buf[co+coo][14], bottom[14][h][w],
												 weight_buf[co+coo][15], bottom[15][h][w]);
				}

				for(int coo = 0; coo < 16; coo++)
#pragma HLS unroll
					top[co+coo][h][w] += tmp[coo];

			}
		}
	}
}
