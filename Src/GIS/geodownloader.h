//
// 2018-05-01, jjuiddong
// geometry data download from geo server *.bil, *.dds, *.dat, .. etc
//
// 2019-05-18
//	- download using thread pool
//
#pragma once


class cQuadTileManager;

namespace gis
{

	class cGeoDownloader
	{
	public:
		cGeoDownloader();
		virtual ~cGeoDownloader();

		bool DownloadFile(const int level, const int xLoc, const int yLoc
			, const int idx
			, const eLayerName::Enum type
			, cQuadTileManager &tileMgr
			, iDownloadFinishListener *listener
			, const char *dataFileName = NULL
		);

		void UpdateDownload();
		bool Insert(const sDownloadData &dnData);
		bool Remove(const sDownloadData &dnData);
		void Clear();


	protected:
		std::tuple<int, int, __int64> GetKey(const sDownloadData &dnData);


	public:
		bool m_isOfflineMode;
		CriticalSection m_cs;
		vector<sDownloadData> m_complete;
		set<iDownloadFinishListener*> m_listeners;

		// 중복 요청 체크, filename + (id + layer * 100) + cQuadTree::MakeKey()
		set<std::tuple<int, int, __int64>> m_requestIds;
		__int64 m_totalDownloadFileSize;
		common::cTPSemaphore m_tpDownloader;
	};

}
