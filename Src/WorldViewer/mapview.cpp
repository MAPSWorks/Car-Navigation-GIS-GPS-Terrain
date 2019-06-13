
#include "stdafx.h"
#include "mapview.h"

using namespace graphic;
using namespace framework;


cMapView::cMapView(const string &name) 
	: framework::cDockWindow(name)
	, m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_groundPlane2(Vector3(0, -1, 0), 0)
	, m_showGround(false)
	, m_showWireframe(false)
	, m_lookAtDistance(0)
	, m_lookAtYVector(0)
{
}

cMapView::~cMapView() 
{
	m_quadTree.Clear();
}


bool cMapView::Init(cRenderer &renderer) 
{
	const Vector3 eyePos(331843.563f, 478027.719f, 55058.8672f);
	const Vector3 lookAt(331004.031f, 0.000000000f, 157745.281f);
	m_camera.SetCamera(eyePos, lookAt, Vector3(0, 1, 0));
	m_camera.SetProjection(MATH_PI / 4.f, m_rect.Width() / m_rect.Height(), 1.f, 1000000.f);
	m_camera.SetViewPort(m_rect.Width(), m_rect.Height());

	sf::Vector2u size((u_int)m_rect.Width() - 15, (u_int)m_rect.Height() - 50);
	cViewport vp = renderer.m_viewPort;
	vp.m_vp.Width = (float)size.x;
	vp.m_vp.Height = (float)size.y;
	m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true
		, DXGI_FORMAT_D24_UNORM_S8_UINT);

	m_ground.Create(renderer, 100, 100, 10, 10);

	if (!m_quadTree.Create(renderer, true))
		return false;

	m_quadTree.m_isShowDistribute = false;
	m_quadTree.m_isShowQuadTree = false;
	m_quadTree.m_isShowFacility = false;
	m_quadTree.m_isShowPoi1 = true;
	m_quadTree.m_isShowPoi2 = false;

	m_skybox.Create(renderer, "../media/skybox/sky.dds");
	m_skybox.m_isDepthNone = true;

	m_curPosObj.Create(renderer, 1.f, 10, 10
		, (eVertexType::POSITION | eVertexType::NORMAL), cColor::RED);

	return true;
}


void cMapView::OnUpdate(const float deltaSeconds)
{
	// ī�޶� ����Ű�� ������ �������� ���Ѵ�.
	const Vector2d camLonLat = m_quadTree.GetLongLat(m_camera.GetRay());

	m_quadTree.Update(GetRenderer(), camLonLat, deltaSeconds);
	m_camera.Update(deltaSeconds);

	UpdateGPS();
	UpdateMapScanning(deltaSeconds);
}


// GPS ������ ���� ��ġ������ �޾� ������Ʈ �Ѵ�.
// receive from gps server (mobile phone)
void cMapView::UpdateGPS()
{
	const float MIN_LENGTH = 0.5f;

	gis::sGPRMC gpsInfo;
	if (!g_global->m_gpsClient.GetGpsInfo(gpsInfo))
		return;

	m_gpsInfo = gpsInfo;
	m_curGpsPos = gpsInfo.lonLat;

	static Vector2d oldGpsPos;
	static Vector3 oldEyePos;
	if (oldGpsPos.IsEmpty())
		oldGpsPos = m_curGpsPos;

	if (m_lookAtDistance == 0)
	{
		m_lookAtDistance = m_camera.GetEyePos().Distance(m_camera.GetLookAt());
		m_lookAtYVector = m_camera.GetDirection().y;
	}

	// path �α� ���Ͽ� ����
	if (g_global->m_gpsClient.IsServer())
	{
		const string date = common::GetCurrentDateTime();
		std::ofstream ofs("path.txt", std::ios::app);
		ofs.precision(std::numeric_limits<double>::max_digits10);
		ofs << date << ", " << m_curGpsPos.x << ", " << m_curGpsPos.y << std::endl;
	}

	// ���� ��ġ�� ���� ī�޶� looAt�� �����Ѵ�.
	// lookAt�� �ٲ� ���� ī�޶� ��ġ�� ���� �Ÿ��� �����ϸ� �̵��Ѵ�.
	g_root.m_lonLat = Vector2((float)m_curGpsPos.x, (float)m_curGpsPos.y);
	const Vector3 pos = m_quadTree.Get3DPos(m_curGpsPos);
	const Vector3 oldPos = m_quadTree.Get3DPos(oldGpsPos);

	const bool isTraceGPSPos = g_global->m_isTraceGPSPos
		&& !m_mouseDown[0] && !m_mouseDown[1] && IsTouchWindow(m_owner->getSystemHandle(), NULL);

	// ī�޶� ������ �ٲ۴�. �̵��ϴ� �������� ���ϰ� �Ѵ�.
	// �ֱ� �̵� �������� n���� ���͸� ����ؼ� ���� ������ �����Ѵ�. (����ġ ���)
	Vector3 avrDir;
	{
		// �ֱ� n1�� 50% ����ġ
		// ������ n2�� 50% ����ġ
		const int n1 = 10;
		const int n2 = 50;
		const float r1 = 50.f / (float)n1;
		const float r2 = 50.f / (float)n2;

		int cnt = 0;
		auto &track = g_global->m_gpsClient.m_paths;
		for (int i = (int)track.size() - 1; (i >= 1) && (cnt < (n1 + n2)); --i, ++cnt)
		{
			const Vector2d &ll0 = track[i - 1].lonLat;
			const Vector2d &ll1 = track[i].lonLat;
			const Vector3 p0 = m_quadTree.Get3DPos(ll0);
			const Vector3 p1 = m_quadTree.Get3DPos(ll1);
			Vector3 d = p1 - p0;
			d.y = 0.f;
			d.Normalize();
			if (cnt < n1)
				avrDir += d * r1;
			else
				avrDir += d * r2;
		}
		avrDir.Normalize();
		m_avrDir = avrDir;
	}

	if (oldPos.Distance(pos) > MIN_LENGTH)
	{
		Vector3 dir = avrDir;
		dir.y = m_lookAtYVector;
		const Vector3 newEyePos = pos + dir * -m_lookAtDistance;

		// ����ó �Է� �ÿ��� ī�޶� �ڵ����� �������� �ʴ´�.
		if (isTraceGPSPos)
			m_camera.Move(newEyePos, pos, 30.0f);

		oldGpsPos = m_curGpsPos;
		oldEyePos = newEyePos;
	}
	else if (isTraceGPSPos)
	{
		m_camera.Move(oldEyePos, pos, 30.0f);
	}

	// �̵����� ����
	// ���� ��ġ�� ���� ���ٸ� �������� �ʴ´�.
	{
		auto &track = g_global->m_gpsClient.m_paths;
		const Vector3 lastPos = track.empty() ? Vector3::Zeroes : m_quadTree.Get3DPos(track.back().lonLat);

		if (track.empty()
			|| (!track.empty() && (lastPos.Distance(pos) > MIN_LENGTH)))
			track.push_back({ common::GetCurrentDateTime3(), gpsInfo.lonLat });
	}
}


// ī�޶� �̵��ϸ鼭 ������ �ٿ�ε� �޴´�.
void cMapView::UpdateMapScanning(const float deltaSeconds)
{
	if (!g_global->m_isMapScanning)
		return;

	// ī�޶� ��ġ�� ���浵�� �ľ��ϰ�, Ư�� �����ȿ��� �����̵��� �Ѵ�.
	
	// ��ĵ�� ���� ���浵 ��ǥ
	const Vector2d leftTop = g_global->m_scanLeftTop;
	const Vector2d rightBottom = g_global->m_scanRightBottom;
	const Vector3 lookAt = m_camera.GetLookAt();
	const Vector2d longLat = m_quadTree.GetWGS84(lookAt);
	const float speed = g_global->m_scanSpeed;
	const float lineOffset = g_global->m_scanLineOffset;

	// check horizontal moving
	if (g_global->m_isMoveRight) // right move
	{
		if (longLat.x > rightBottom.x)
		{
			g_global->m_isMoveRight = false;
			m_camera.MoveCancel();

			const float len = m_camera.GetLookAt().Distance(m_camera.GetEyePos());
			const Vector3 lt = m_quadTree.Get3DPos(leftTop - Vector2d(1.f, 0));
			const Vector3 target(lt.x, 0, lookAt.z - lineOffset);
			const Vector3 eyePos = target + Vector3(0, m_camera.GetEyePos().y, 0);
			m_camera.Move(eyePos, target, speed);
		}
	}
	else // left move
	{
		if (longLat.x < leftTop.x)
		{
			g_global->m_isMoveRight = true;
			m_camera.MoveCancel();

			const float len = m_camera.GetLookAt().Distance(m_camera.GetEyePos());
			const Vector3 rb = m_quadTree.Get3DPos(rightBottom + Vector2d(1.f,0));
			const Vector3 target(rb.x, 0, lookAt.z - lineOffset);
			const Vector3 eyePos = target + Vector3(0, m_camera.GetEyePos().y, 0);
			m_camera.Move(eyePos, target, speed);
		}
	}

	// check vertical moving
	if (longLat.y < rightBottom.y)
	{
		m_camera.MoveCancel();
		g_global->m_isMapScanning = false;
	}
}


void cMapView::OnPreRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();
	cAutoCam cam(&m_camera);

	renderer.UnbindTextureAll();

	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);

	if (m_renderTarget.Begin(renderer))
	{
		CommonStates states(renderer.GetDevice());

		if (m_showWireframe)
		{
			renderer.GetDevContext()->RSSetState(states.Wireframe());
		}
		else
		{
			renderer.GetDevContext()->RSSetState(states.CullCounterClockwise());
			m_skybox.Render(renderer);
		}

		if (m_showGround)
			m_ground.Render(renderer);

		const Ray ray1 = GetMainCamera().GetRay();
		const Ray ray2 = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);

		cFrustum frustum;
		frustum.SetFrustum(GetMainCamera().GetViewProjectionMatrix());
		m_quadTree.Render(renderer, deltaSeconds, frustum, ray1, ray2);

		// latLong position
		{
			const Vector3 p0 = m_quadTree.Get3DPos({ (double)g_root.m_lonLat.x, (double)g_root.m_lonLat.y });
			renderer.m_dbgLine.SetColor(cColor::WHITE);
			renderer.m_dbgLine.SetLine(p0, p0 + Vector3(0, 1, 0), 0.01f);
			renderer.m_dbgLine.Render(renderer);

			renderer.m_dbgArrow.SetColor(cColor::WHITE);
			renderer.m_dbgArrow.SetDirection(p0 + Vector3(0, 1, 0)
				, p0 + Vector3(0, 1, 0) + m_avrDir * 2.f
				, 0.02f);
			renderer.m_dbgArrow.Render(renderer);


			// ī�޶���� �Ÿ��� ���� ũ�⸦ �����Ѵ�. (�׻� ���� ũ��� ���̱� ����)
			m_curPosObj.m_transform.pos = p0 + Vector3(0, 1, 0);
			const float dist = GetMainCamera().GetEyePos().Distance(p0);
			const float scale = common::clamp(0.2f, 1000.f, (dist * 1.5f) / 140.f);
			m_curPosObj.m_transform.scale = Vector3::Ones * scale;
			m_curPosObj.Render(renderer);
		}

		// render autopilot route
		if (!g_root.m_route.empty())
		{
			renderer.m_dbgLine.SetColor(cColor::WHITE);
			for (int i=0; i < (int)g_root.m_route.size()-1; ++i)
			{
				auto &lonLat0 = g_root.m_route[i];
				auto &lonLat1 = g_root.m_route[i+1];

				const Vector3 p0 = gis::GetRelationPos(gis::WGS842Pos(lonLat0));
				const Vector3 p1 = gis::GetRelationPos(gis::WGS842Pos(lonLat1));
				renderer.m_dbgLine.SetLine(p0+ Vector3(0, 20, 0), p1 + Vector3(0, 20, 0), 0.03f);
				renderer.m_dbgLine.Render(renderer);
			}
		}

		// render track pos
		auto &track = g_global->m_gpsClient.m_paths;
		if (!track.empty())
		{
			renderer.GetDevContext()->OMSetDepthStencilState(states.DepthNone(), 0);

			Vector3 prevPos;
			const Vector3 camPos = GetMainCamera().GetEyePos();
			renderer.m_dbgLine.SetColor(cColor::RED);
			for (int i = 0; i < (int)track.size() - 1; ++i)
			{
				const Vector2d &ll0 = track[i].lonLat;
				const Vector2d &ll1 = track[i + 1].lonLat;
				const Vector3 p0 = m_quadTree.Get3DPos(ll0);
				const Vector3 p1 = m_quadTree.Get3DPos(ll1);
				if (p0.Distance(p1) > 100.f)
					continue;

				if (prevPos.IsEmpty())
					prevPos = p0;

				const float len = prevPos.Distance(p1);
				const float camLen = camPos.Distance(p1);
				if (camLen == 0.f)
					continue;

				// ī�޶���� �Ÿ����, ������ ���� ��ġ�� ������� �ʴ´�.
				// ������ 20���� ��ġ�� ����Ѵ�.
				if ((len / camLen < 0.05f) 
					&& (i < (int)(track.size() - 20)))
					continue;

				renderer.m_dbgLine.SetLine(prevPos, p1, 0.03f);
				renderer.m_dbgLine.Render(renderer);
				prevPos = p1;
			}

			renderer.GetDevContext()->OMSetDepthStencilState(states.DepthDefault(), 0);

		} // ~if trackpos

	}
	m_renderTarget.End(renderer);
}


void cMapView::OnRender(const float deltaSeconds) 
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_viewPos = { (int)(pos.x), (int)(pos.y) };
	m_viewRect = { pos.x + 5, pos.y, pos.x + m_rect.Width() - 30, pos.y + m_rect.Height() - 42 };
	ImGui::Image(m_renderTarget.m_resolvedSRV, ImVec2(m_rect.Width() - 15, m_rect.Height() - 42));

	// HUD
	const float windowAlpha = 0.0f;
	bool isOpen = true;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowBgAlpha(windowAlpha);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::SetNextWindowSize(ImVec2(min(m_viewRect.Width(), 500), 250));
	if (ImGui::Begin("Information", &isOpen, ImVec2(500.f, 250.f), windowAlpha, flags))
	{
		// render datetime
		{
			static float incT = 0;
			static ImVec4 color(0, 0, 0, 1);
			incT += deltaSeconds;
			if (incT > 0.5f)
			{
				color = (color.x == 0.f) ? ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1);
				incT = 0.f;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::PushFont(m_owner->m_fontBig);
			const string dateTime = common::GetCurrentDateTime5();
			ImGui::Text("%s, Speed: %.1f", dateTime.c_str(), m_gpsInfo.speed);

			ImGui::PopStyleColor();
			ImGui::PopFont();
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
		ImGui::Text(IsTouchWindow(m_owner->getSystemHandle(), NULL) ? "Touch" : "Gesture");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::Text("GPS = %.6f, %.6f", m_curGpsPos.y, m_curGpsPos.x);

		if (g_global->m_isShowGPS)
			ImGui::Text(g_global->m_gpsClient.m_recvStr.c_str());
		ImGui::End();
	}
	ImGui::PopStyleColor();
}


void cMapView::OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect) 
{
	if (type == eDockResize::DOCK_WINDOW)
	{
		m_owner->RequestResetDeviceNextFrame();
	}
}


void cMapView::UpdateLookAt()
{
	GetMainCamera().MoveCancel();

	const float centerX = GetMainCamera().m_width / 2;
	const float centerY = GetMainCamera().m_height / 2;
	const Ray ray = GetMainCamera().GetRay((int)centerX, (int)centerY);
	const float distance = m_groundPlane1.Collision(ray.dir);
	if (distance < -0.2f)
	{
		GetMainCamera().m_lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	}
	else
	{ // horizontal viewing
		const Vector3 lookAt = GetMainCamera().m_eyePos + GetMainCamera().GetDirection() * 50.f;
		GetMainCamera().m_lookAt = lookAt;
	}

	GetMainCamera().UpdateViewMatrix();
}


// ���� �������� ��,
// ī�޶� �տ� �ڽ��� �ִٸ�, �ڽ� ���鿡�� �����.
void cMapView::OnWheelMove(const float delta, const POINT mousePt)
{
	UpdateLookAt();

	float len = 0;
	const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
	Vector3 lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	len = (ray.orig - lookAt).Length();

	const int lv = m_quadTree.GetLevel(len);
	float zoomLen = min(len * 0.1f, (float)(2 << (16-lv)));

	if (m_isGestureInput 
		&& !IsTouchWindow(m_owner->getSystemHandle(), NULL))
	{
		zoomLen *= 5.f;
	}

	GetMainCamera().Zoom(ray.dir, (delta < 0) ? -zoomLen : zoomLen);

	// �������� �Ʒ��� ī�޶� ��ġ�ϸ�, ���̸� �����Ѵ�.
	const Vector3 eyePos = m_camera.GetEyePos();
	const float h = m_quadTree.GetHeight(Vector2(eyePos.x, eyePos.z));
	if (eyePos.y < (h + 2))
	{
		const Vector3 newPos(eyePos.x, h + 2, eyePos.z);
		m_camera.SetEyePos(newPos);
	}

	// �̵� ��ο��� �Ÿ��� ������Ʈ �Ѵ�.
	// �� �Ÿ��� �����ϸ鼭 GPS ��ǥ�� �����Ѵ�.
	auto &track = g_global->m_gpsClient.m_paths;
	if (g_global->m_isTraceGPSPos && !track.empty())
	{
		const Vector3 p0 = m_quadTree.Get3DPos(track.back().lonLat);
		m_lookAtDistance = m_camera.GetEyePos().Distance(p0);
		m_lookAtYVector = m_camera.GetDirection().y;
	}
}


// Gesture input
void cMapView::OnGestured(const int id, const POINT mousePt)
{
	static bool isGesturePanInput = false;

	switch (id)
	{
	case GID_BEGIN:
		m_isGestureInput = true;
		break;

	case GID_END:
		isGesturePanInput = false;
		m_isGestureInput = true;
		m_mouseDown[1] = false;
		break;

	case GID_PAN:
		if (isGesturePanInput)
		{
			m_mouseDown[1] = true;
			OnMouseMove(mousePt);
		}
		else
		{
			isGesturePanInput = true;
			m_mousePos = mousePt;
			UpdateLookAt();
		}
		break;

	case GID_TWOFINGERTAP:
	case GID_PRESSANDTAP:
		if (!IsTouchWindow(m_owner->getSystemHandle(), NULL))
			RegisterTouchWindow(m_owner->getSystemHandle(), 0);
		break;
	}
}


// Handling Mouse Move Event
void cMapView::OnMouseMove(const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	if (m_mouseDown[0])
	{
		Vector3 dir = GetMainCamera().GetDirection();
		Vector3 right = GetMainCamera().GetRight();
		dir.y = 0;
		dir.Normalize();
		right.y = 0;
		right.Normalize();

		GetMainCamera().MoveRight(-delta.x * m_rotateLen * 0.001f);
		GetMainCamera().MoveFrontHorizontal(delta.y * m_rotateLen * 0.001f);
	}
	else if (m_mouseDown[1])
	{
		const float scale = (m_isGestureInput)? 0.001f : 0.005f;
		m_camera.Yaw2(delta.x * scale, Vector3(0, 1, 0));
		m_camera.Pitch2(delta.y * scale, Vector3(0, 1, 0));
	}
	else if (m_mouseDown[2])
	{
		const float len = GetMainCamera().GetDistance();
		GetMainCamera().MoveRight(-delta.x * len * 0.001f);
		GetMainCamera().MoveUp(delta.y * len * 0.001f);
	}

	// �������� �Ʒ��� ī�޶� ��ġ�ϸ�, ���̸� �����Ѵ�.
	const Vector3 eyePos = m_camera.GetEyePos();
	const float h = m_quadTree.GetHeight(Vector2(eyePos.x, eyePos.z));
	if (eyePos.y < (h + 2))
	{
		const Vector3 newPos(eyePos.x, h + 2, eyePos.z);
		m_camera.SetEyePos(newPos);
	}

	// �̵� ��ο��� �Ÿ��� ������Ʈ �Ѵ�.
	// �� �Ÿ��� �����ϸ鼭 GPS ��ǥ�� �����Ѵ�.
	auto &track = g_global->m_gpsClient.m_paths;
	if (g_global->m_isTraceGPSPos && !track.empty())
	{
		const Vector3 p0 = m_quadTree.Get3DPos(track.back().lonLat);
		m_lookAtDistance = m_camera.GetEyePos().Distance(p0);
		m_lookAtYVector = m_camera.GetDirection().y;
	}
}


// Handling Mouse Button Down Event
void cMapView::OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt)
{
	m_mousePos = mousePt;
	UpdateLookAt();
	SetCapture();

	switch (button)
	{
	case sf::Mouse::Left:
	{
		m_mouseDown[0] = true;
		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		auto result = m_quadTree.Pick(ray);
		Vector3 p1 = result.second;

		m_rotateLen = (p1 - ray.orig).Length();// min(500.f, (p1 - ray.orig).Length());

		cAutoCam cam(&m_camera);
		const Vector2d longLat = m_quadTree.GetWGS84(p1);
		gis::LatLonToUTMXY(longLat.y, longLat.x, 52, g_root.m_utmLoc.x, g_root.m_utmLoc.y);
		g_root.m_lonLat = Vector2((float)longLat.x, (float)longLat.y);

		if (eState::PICK_RANGE == m_state)
		{
			m_pickRange.push_back(longLat);
		}

		if (g_global->m_pickScanLeftTop)
			g_global->m_scanLeftTop = longLat;
		if (g_global->m_pickScanRightBottom)
			g_global->m_scanRightBottom = longLat;
	}
	break;

	case sf::Mouse::Right:
	{
		m_mouseDown[1] = true;

		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		Vector3 target = m_groundPlane1.Pick(ray.orig, ray.dir);
		const float len = (GetMainCamera().GetEyePos() - target).Length();
	}
	break;

	case sf::Mouse::Middle:
		m_mouseDown[2] = true;
		break;
	}
}


void cMapView::OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;
	ReleaseCapture();

	switch (button)
	{
	case sf::Mouse::Left:
		m_mouseDown[0] = false;
		break;
	case sf::Mouse::Right:
		m_mouseDown[1] = false;
		break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = false;
		break;
	}
}


void cMapView::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.code)
		{
		case sf::Keyboard::Return:
			break;

		case sf::Keyboard::Space:
		{
			// 128.467758, 37.171993
			//m_camera.Move();
		}
		break;

		case sf::Keyboard::Left: m_camera.MoveRight2(-0.5f); break;
		case sf::Keyboard::Right: m_camera.MoveRight2(0.5f); break;
		case sf::Keyboard::Up: m_camera.MoveUp2(0.5f); break;
		case sf::Keyboard::Down: m_camera.MoveUp2(-0.5f); break;
		}
		break;

	case sf::Event::MouseMoved:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnMouseMove(pos);
	}
	break;

	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		if (sf::Event::MouseButtonPressed == evt.type)
		{
			if (m_viewRect.IsIn((float)curPos.x, (float)curPos.y))
				OnMouseDown(evt.mouseButton.button, pos);
		}
		else
		{
			OnMouseUp(evt.mouseButton.button, pos);
		}
	}
	break;

	case sf::Event::MouseWheelScrolled:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnWheelMove(evt.mouseWheelScroll.delta, pos);
	}
	break;

	case sf::Event::Gestured:
	{
		POINT curPos = { evt.gesture.x, evt.gesture.y };
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnGestured(evt.gesture.id, pos);
	}
	break;

	}
}


void cMapView::OnResetDevice()
{
	cRenderer &renderer = GetRenderer();

	// update viewport
	sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
	m_camera.SetViewPort(viewRect.Width(), viewRect.Height());

	cViewport vp = GetRenderer().m_viewPort;
	vp.m_vp.Width = viewRect.Width();
	vp.m_vp.Height = viewRect.Height();
	m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, true, true, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

