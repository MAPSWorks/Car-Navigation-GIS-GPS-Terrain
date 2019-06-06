
#include "stdafx.h"
#include "gis.h"
#include "quadtree.h"

using namespace gis;

// ������ ���浵 ����
// index = level
static const double deg[16] = {
	36.f  // 0 level
	, 36.f / 2.f // 1 level
	, 36.f / 2.f / 2.f // 2 level
	, 36.f / 2.f / 2.f / 2.f // 3 level
	, 36.f / 2.f / 2.f / 2.f / 2.f // 4 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f // 5 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 6 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 7 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 8 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 9 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 10 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 11 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 12 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 13 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 14 level
	, 36.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f / 2.f // 15 level
};

static const int g_offsetLv = 7;
static const int g_offsetXLoc = 1088;
static const int g_offsetYLoc = 442;



// return Tile Left-Top WGS84 Location
// xLoc : col, �浵
// yLoc : row, ����
//
// 0 level, row=5, col=10
//	- latitude : 36 degree / row 1
//	- longitude : 36 degrre / col 1
//
// return: .x = longitude (�浵)
//		   .y = latitude (����)
Vector2d gis::TileLoc2WGS84(const int level, const int xLoc, const int yLoc)
{
	Vector2d lonLat;
	lonLat.x = deg[level] * xLoc - 180.f;
	lonLat.y = deg[level] * yLoc - 90.f;
	return lonLat;
}


Vector2d gis::TileLoc2WGS84(const sTileLoc &loc)
{
	return TileLoc2WGS84(loc.level, loc.xLoc, loc.yLoc);
}


sTileLoc gis::WGS842TileLoc(const Vector2d &lonLat)
{
	assert(0);
	return {};
}


// Convert 3D Position to longitude, raditude
// return: .x = longitude (�浵)
//		   .y = latitude (����)
//
// leftBottomLonLat, rightTopLonLat:
//   +--------* (Right Top Longitude Latitude)
//   |        |
//   |        |
//   |        |
//   *--------+ 
//	(Left Bottom Longitude Latitude)
//
// rect: VWorld 3D Coordinate system
//       Ground X-Z Plane Rect
//
//  /\ Z 
//  |
//  | 
//  |
//  ---------> X 
//  Ground = X-Z Plane
//
Vector2d gis::Pos2WGS84(const Vector3 &pos
	, const Vector2d &leftBottomLonLat, const Vector2d &rightTopLonLat
	, const sRectf &rect)
{
	Vector3 p = Vector3(pos.x - rect.left, pos.y, pos.z - rect.top);
	p.x /= rect.Width();
	p.z /= rect.Height();

	Vector2d ret;
	ret.x = common::lerp(leftBottomLonLat.x, rightTopLonLat.x, (double)p.x);
	ret.y = common::lerp(leftBottomLonLat.y, rightTopLonLat.y, (double)p.z);
	return ret;
}


// Convert longitude, latitude to 3D Position (Global Position)
Vector3 gis::WGS842Pos(const Vector2d &lonLat)
{
	const double size = 1 << 16;
	const double x = (lonLat.x + 180.f) * (size / 36.f);
	const double z = (lonLat.y + 90.f) * (size / 36.f);
	return Vector3((float)x, 0, (float)z);
}


Vector3 gis::GetRelationPos(const Vector3 &globalPos)
{
	const float size = (1 << (16 - g_offsetLv));
	const int xLoc = g_offsetXLoc;
	const int yLoc = g_offsetYLoc;
	const float offsetX = size * xLoc;
	const float offsetY = size * yLoc;
	return Vector3(globalPos.x - offsetX, globalPos.y, globalPos.z - offsetY);
}


// meter -> 3D unit
// ���� ���� �ѷ�, 40030000.f,  40,030 km
// �浵������ 10��� �� ũ�⸦ (1 << cQuadTree<int>::MAX_LEVEL) �� 3D�� ����Ѵ�.
float gis::Meter23DUnit(const float meter)
{
	static const float rate = (float)(1 << cQuadTree<int>::MAX_LEVEL) / (40030000.f * 0.1f);
	return meter * rate;
}


// GPRMC ���ڿ��� ���浵�� �����Ѵ�.
//$GPRMC, 103906.0, A, 3737.084380, N, 12650.121679, E, 0.0, 0.0, 190519, 6.1, W, A * 15
// lat: 37 + 37.084380/60
// lon: 126 + 50.121679/60
// return : x=longitude, y=latitude
Vector2d gis::GetGPRMCLonLat(const Str512 &gprmc)
{
	if (gprmc.size() < 6)
		return Vector2d(0, 0);
	
	if (strncmp(gprmc.m_str, "$GPRMC", 6))
		return Vector2d(0, 0);

	vector<string> strs;
	common::tokenizer(gprmc.c_str(), ",", "", strs);

	if (strs.size() < 6)
		return Vector2d(0, 0);

	if (strs[4] != "N")
		return Vector2d(0, 0);
	
	if (strs[6] != "E")
		return Vector2d(0, 0);

	if (strs[3].size() < 2)
		return Vector2d(0, 0);

	Vector2d reval;
	const float lat1 = (float)atof(strs[3].substr(0, 2).c_str());
	const float lat2 = (float)atof(strs[3].substr(2).c_str());
	if (lat2 == 0.f)
		return Vector2d(0, 0);
	reval.y = lat1 + (lat2 / 60.f);

	if (strs[5].size() < 3)
		return Vector2d(0, 0);
	const float lon1 = (float)atof(strs[5].substr(0, 3).c_str());
	const float lon2 = (float)atof(strs[5].substr(3).c_str());
	if (lon2 == 0.f)
		return Vector2d(0, 0);
	reval.x = lon1 + (lon2 / 60.f);

	return reval;
}