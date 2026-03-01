#pragma once

struct point
{
	float fx;
	float fy;
	float fz;
};

struct texturecoord
{
	float fu;
	float fv;
};

void vecMult(float b[4], float a[16], float vout[4])
{
	vout[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	vout[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	vout[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	vout[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

}

void vecMultd(double b[4], float a[16], float vout[4])
{
	vout[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	vout[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	vout[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	vout[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

}

void crossProduct(float v_A[], float v_B[], float c_P[]) {

	c_P[0] = (v_A[1] * v_B[2] - v_A[2] * v_B[1]);
	c_P[1] = -(v_A[0] * v_B[2] - v_A[2] * v_B[0]);
	c_P[2] = (v_A[0] * v_B[1] - v_A[1] * v_B[0]);

}

void transpose(float mat[16], float tMat[16])
{
	tMat[0] = mat[0]; tMat[4] = mat[1]; tMat[8] = mat[2]; tMat[12] = -(mat[12] * mat[0] + mat[13] * mat[1] + mat[14] * mat[2]);
	tMat[1] = mat[4]; tMat[5] = mat[5]; tMat[9] = mat[6]; tMat[13] = -(mat[12] * mat[4] + mat[13] * mat[5] + mat[14] * mat[6]);
	tMat[2] = mat[8]; tMat[6] = mat[9]; tMat[10] = mat[10]; tMat[14] = -(mat[12] * mat[8] + mat[13] * mat[9] + mat[14] * mat[10]);
	tMat[3] = mat[3]; tMat[7] = mat[7]; tMat[11] = mat[11]; tMat[15] = mat[15];
}