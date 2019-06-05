
#include "stdafx.h"
#include "vworldwebdownloader.h"
#include "quadtilemanager.h"

using namespace gis;


cVWorldWebDownloader::cVWorldWebDownloader()
	: m_isOfflineMode(false)
{
	m_tpDownloader.Init(5); // 5 thread
}

cVWorldWebDownloader::~cVWorldWebDownloader()
{
	Clear();
}


bool cVWorldWebDownloader::DownloadFile(const int level, const int xLoc, const int yLoc
	, const int idx
	, const eLayerName::Enum type
	, cQuadTileManager &tileMgr
	, iDownloadFinishListener *listener
	, const char *dataFileName //= NULL
)
{
	if (m_isOfflineMode)
		return false;

	sDownloadData data;
	ZeroMemory(&data, sizeof(data));
	data.level = level;
	data.xLoc = xLoc;
	data.yLoc = yLoc;
	data.idx = idx;
	data.layer = type;
	data.dataFile = dataFileName;
	data.listener = listener;

	{
		AutoCSLock cs(m_cs);

		m_listeners.insert(listener);

		const auto key = GetKey(data);
		if (m_requestIds.end() != m_requestIds.find(key))
			return false; //already requested

		m_requestIds.insert(key);
	}

	cTaskWebDownload *taskWebDownload = new cTaskWebDownload();
	taskWebDownload->SetParameter(this, &tileMgr, data);
	m_tpDownloader.PushTask(taskWebDownload);

	return true;
}


// �ٿ�ε� ���� ������, �����ʿ��� �˸���.
void cVWorldWebDownloader::UpdateDownload()
{
	AutoCSLock cs(m_cs);

	for (auto &file : m_complete)
	{
		// ��� �����ʿ��� �˸���.
		for (auto &listener : m_listeners)
			listener->OnDownloadComplete(file);

		const auto key = GetKey(file);
		m_requestIds.erase(key);
	}

	m_complete.clear();
}


bool cVWorldWebDownloader::Insert(const sDownloadData &dnData)
{
	AutoCSLock cs(m_cs);
	m_complete.push_back(dnData);
	return true;
}


bool cVWorldWebDownloader::Remove(const sDownloadData &dnData)
{
	AutoCSLock cs(m_cs);
	const auto key = GetKey(dnData);
	m_requestIds.erase(key);
	return true;
}


std::tuple<int, int, __int64> cVWorldWebDownloader::GetKey(const sDownloadData &dnData)
{
	const int key1 = (dnData.dataFile.empty()) ? 0 : dnData.dataFile.GetHashCode();
	const int key2 = ((int)dnData.layer * 100 + dnData.idx);
	const __int64 key3 = cQuadTree<sQuadData>::MakeKey(dnData.level, dnData.xLoc, dnData.yLoc);
	const auto key = std::make_tuple(key1, key2, key3);
	return key;
}


void cVWorldWebDownloader::Clear()
{
	{
		AutoCSLock cs(m_cs);
		m_complete.clear();
		m_requestIds.clear();
	}

	m_tpDownloader.Clear();
	common::cMemoryPool4<cTaskWebDownload>::Clear();
}
