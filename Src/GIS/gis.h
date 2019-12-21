//
// 2018-04-26, jjuiddong
// GIS 관련된 정의 모음
//
// world 3D coordinate system
//
//  /\ Z 
//  |
//  | 
//  |
//  ---------> X 
//  Ground = X-Z Plane
//
//
#pragma once


#include "../../../Common/Common/common.h"
using namespace common;
#include "../../../Common/Graphic11/graphic11.h"

#include "CFW1StateSaver.h"


namespace gis
{
	// using fo x=longitude, y=latitude
	//struct Vector2d
	//{
	//	double x, y;

	//	Vector2d() : x(0), y(0) {}
	//	Vector2d(const double x0, const double y0) :x(x0), y(y0) {}

	//	bool operator==(const Vector2d &rhs) {
	//		return (x == rhs.x) && (y == rhs.y);
	//	}
	//	bool operator!=(const Vector2d &rhs) {
	//		return !operator==(rhs);
	//	}
	//};

	struct Vector3d
	{
		double x, y, z;

		Vector3d() : x(0), y(0), z(0) {}
		Vector3d(const double x0, const double y0, const double z0) :x(x0), y(y0), z(z0) {}
	};

	// aabbox3dd
	struct sAABBox
	{
		Vector3d _min;
		Vector3d _max;
	};

	struct sTileLoc
	{
		int level;
		int xLoc;
		int yLoc;
	};


	// world tile
	//
	//  /\ tileLoc Y (row)
	//  |
	//  | 
	//  |
	//  ---------> tileLoc X (col)
	// 
	struct sTile
	{
		// 2 ---- 3
		// |      |
		// |      |
		// 0 ---- 1
		Vector2d lonLat[4];
		sTileLoc loc;
	};


	// Transform Coordinate System 
	Vector2d TileLoc2WGS84(const int level, const int xLoc, const int yLoc);
	Vector2d TileLoc2WGS84(const sTileLoc &loc);
	sTileLoc WGS842TileLoc(const Vector2d &lonLat);
	Vector2d Pos2WGS84(const Vector3 &pos
		, const Vector2d &leftBottomLonLat, const Vector2d &rightTopLonLat
		, const sRectf &rect);
	Vector3 WGS842Pos(const Vector2d &lonLat);
	Vector3 GetRelationPos(const Vector3 &globalPos);

	// meter -> 3D unit
	float Meter23DUnit(const float meter);

	// GPRMC, Recommended Minimmum data
	struct sGPRMC {
		uint64 date; // yyyymmddhhmmssmmm
		bool available;
		Vector2d lonLat;
		float speed;
		float angle;
		float altitude;
		float north;
	};

	bool GetGPRMCLonLat(const Str512 &gprmc, OUT sGPRMC &out);
	bool GetGPATTLonLat(const Str512 &gpatt, OUT sGPRMC &out);
	bool Check6Val(const double val);
}


#include "UTM.h"
#include "webdownload.h"
#include "taskwebdownload.h"
#include "geodownloader.h"
#include "quadtree.h"
#include "heightmap.h"
#include "heightmap2.h"
#include "tiletexture.h"
#include "poireader.h"
#include "real3dmodelindexreader.h"
#include "xdoreader.h"
#include "quadtile.h"
#include "quadtilemanager.h"
#include "terrainquadtree.h"
#include "triangulate.h"
#include "route.h"
#include "root.h"



extern StrPath g_mediaDir;

