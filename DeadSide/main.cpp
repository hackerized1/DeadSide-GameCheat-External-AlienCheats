#include <Windows.h>
#include <iostream>
#include "overlay.h"
#include "memory.h"
#include "struct.h"
#include "auth.hpp"
#include "protect/protectmain.h"
#include "xorstr.hpp"
#include <iostream>
#include <Windows.h>
#define COLOUR(x) x/255
bool showmenu = true;
uintptr_t LocalPawn, ULocalPlayer, LocalController, World, CameraManager;
#define GWorld 0x4b08ec0
#define UGameInstance 0x180
#define PersistentLevel 0x30
#define Actors 0x98
#define LocalPlayers 0x38
#define PlayerController 0x30
#define Pawn 0x2a0
#define Mesh 0x280
#define RootComponent 0x130
#define RelativeLocation 0x11c
#define RelativeRotation 0xE4C
#define PlayerCameraManager 0x2b8
#define CameraCachePrivate 0x1ae0
TArray<uintptr_t> Characters;
FMinimalViewInfo Kamera;

ImVec4 boxcolor = ImColor(255,255,255);
ImVec4 Snaplinecolor = ImColor(255,255,255);

Vector3 GetBone(DWORD_PTR mesh, int id)
{
	DWORD_PTR array = driver.read<uintptr_t>(mesh + 0x420); // BONE ARRAY
	if (array == NULL)
		array = driver.read<uintptr_t>(mesh + 0x420 + 0X10); // BONE ARRAY

	//printf("array %d\n", array);

	FTransform bone = driver.read<FTransform>(array + (id * 0x30));

	FTransform ComponentToWorld = driver.read<FTransform>(mesh + 0x1C0); // COMPONENT
	D3DMATRIX Matrix;

	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}
Vector3 project_world_to_screen(Vector3 WorldLocation)
{
	Vector3 Screenlocation = Vector3(0, 0, 0);
	FMinimalViewInfo camera = driver.read<FMinimalViewInfo>(CameraManager + CameraCachePrivate + 0x10);

	D3DMATRIX tempMatrix = Matrix(camera.Rotation, Vector3(0, 0, 0));

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]),
		vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]),
		vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - camera.Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f) vTransformed.z = 1.f;

	float FovAngle = camera.FOV;

	float ScreenCenterX = Width / 2.0f;
	float ScreenCenterY = Height / 2.0f;

	Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
	Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

	return Screenlocation;
}

int realkey;
int keystatus;
int aimkey;
bool GetKey(int key)
{
	realkey = key;
	return true;
}
void ChangeKey(void* blank)
{
	keystatus = 1;
	while (true)
	{
		for (int i = 0; i < 0x87; i++)
		{
			if (GetKeyState(i) & 0x8000)
			{
				aimkey = i;
				keystatus = 0;
				return;
			}
		}
	}
}
static const char* keyNames[] = {
	"",
	"Left Mouse",
	"Right Mouse",
	"Cancel",
	"Middle Mouse",
	"Mouse 5",
	"Mouse 4",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
};
static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}
void HotkeyButton(int aimkey, void* changekey, int status)
{
	const char* preview_value = NULL;
	if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
		Items_ArrayGetter(keyNames, aimkey, &preview_value);

	std::string aimkeys;
	if (preview_value == NULL)
		aimkeys = skCrypt("Select Key");
	else
		aimkeys = preview_value;

	if (status == 1)
	{
		aimkeys = skCrypt("Press key to bind");
	}
	if (ImGui::Button(aimkeys.c_str(), ImVec2(100, 20)))
	{
		if (status == 0)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
		}
	}
}

float distanceee = 300;
bool ESP = true;
bool SNAPLINE = true;
bool FILLBOX = true;
bool DISTANCEESP = true;
bool healthbar = true;
bool BoneESP = true;
bool viewrotation = false;
bool radar = true;

bool aimbot;
float aimfovv = 45;
float aSmoothAmount = 1;


ImColor cRainbow = ImColor(255, 255, 182);
static int widthh = 576;
static int heightt = 326;
static int MenuSayfasi = 1;
static int tab = 1;
void draw_menu()
{
	auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 620, 485 };
	ImGui::SetNextWindowSize(ImVec2(gs));
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin(skCrypt("Alien Cheats - (ver. 0.0.2)"), NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	{

		ImGui::SetCursorPosX(121);
		s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2); p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y); auto draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), ImColor(15, 17, 19), 5);
		draw->AddRectFilled(ImVec2(p.x, p.y + 40), ImVec2(p.x + s.x, p.y + s.y), ImColor(18, 20, 21), 5, ImDrawCornerFlags_Bot);

		//draw->AddImage(lg, ImVec2(p.x - 60, p.y - 30), ImVec2(p.x + 110, p.y + 70), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 102));

		ImGui::PushFont(icons);
		auto logo_size = ImGui::CalcTextSize(skCrypt("A"));
		draw->AddText(ImVec2(p.x + 28 - logo_size.x / 2, p.y + 29 - (logo_size.y / 2) + 4), cRainbow, skCrypt("A"));
		ImGui::PopFont();

		ImGui::PushFont(main_font2);
		auto logo_size2 = ImGui::CalcTextSize(skCrypt("A"));
		draw->AddText(ImVec2(p.x + 42 - logo_size2.x / 2, p.y + 29 - logo_size2.y), cRainbow, skCrypt("Alien Cheats"));
		ImGui::PopFont();

		ImGui::PushFont(main_font);
		ImGui::BeginGroup();
		ImGui::SameLine(110);
		if (ImGui::tab(skCrypt("AIMBOT"), tab == 1))tab = 1; ImGui::SameLine();
		if (ImGui::tab(skCrypt("Visuals"), tab == 2))tab = 2;
		ImGui::EndGroup();
		ImGui::PopFont();

		if (tab == 1)
		{
			ImGui::PushFont(main_font);
			{//left upper
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(15);
				ImGui::MenuChild(skCrypt("Aimbot"), ImVec2(285, 415), false);
				{
					ImGui::Checkbox("AIMBOT", &aimbot);
					ImGui::Text(skCrypt("")); ImGui::SameLine(8); HotkeyButton(aimkey, ChangeKey, keystatus); ImGui::SameLine(); ImGui::Text(skCrypt("Aimbot Key"));
					ImGui::SliderFloat(skCrypt("AimFov"), &aimfovv, 30, 180);
					ImGui::SliderFloat(skCrypt("AimSmooth"), &aSmoothAmount, 1, 20);
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			{//right
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(320);
				ImGui::MenuChild(skCrypt(" "), ImVec2(285, 415), false);
				{

				}
				ImGui::EndChild();
				ImGui::EndGroup();
				ImGui::PopFont();
			}
		}
		if (tab == 2)
		{
			ImGui::PushFont(main_font);
			{//left upper
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(15);
				ImGui::MenuChild(skCrypt("Visuals"), ImVec2(285, 415), false);
				{
					ImGui::SetCursorPos({ 120,35 }); ImGui::ColorEdit4("3D Box Color", (float*)&boxcolor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel); ImGui::SetCursorPos({ 0,35 });
					ImGui::Checkbox("ESP", &ESP);
					ImGui::SetCursorPos({ 120,60 }); ImGui::ColorEdit4("Snapline Color", (float*)&Snaplinecolor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel); ImGui::SetCursorPos({ 0,60 });
					ImGui::Checkbox("SNAPLINE", &SNAPLINE);
					ImGui::SetCursorPos({ 0,85 });
					ImGui::Checkbox("FILL BOX", &FILLBOX);
					ImGui::SetCursorPos({ 0,110 });
					ImGui::Checkbox("Radar", &radar);
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			{//right
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(320);
				ImGui::MenuChild(skCrypt(" "), ImVec2(285, 415), false);
				{

				}
				ImGui::EndChild();
				ImGui::EndGroup();
				ImGui::PopFont();
			}
		}
	}
}
RGBA col = { 255,0,0,255 };

ImVec2 GetWindowSize() {
	return { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };
}

float CalculateDistance(int p1x, int p1y, int p2x, int p2y)
{
	float diffY = p1y - p2y;
	float diffX = p1x - p2x;
	return sqrt((diffY * diffY) + (diffX * diffX));
}


void skeletonesp(int X, int Y, int W, int H, int thickness) {
	auto RGB = ImGui::GetColorU32({ 255, 255, 255, 255 });
	ImDrawList* Renderer = ImGui::GetOverlayDrawList();
	Renderer->AddLine(ImVec2(X, Y), ImVec2(W, H), RGB, thickness);

}


float DegreeToRadian(float degree)
{
	return degree * (M_PI / 180);

}

Vector3 RadianToDegree(Vector3 radians)
{
	Vector3 degrees;
	degrees.x = radians.x * (180 / M_PI);
	degrees.y = radians.y * (180 / M_PI);
	degrees.z = radians.z * (180 / M_PI);
	return degrees;
}

Vector3 DegreeToRadian(Vector3 degrees)
{
	Vector3 radians;
	radians.x = degrees.x * (M_PI / 180);
	radians.y = degrees.y * (M_PI / 180);
	radians.z = degrees.z * (M_PI / 180);
	return radians;
}
Vector2 WorldRadar(Vector3 srcPos, Vector3 distPos, float yaw, float radarX, float radarY, float size)
{
	auto cosYaw = cos(DegreeToRadian(yaw));
	auto sinYaw = sin(DegreeToRadian(yaw));

	auto deltaX = srcPos.x - distPos.x;
	auto deltaY = srcPos.y - distPos.y;

	auto locationX = (float)(deltaY * cosYaw - deltaX * sinYaw) / 45.f;
	auto locationY = (float)(deltaX * cosYaw + deltaY * sinYaw) / 45.f;

	if (locationX > (size - 2.f))
		locationX = (size - 2.f);
	else if (locationX < -(size - 2.f))
		locationX = -(size - 2.f);

	if (locationY > (size - 6.f))
		locationY = (size - 6.f);
	else if (locationY < -(size - 6.f))
		locationY = -(size - 6.f);

	return Vector2((int)(-locationX + radarX), (int)(locationY + radarY));
}
static Vector3 pRadar;
void DrawLine3(int x1, int y1, int x2, int y2, RGBA* color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickness);
}
void DrawBorder(float x, float y, float w, float h, float px, RGBA* BorderColor)
{
	DrawRect(x, (y + h - px), w, px, BorderColor, 1 / 2);
	DrawRect(x, y, px, h, BorderColor, 1 / 2);
	DrawRect(x, y, w, px, BorderColor, 1 / 2);
	DrawRect((x + w - px), y, px, h, BorderColor, 1 / 2);
}

void DrawCircleFilled2(int x, int y, int radius, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
}

void DrawRadar(Vector3 EntityPos)
{
	auto radar_posX = pRadar.x + 135;
	auto radar_posY = pRadar.y + 135;
	uint64_t LocalRootComp = driver.read<uint64_t>(LocalPawn + RootComponent);
	FMinimalViewInfo camera = driver.read<FMinimalViewInfo>(CameraManager + CameraCachePrivate + 0x10);
	Vector3 LocalPos = driver.read<Vector3>(LocalRootComp + RelativeLocation);
	auto Radar2D = WorldRadar(LocalPos, EntityPos, camera.Rotation.y, radar_posX, radar_posY, 135.f);
	DrawCircleFilled2(Radar2D.x, Radar2D.y, 3, &Col.red);
}
int r = 255;
int g = 255;
int b = 255;

std::vector<DWORD64> Entity;

void cheat()
{
	for (auto actor : Entity)
	{ 

		uintptr_t pawn = driver.read<uintptr_t>(actor + 0x118);
		uintptr_t playerstate = driver.read<uintptr_t>(pawn + 0x240);
		if (!playerstate)
			continue;


		//uintptr_t rootcomponent = driver.read<uintptr_t>(LocalPawn + RootComponent);
		//
		//Vector3 rootposition = driver.read<Vector3>(rootcomponent + RelativeLocation);
		//Vector3 rootpositionout = project_world_to_screen(rootposition);

		//float health = driver.read<float>(actor + 0x3730);

		uintptr_t mesh = driver.read<uintptr_t>(actor + Mesh);

		Vector3 vBaseBone = GetBone(mesh, 0);
		Vector3 vHeadBone = GetBone(mesh, 90);

		Vector3 vBaseBoneOut = project_world_to_screen(vBaseBone);
		Vector3 vHeadBoneOut = project_world_to_screen(vHeadBone);

		Vector3 vBaseBoneOut2 = project_world_to_screen(Vector3(vBaseBone.x, vBaseBone.y, vBaseBone.z - 15));
		Vector3 vHeadBoneOut2 = project_world_to_screen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z));


		Vector3 pos = GetBone(mesh, 0);
		Vector3 headPos = pos; headPos.z = pos.z + (52.f * 3);
		Vector3 HeadBone = project_world_to_screen(Vector3(headPos));




		FMinimalViewInfo camerainfo = driver.read<FMinimalViewInfo>(CameraManager + CameraCachePrivate + 0x10);
		Vector3 localView = driver.read<Vector3>(LocalController + 0x288);

		Vector3 neck_position = headPos;
		uintptr_t rootcomponentttt = driver.read<uintptr_t>(actor + RootComponent);
		Vector3 root_position = driver.read<Vector3>(rootcomponentttt + RelativeLocation);
		if (neck_position.z <= root_position.z)
		{
			return;
		}
		Vector3 vecCaclculatedAngles = fhgfsdhkfshdghfsd205(camerainfo.Location, headPos);
		Vector3 angleEx = CaadadalcAngle(camerainfo.Location, headPos);

		Vector3 fin = Vector3(vecCaclculatedAngles.y, angleEx.y, 0);
		Vector3 delta = fin - localView;
		Vector3 TargetAngle = localView + (delta / aSmoothAmount);
	
	
		float distance5 = 999999;
		float dist = CalculateDistance(GetWindowSize().x / 2, GetWindowSize().y / 2, HeadBone.x, HeadBone.y);
		if (dist < distance5 && dist < aimfovv)
		{
			if (GetAsyncKeyState(aimkey))
			{
				if (aimbot)
				{
					//CURSORINFO ci = { sizeof(CURSORINFO) };
					//if (GetCursorInfo(&ci))
					//{
					//	if (ci.flags == 0)
					//		mouse_event(MOUSEEVENTF_MOVE, (HeadBone.x - GetWindowSize().x / 2) / aSmoothAmount, (HeadBone.y - GetWindowSize().y / 2) / aSmoothAmount	, 0, 0);
					//}		
					driver.write<Vector3>(LocalController + 0x288, TargetAngle);
				}
			}
		}
	
		float BoxHeight = abs(HeadBone.y - vBaseBoneOut.y);
		float BoxWidth = BoxHeight * 0.55;
		

		if (radar)
		{
			pRadar.x = (GetWindowSize().x / GetWindowSize().x);
			pRadar.y = GetWindowSize().x / 2 - GetWindowSize().y / 2 - 410;
			DrawRect(pRadar.x, pRadar.y, 270, 270, &Col.black, 1);
			DrawBorder(pRadar.x, pRadar.y, 270, 270, 1, &Col.SilverWhite);
			auto radar_posX = pRadar.x + 135;
			auto radar_posY = pRadar.y + 135;
			DrawLine3(radar_posX, radar_posY, radar_posX, radar_posY + 135, &Col.SilverWhite, 1);
			DrawLine3(radar_posX, radar_posY, radar_posX, radar_posY - 135, &Col.SilverWhite, 1);
			DrawLine3(radar_posX, radar_posY, radar_posX + 135, radar_posY, &Col.SilverWhite, 1);
			DrawLine3(radar_posX, radar_posY, radar_posX - 135, radar_posY, &Col.SilverWhite, 1);
			DrawCircleFilled2(radar_posX - 0.5f, radar_posY - 0.5f, 3, &Col.yellow);
			DrawRadar(pos);
		}

		//DrawString(12, vBaseBoneOut2.x, vBaseBoneOut2.y, &Col.white, true, true, "Player");

		if (dist < distance5 && dist < 960)
		{
			if (ESP)
			{
				Vector3 bottom1 = project_world_to_screen(Vector3(vBaseBone.x + 40, vBaseBone.y - 40, vBaseBone.z));
				Vector3 bottom2 = project_world_to_screen(Vector3(vBaseBone.x - 40, vBaseBone.y - 40, vBaseBone.z));
				Vector3 bottom3 = project_world_to_screen(Vector3(vBaseBone.x - 40, vBaseBone.y + 40, vBaseBone.z));
				Vector3 bottom4 = project_world_to_screen(Vector3(vBaseBone.x + 40, vBaseBone.y + 40, vBaseBone.z));

				Vector3 top1 = project_world_to_screen(Vector3(headPos.x + 40, headPos.y - 40, headPos.z + 15));
				Vector3 top2 = project_world_to_screen(Vector3(headPos.x - 40, headPos.y - 40, headPos.z + 15));
				Vector3 top3 = project_world_to_screen(Vector3(headPos.x - 40, headPos.y + 40, headPos.z + 15));
				Vector3 top4 = project_world_to_screen(Vector3(headPos.x + 40, headPos.y + 40, headPos.z + 15));

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ImGui::ColorConvertFloat4ToU32(boxcolor), 0.1f);
			}

			if (SNAPLINE)
			{
				DrawLine(ImVec2(GetWindowSize().x / 2, GetWindowSize().y), ImVec2(vBaseBoneOut.x, vBaseBoneOut.y), ImGui::ColorConvertFloat4ToU32(Snaplinecolor), 1.f); // ASAGIDAN
			}
		}
	}
}


void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(main_font);
	if (GetAsyncKeyState(VK_INSERT) & 1) { showmenu = !showmenu; }
	cheat();

	if (aimbot)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(GetWindowSize().x / 2, GetWindowSize().y / 2), aimfovv, IM_COL32_WHITE, 10000, 1);
	}

	if (showmenu) { draw_menu(); }
	ImGui::PopFont();
	ImGui::EndFrame();
	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}

	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}


void Loop()
{
	while (true)
	{
		World = driver.read<uintptr_t>(g_base + GWorld);
		//printf("[>] GWorld 0x%p\n", World);
		uintptr_t GameInstance = driver.read<uintptr_t>(World + UGameInstance);
		//printf("[>] UGameInstance 0x%p\n", GameInstance);
		uintptr_t Level = driver.read<uintptr_t>(World + PersistentLevel);
		//printf("[>] Level 0x%p\n", Level);
		uintptr_t ULocalPlayers = driver.read<uintptr_t>(GameInstance + LocalPlayers);
		//printf("[>] ULocalPlayers 0x%p\n", ULocalPlayers);
		ULocalPlayer = driver.read<uintptr_t>(ULocalPlayers);
		//printf("[>] ULocalPlayer 0x%p\n", ULocalPlayer);
		LocalController = driver.read<uintptr_t>(ULocalPlayer + PlayerController);
		//printf("[>] LocalController 0x%p\n", LocalController);
		LocalPawn = driver.read<uintptr_t>(LocalController + Pawn);
		//printf("[>] LocalPawn 0x%p\n", LocalPawn);	
		CameraManager = driver.read<uintptr_t>(LocalController + PlayerCameraManager);
		//printf("[>] CameraManager 0x%p\n", CameraManager);
		Characters = driver.read<TArray<uintptr_t>>(Level + Actors);

		if (Characters.Count == 0)
			return;
		for (int i = 0; i < Characters.Count; i++)
		{
			uintptr_t actor = Characters[i];
			if (!actor)
				continue;

			if (actor == LocalPawn)
				continue;


			Entity.push_back(actor);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		Entity.clear();
	}
}

using namespace KeyAuth;

std::string namee = "Super People"; // application name. right above the blurred text aka the secret on the licenses tab among other tabs
std::string ownerid = "jHHXUBClti"; // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
std::string secret = "ef71c43cbc4fdfaedd38941b6d240cb1b2b417b66897ea67359780734731ecca"; // app secret, the blurred text on licenses tab and other tabs
std::string version = "1.1"; // leave alone unless you've changed version on website
std::string url = "https://auth.aliencheats.com/api/1.1/"; // change if you're self-hosting
std::string sslPin = "ssl pin key (optional)"; // don't change unless you intend to pin public certificate key. you can get here in the "Pin SHA256" field https://www.ssllabs.com/ssltest/analyze.html?d=keyauth.win&latest. If you do this you need to be aware of when SSL key expires so you can update it
api KeyAuthApp(namee, ownerid, secret, version, url, sslPin);

int main()
{
//	SetConsoleTitleA(skCrypt("SUPER PEOPLE V1"));
//	int horizontal = 0, vertical = 0;
//	int x = 350, y = 250; //// alta doðru
//	HWND consoleWindow = GetConsoleWindow();
//	SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
//	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
//	CONSOLE_CURSOR_INFO     cursorInfo;
//	GetConsoleCursorInfo(out, &cursorInfo);
//	SetConsoleCursorInfo(out, &cursorInfo);
//	CONSOLE_FONT_INFOEX cfi;
//	cfi.cbSize = sizeof(cfi);
//	cfi.nFont = 0;
//	cfi.dwFontSize.X = 0;
//	cfi.dwFontSize.Y = 15;
//	cfi.FontFamily = FF_DONTCARE;
//	cfi.FontWeight = FW_NORMAL;
//	system("color 4");
//	wcscpy_s(cfi.FaceName, L"Consolas");
//	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
//	HWND hwnd = GetConsoleWindow();
//	MoveWindow(hwnd, 0, 0, x, y, TRUE);
//	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
//	lStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
//	SetWindowLong(hwnd, GWL_STYLE, lStyle);
//	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
//	CONSOLE_SCREEN_BUFFER_INFO csbi;
//	GetConsoleScreenBufferInfo(console, &csbi);
//	COORD scrollbar = {
//		csbi.srWindow.Right - csbi.srWindow.Left + 1,
//		csbi.srWindow.Bottom - csbi.srWindow.Top + 1
//	};
//	if (console == 0)
//		throw 0;
//	else
//		SetConsoleScreenBufferSize(console, scrollbar);
//	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
//	SetLayeredWindowAttributes(hwnd, 0, 225, LWA_ALPHA);
//	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
//
//	SetConsoleTitle("SUPER PEOPLE V1");
//	mainprotect();
//
//	std::string DosyaKonumu = (_xor_("C:\\Windows\\System\\SuperPeoplePrivLicense.txt"));
//
//	KeyAuthApp.init();
//EnBasaDon:
//	std::string License;
//	FILE* Dosya;
//	if (Dosya = fopen((DosyaKonumu.c_str()), skCrypt("r"))) {
//		std::ifstream DosyaOku(DosyaKonumu.c_str());
//		std::string Anahtar;
//		std::getline(DosyaOku, Anahtar);
//		DosyaOku.close();
//		fclose(Dosya);
//		License = Anahtar;
//		goto LisansaGit;
//	}
//	else
//	{
//		SetConsoleTitleA(skCrypt("  "));
//
//		system(skCrypt("cls"));
//		std::cout << skCrypt("\n\n  [+] Connecting..");
//		std::cout << skCrypt("\n\n  [+] Enter License: ");
//		std::cin >> License;
//	LisansaGit:
//		std::ofstream key; key.open(DosyaKonumu.c_str());
//		key << License;
//		key.close();
//		KeyAuthApp.license(License);
//		if (!KeyAuthApp.data.success)
//		{
//			std::cout << _xor_("\n Status: ").c_str() + KeyAuthApp.data.message;
//			Sleep(1500);
//			remove(DosyaKonumu.c_str());
//			goto EnBasaDon;
//			//exit(0);
//		}
//		system(skCrypt("cls"));
//		Sleep(300);
//		std::cout << skCrypt("\n\n  [+] License Activated.") << std::endl;;
//	}

r8:
	if (!driver.init())
	{
	r7:
		if (FindWindowA(skCrypt("Deadside-Win64-Shipping.exe"), NULL))
		{
			printf(skCrypt("[>] close game please...\n"));
			Sleep(1000);
			goto r7;
		}
		if (LoadDriver())
		{
			printf(skCrypt("[>] driver loaded!\n"));
			Sleep(1000);
			system("cls");
			goto r8;
		}
	}
	HWND Entryhwnd = NULL;
	while (Entryhwnd == NULL)
	{
		printf(skCrypt("[>] waiting for game\n"));
		Sleep(1);
		g_pid = get_pid("Deadside-Win64-Shipping.exe");
		Entryhwnd = get_process_wnd(g_pid);
		Sleep(1);
	}
	printf(skCrypt("[>] pid: %d\n"), g_pid);
	driver.attach(g_pid);
	setup_window();
	init_wndparams(MyWnd);
	g_base = driver.get_process_base(g_pid);
	if (!g_base) { printf(skCrypt("[>] g_base can't found!\n")); return 0; }
	printf("[>] g_base: 0x%p\n", g_base);
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));
	Style();
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	std::thread(Loop).detach();
	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();


		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom) {

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
		Sleep(1);
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	cleanup_d3d();
	DestroyWindow(MyWnd);
	return Message.wParam;
}