
#include "stdafx.h"
#include "global.h"


cGlobal::cGlobal()
	: m_isMapScanning(false)
	, m_isSelectMapScanningCenter(false)
	, m_isShowGPS(true)
	, m_isTraceGPSPos(true)
	, m_isShowTerrain(true)
	, m_isShowRenderGraph(false)
	, m_isCalcRenderGraph(false)
	, m_analysisType(eAnalysisType::MapView)
	, m_renderT0(0.f)
	, m_renderT1(0.f)
	, m_renderT2(0.f)
	, m_isShowMapView(true)
	, m_scanRadius(1.f)
	, m_scanHeight(10.f)
	, m_scanSpeed(1.f)
	, m_isMakeTracePath(false)
	, m_isShowPrevPath(false)
	, m_isShowLandMark(true)
	, m_isShowLandMark2(true)
	, m_landMarkSelectState(0)
	, m_landMarkSelectState2(0)
{
}

cGlobal::~cGlobal()
{
	m_config.Write("carnavi_config.txt");
	m_landMark.Write("landmark.txt");
	Clear();
}


bool cGlobal::Init(HWND hwnd)
{
	m_config.Read("carnavi_config.txt");

	m_timer.Create();
	m_touch.Init(hwnd);
	m_gpsClient.Init();
	m_landMark.Read("landmark.txt");

	return true;
}


// pathDirectoryName ���� �ִ� *.txt ���� (path ����)�� �о
// ȭ�鿡 ǥ���Ѵ�. ���� ������ ��� *.3dpos ���Ϸ� ��ȯ�ؼ� �����Ѵ�.
bool cGlobal::ReadPathFiles(graphic::cRenderer &renderer
	, cTerrainQuadTree &terrain
	, const StrPath pathDirectoryName)
{
	list<string> files;
	list<string> exts;
	exts.push_back(".txt");
	common::CollectFiles(exts, pathDirectoryName.c_str(), files);

	for (auto &file : files)
	{
		cPath path(file);
		if (!path.IsLoad())
			continue;

		StrPath pos3DFileName = pathDirectoryName;
		pos3DFileName += StrPath(file).GetFileNameExceptExt();
		pos3DFileName += ".3dpos";

		cPathRenderer *p = new cPathRenderer();
		if (!p->Create(renderer, terrain, path, pos3DFileName))
		{
			delete p;
			continue;
		}

		terrain.Clear();
		m_pathRenderers.push_back(p);
	}

	return true;
}


// pathDirectoryName ���� �ִ� *.3dpos ���� (path ����)�� �о
// ȭ�鿡 ǥ���Ѵ�. 
bool cGlobal::Read3DPosFiles(graphic::cRenderer &renderer, const StrPath pathDirectoryName)
{
	list<string> files;
	list<string> exts;
	exts.push_back(".3dpos");
	common::CollectFiles(exts, pathDirectoryName.c_str(), files);

	for (auto &file : files)
	{
		cPathRenderer *p = new cPathRenderer();
		if (!p->Create(renderer, file))
		{
			delete p;
			continue;
		}

		m_pathRenderers.push_back(p);
	}

	return true;
}


// pathDirectoryName ���� ���� *.txt path������ �о�´�.
// path���Ͽ� �ش��ϴ� *.3dpos ������ �ִٸ�, �ش� ������ �о� �´�.
// path���� - 3dpos ������ ���ϸ��� ����, Ȯ���ڸ� �ٸ���.
// path���� �߿� 3dpos ���Ϸ� ��ȯ���� �ʴ� ������ ��ȯ��Ų��.
bool cGlobal::ReadAndConvertPathFiles(graphic::cRenderer &renderer, cTerrainQuadTree &terrain
	, const StrPath pathDirectoryName)
{
	list<string> files;
	list<string> exts;
	exts.push_back(".txt");
	common::CollectFiles(exts, pathDirectoryName.c_str(), files);

	for (auto &file : files)
	{
		StrPath pos3DFileName = StrPath(file).GetFileNameExceptExt2();
		pos3DFileName += ".3dpos";

		auto it = std::find_if(m_pathRenderers.begin(), m_pathRenderers.end()
			, [&](const auto &a) { return a->m_fileName == pos3DFileName; });
		if (m_pathRenderers.end() != it)
			continue; // already loaded

		cPathRenderer *p = NULL;
		if (pos3DFileName.IsFileExist())
		{
			p = new cPathRenderer();
			if (!p->Create(renderer, pos3DFileName))
			{
				delete p;
				continue;
			}
		}
		else
		{
			cPath path(file);
			if (!path.IsLoad())
				continue;

			p = new cPathRenderer();
			if (!p->Create(renderer, terrain, path, pos3DFileName))
			{
				delete p;
				continue;
			}
		}

		terrain.Clear();
		m_pathRenderers.push_back(p);
	}
	return true;
}


void cGlobal::Clear()
{
	for (auto &p : m_pathRenderers)
		delete p;
	m_pathRenderers.clear();
}
