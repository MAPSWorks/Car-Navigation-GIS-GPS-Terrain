//
// Navigation Server
// CarNavi�κ��� GPS������ �޾Ƽ�, ���Ͽ� �����Ѵ�.
//
#include "stdafx.h"
#include "naviserver.h"


using namespace std;

bool g_isLoop = true;
string g_configFileName = "carnavi_config.txt"; // ȯ�溯�� ���ϸ�

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	g_isLoop = false;
	return TRUE;
}

int MainThreadFunction()
{
	common::cConfig config(g_configFileName);
	dbg::Logc(1, "Start Navigation Server... \n");

	network2::cNetController netController;
	cNaviServer naviSvr;
	naviSvr.Init(netController);

	cout << "==== Start Navigation Server ====\n";
	dbg::Logc(1, "==== Start Navigation Server ====\n");

	common::cTimer timer;
	timer.Create();

	double oldT = timer.GetSeconds();
	while (g_isLoop)
	{
		const double curT = timer.GetSeconds();
		const float dt = (float)(curT - oldT);
		oldT = curT;

		netController.Process(dt);
		naviSvr.Update(dt);

		Sleep(1);
	}

	netController.Clear();
	naviSvr.Clear();
	network2::DisplayPacketCleanup();

	return 1;
}


int main(int argc, char *argv[])
{
	// ���� ���� ������ ȯ������ �̸��� �޾ƿ´�.
	if (argc > 1)
		g_configFileName = argv[1];

	common::dbg::SetLogPath("./naviserver_"); // naviserver_log.txt
	common::dbg::SetErrLogPath("./naviserver_"); // naviserver_errlogt.txt
	common::dbg::RemoveLog();
	common::dbg::RemoveErrLog();

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		cout << "SetConsoleCtrlHandler failed, code : " << GetLastError() << endl;
		return -1;
	}

	std::thread thread = std::thread(MainThreadFunction);
	thread.join();

	return 1;
}
