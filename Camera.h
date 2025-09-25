#pragma once

#include <math.h>
#include <stdio.h>

#define RADIAN	(0.017453292519943295769236907684886) //(PI/180)

struct CammeraProperty
{
	float position[3];

	float rotation[3];

	float _matA[16];
	float _matB[16];
	float _matView[16];
	float _matProj[16];
	float _matPlaneModel[16];
	float _matPlaneModelRecon[16];
	float _eye[4] = { 0,0,0,0 };
	float  _at[4] = { 1, 0, 0, 0 };
	float  /*at[3],*/ _up[4] = { 0,1,0,0 };
	float  /*at[3],*/ _rotY[16];
	float  /*at[3],*/ _rotX[16];
	float  /*at[3],*/ _rotZ[16];

	float _Trs[16];

	CammeraProperty(float pos[3], float camrot[3])
	{
		for (int i = 0; i < 3; i++)
		{
			if (i == 2 )
			{
				pos[i] *= -1;
			}
			position[i] = pos[i];
			_eye[i] = pos[i];
			rotation[i] = camrot[i] * RADIAN;
		}

		_SettingCammera();
		_SettingScenePlaneModelMatrix();
	}

	void _SettingScenePlaneModelMatrix()
	{
		float _matTemp[16];
		float _matTempRec[16];
		_matIden(_matTemp);
		_matIden(_matTempRec);
		_matIden(_matPlaneModel);

		_matMult(_matTemp, _rotY, _rotZ);
		_matMult(_matTempRec, _rotY, _rotZ);
		_matMult(_matPlaneModel, _Trs, _matTemp);
	}

	void _SettingCammera()
	{
		float nAt[4] = { 0,0,0,0 };
		float l;
		float fHorRot[4] = { 0,0,0,0 };
		float fVerRot[4] = { 0,0,0,0 };
		float u[3];
		float fRotAngle = 0.0;
		float fRotAngley = 0.0;

		l = sqrt(_at[0] * _at[0] + _at[1] * _at[1] + _at[2] * _at[2]);
		nAt[0] = _at[0] / l; nAt[1] = _at[1] / l; nAt[2] = _at[2] / l;

		
		fRotAngle = rotation[0];
		fRotAngley = -1 * rotation[1];

		_rotY[0] = cos(fRotAngle);  _rotY[4] = 0.0; _rotY[8] = sin(fRotAngle); _rotY[12] = 0.0;
		_rotY[1] = 0.0;          _rotY[5] = 1.0; _rotY[9] = 0.0; _rotY[13] = 0.0;
		_rotY[2] = -sin(fRotAngle); _rotY[6] = 0.0; _rotY[10] = cos(fRotAngle); _rotY[14] = 0.0;
		_rotY[3] = 0.0; _rotY[7] = 0.0; _rotY[11] = 0.0; _rotY[15] = 1.0;

		_rotZ[0] = cos(fRotAngley); _rotZ[4] = -sin(fRotAngley); _rotZ[8] = 0.0;  _rotZ[12] = 0.0;
		_rotZ[1] = sin(fRotAngley); _rotZ[5] = cos(fRotAngley);  _rotZ[9] = 0.0;  _rotZ[13] = 0.0;
		_rotZ[2] = 0.0;             _rotZ[6] = 0.0;              _rotZ[10] = 1.0; _rotZ[14] = 0.0;
		_rotZ[3] = 0.0;             _rotZ[7] = 0.0;              _rotZ[11] = 0.0; _rotZ[15] = 1.0;

		_vecMult(nAt, _rotZ, fVerRot);
		_vecMult(fVerRot, _rotY, fHorRot);

		_at[0] = fHorRot[0] + _eye[0];
		_at[1] = fHorRot[1] + _eye[1];
		_at[2] = fHorRot[2] + _eye[2];

		_Trs[0] = 1.0; _Trs[4] = 0.0; _Trs[8] = 0.0; _Trs[12] = position[0] + fHorRot[0] * 15;
		_Trs[1] = 0.0; _Trs[5] = 1.0; _Trs[9] = 0.0; _Trs[13] = position[1] + fHorRot[1] * 15;
		_Trs[2] = 0.0; _Trs[6] = 0.0; _Trs[10] = 1.0; _Trs[14] = position[2] + fHorRot[2] * 15;
		_Trs[3] = 0.0; _Trs[7] = 0.0; _Trs[11] = 0.0; _Trs[15] = 1.0;

		GetViewMet();
		setProjMat();
	}

	void _vecMult(float b[4], float a[16], float vout[4])
	{
		vout[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
		vout[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
		vout[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
		vout[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

	}
	void _matMult(float c[16], float a[16], float b[16])
	{
		c[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
		c[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
		c[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
		c[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
		//
		c[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
		c[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
		c[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
		c[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
		//
		c[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
		c[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
		c[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
		c[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
		//
		c[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
		c[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
		c[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
		c[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
	}

	void _matCopy(float a[16], float b[16]) { // a = b;
		a[0] = b[0]; a[4] = b[4]; a[8] = b[8]; a[12] = b[12];
		a[1] = b[1]; a[5] = b[5]; a[9] = b[9]; a[13] = b[13];
		a[2] = b[2]; a[6] = b[6]; a[10] = b[10]; a[14] = b[14];
		a[3] = b[3]; a[7] = b[7]; a[11] = b[11]; a[15] = b[15];
	}

	void _matIden(float a[16]) { // a = I
		a[0] = 1.0; a[4] = 0.0; a[8] = 0.0; a[12] = 0.0;
		a[1] = 0.0; a[5] = 1.0; a[9] = 0.0; a[13] = 0.0;
		a[2] = 0.0; a[6] = 0.0; a[10] = 1.0; a[14] = 0.0;
		a[3] = 0.0; a[7] = 0.0; a[11] = 0.0; a[15] = 1.0;
	}
	void GetViewMet()
	{
		float p[3], n[3], v[3], u[3];
		float l;

		p[0] = _eye[0];
		p[1] = _eye[1];
		p[2] = _eye[2];

		n[0] = _at[0] - _eye[0];
		n[1] = _at[1] - _eye[1];
		n[2] = _at[2] - _eye[2];

		//printf("eye %f,", _eye[0]);
		//printf("%f,", _eye[1]);
		//printf("%f\n", _eye[2]);

		//printf("at %f,", _at[0]);
		//printf("%f,", _at[1]);
		//printf("%f\n", _at[2]);


		l = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		n[0] /= l; n[1] /= l; n[2] /= l;

		u[0] = n[1] * _up[2] - n[2] * _up[1];
		u[1] = n[2] * _up[0] - n[0] * _up[2];
		u[2] = n[0] * _up[1] - n[1] * _up[0];

		l = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
		u[0] /= l; u[1] /= l; u[2] /= l;

		//printf("up %f,", u[0]);
		//printf("%f,", u[1]);
		//printf("%f\n", u[2]);

		v[0] = u[1] * n[2] - u[2] * n[1];
		v[1] = u[2] * n[0] - u[0] * n[2];
		v[2] = u[0] * n[1] - u[1] * n[0];

		l = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		v[0] /= l; v[1] /= l; v[2] /= l;
		//
		// view matrix
		_matA[0] = u[0]; _matA[4] = u[1]; _matA[8] = u[2]; _matA[12] = 0;
		_matA[1] = v[0]; _matA[5] = v[1]; _matA[9] = v[2]; _matA[13] = 0;
		_matA[2] = n[0]; _matA[6] = n[1]; _matA[10] = n[2]; _matA[14] = 0;
		_matA[3] = 0.0; _matA[7] = 0.0; _matA[11] = 0.0; _matA[15] = 1.0;
		_matB[0] = 1; _matB[4] = 0; _matB[8] = 0; _matB[12] = -p[0];
		_matB[1] = 0; _matB[5] = 1; _matB[9] = 0; _matB[13] = -p[1];
		_matB[2] = 0; _matB[6] = 0; _matB[10] = 1; _matB[14] = -p[2];
		_matB[3] = 0.0; _matB[7] = 0.0; _matB[11] = 0.0; _matB[15] = 1.0;
		_matMult(_matView, _matA, _matB);

		//for (int i = 0; i < 4; i += 1)
		//{
		//	printf("%f,", _matView[i]);
		//	printf("%f,", _matView[i + 4]);
		//	printf("%f,", _matView[i + 8]);
		//	printf("%f\n", _matView[i + 12]);
		//}

		float _matInTrans[16];
		transpose(_matView, _matInTrans);

		//for (int i = 0; i < 4; i += 1)
		//{
		//	printf("%f,", _matInTrans[i]);
		//	printf("%f,", _matInTrans[i + 4]);
		//	printf("%f,", _matInTrans[i + 8]);
		//	printf("%f\n", _matInTrans[i + 12]);
		//}
	}


	void setProjMat(void) {
		float xmin, xmax;
		float ymin, ymax;
		float zmin, zmax;
		float a, b, c, d, e, f;
		// input
		xmin = -1.0; xmax = +1.0;
		ymin = -1.0; ymax = +1.0;
		zmin = -0.0; zmax = 30.0;
		// projection matrix
		_matProj[0] = 2.0 / (xmax - xmin); _matProj[4] = 0.0; _matProj[8] = 0.0; _matProj[12] = -(xmax + xmin) / 2.0;
		_matProj[1] = 0.0; _matProj[5] = 2.0 / (ymax - ymin); _matProj[9] = 0.0; _matProj[13] = -(ymax + ymin) / 2.0;
		_matProj[2] = 0.0; _matProj[6] = 0.0; _matProj[10] = 2 / (zmax - zmin); _matProj[14] = -(zmax + zmin) / 30.0;
		_matProj[3] = 0.0; _matProj[7] = 0.0; _matProj[11] = 0.0; _matProj[15] = 1.0;
	}
};

CammeraProperty* pCamProp[24];