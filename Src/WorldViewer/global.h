//
// 2019-05-26, jjuiddong
// global variable
//
#pragma once


class cGlobal
{
public:
	cGlobal();
	virtual ~cGlobal();


public:
	cGpsClient m_gpsClient;

	// map scanning
	bool m_isMapScanning; // ī�޶� ������ ��η� ������ �� true
	bool m_isMoveRight;
	bool m_pickScanLeftTop;
	bool m_pickScanRightBottom;
	Vector2d m_scanLeftTop;
	Vector2d m_scanRightBottom;
	float m_scanSpeed; // m/s
	float m_scanLineOffset; // meter

	cTimer m_timer;
};
