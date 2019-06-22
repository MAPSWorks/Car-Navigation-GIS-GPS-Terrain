//
// 2019-05-26, jjuiddong
// global variable
//
#pragma once

enum class eAnalysisType : int 
{
	MapView
	, Terrain
	, GMain
};

#include "touch.h"


class cGlobal
{
public:
	cGlobal();
	virtual ~cGlobal();


public:
	cTouch m_touch;
	cGpsClient m_gpsClient;

	// GPS
	bool m_isShowGPS;
	bool m_isTraceGPSPos;

	// Render Overhead
	bool m_isShowTerrain;
	bool m_isShowRenderGraph;
	bool m_isCalcRenderGraph;

	// map scanning
	bool m_isMapScanning; // ī�޶� ������ ��η� ������ �� true
	bool m_isSelectMapScanningCenter; // ��ĳ�� �� ������ �����Ѵ�.
	Vector2d m_scanCenter;
	Vector3 m_scanCenterPos;
	Vector3 m_scanPos;
	Vector3 m_scanDir;
	float m_scanRadius;
	float m_scanHeight;
	float m_scanSpeed; // m/s

	// analysis
	eAnalysisType m_analysisType; //0:mapview, 1:terrain renderer
	double m_renderT0;
	double m_renderT1;
	double m_renderT2;
	bool m_isShowMapView;

	cTimer m_timer;
};
