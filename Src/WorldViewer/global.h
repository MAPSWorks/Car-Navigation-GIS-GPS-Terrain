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
	Str16 m_gpsIp = "192.168.1.102";
	int m_gpsPort = 60660;
	network2::cTcpClient m_gpsClient;

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
