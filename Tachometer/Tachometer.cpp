#include "pch.h"
#include "Tachometer.h"
#include "bakkesmod/wrappers/includes.h"
#include "bakkesmod/wrappers/wrapperstructs.h"
#include <algorithm> // For std::clamp

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



BAKKESMOD_PLUGIN(Tachometer, "Tachometer", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

bool pluginEnabled = true;

void Tachometer::onLoad()
{
	// This line is required for LOG to work and must be before any use of LOG()
	_globalCvarManager = cvarManager;
	LOG("Plugin loaded!");

	cvarManager->registerCvar("plugin_enabled", "1", "Enable Tachometer", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		pluginEnabled = cvar.getBoolValue();
			});

	

	gameWrapper->RegisterDrawable(std::bind(&Tachometer::Render, this, std::placeholders::_1));

	
}


void Tachometer::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInGame() || !pluginEnabled)
    return;


	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) return;

	float carSpeedKPH = car.GetVelocity().magnitude() * 0.036f;
	float maxSpeed = 90.0f;

	Vector2 screenSize = canvas.GetSize();
	Vector2F center = { 200.0f, screenSize.Y - 200.0f };
	float radius = 100.0f;

	// Arc sweep: Clockwise from 90� to -30�
	const float startDeg = 90.0f;
	const float endDeg = -30.0f;
	const float startRad = startDeg * (M_PI / 180.0f);
	const float endRad = endDeg * (M_PI / 180.0f);
	const float sweep = (endRad + 2.0f * M_PI - startRad);

	const int segments = 60;
	const float arcThickness = 12.0f;
	const float gapRatio = 0.2f;

	float supersonicThreshold = 79.0f;
	float supersonicStartT = std::clamp(supersonicThreshold / maxSpeed, 0.0f, 1.0f);

	// Combined loop for both gray and red segments
	for (int i = 0; i < segments; ++i)
	{
		float t0 = (float)i / segments;
		float t1 = (float)(i + 1) / segments;

		float segmentGap = (sweep * (t1 - t0)) * gapRatio;
		float a0 = startRad + sweep * t0 + segmentGap;
		float a1 = startRad + sweep * t1 - segmentGap;

		if (a0 >= 2.0f * M_PI) a0 -= 2.0f * M_PI;
		if (a1 >= 2.0f * M_PI) a1 -= 2.0f * M_PI;

		float arcRadius = radius + 12.0f;
		Vector2F p0 = { center.X + cosf(a0) * arcRadius, center.Y + sinf(a0) * arcRadius };
		Vector2F p1 = { center.X + cosf(a1) * arcRadius, center.Y + sinf(a1) * arcRadius };

		// Red if segment is in supersonic region, else gray
		float midT = (t0 + t1) / 2.0f;
		if (midT >= supersonicStartT)
			canvas.SetColor(LinearColor{ 255, 0, 0, 255 });
		else
			canvas.SetColor(LinearColor{ 150, 150, 150, 100 });

		canvas.DrawLine(p0, p1, arcThickness);
	}

	// Tick marks
	canvas.SetColor(LinearColor{ 255, 255, 255, 255 });
	for (int i = 0; i <= 10; ++i)
	{
		float t = (float)i / 10.0f;
		float angle = startRad + sweep * t;
		if (angle >= 2.0f * M_PI) angle -= 2.0f * M_PI;

		Vector2F p1 = { center.X + cosf(angle) * (radius - 8.0f), center.Y + sinf(angle) * (radius - 8.0f) };
		Vector2F p2 = { center.X + cosf(angle) * radius, center.Y + sinf(angle) * radius };

		canvas.DrawLine(p1, p2, 2.5f);
	}

	// Needle
	LinearColor needleColor = LinearColor{ 200, 200, 200, 255 };
	float needleLength = radius - 30.0f;
	float baseInset = 15.0f;
	float needleWidth = 10.0f;

	float ratio = std::clamp(carSpeedKPH / maxSpeed, 0.0f, 1.0f);
	float needleAngle = startRad + sweep * ratio;
	if (needleAngle >= 2.0f * M_PI) needleAngle -= 2.0f * M_PI;

	Vector2F tip = {
		center.X + cosf(needleAngle) * needleLength,
		center.Y + sinf(needleAngle) * needleLength
	};

	Vector2F baseCenter = {
		center.X + cosf(needleAngle) * -baseInset,
		center.Y + sinf(needleAngle) * -baseInset
	};

	float perpAngle = needleAngle + M_PI / 2.0f;
	Vector2F baseLeft = {
		baseCenter.X + cosf(perpAngle) * (needleWidth / 2.0f),
		baseCenter.Y + sinf(perpAngle) * (needleWidth / 2.0f)
	};
	Vector2F baseRight = {
		baseCenter.X - cosf(perpAngle) * (needleWidth / 2.0f),
		baseCenter.Y - sinf(perpAngle) * (needleWidth / 2.0f)
	};

	canvas.FillTriangle(baseLeft, baseRight, tip, needleColor);

	// Digital Speed Text
	canvas.SetColor(LinearColor{ 255, 255, 255, 255 });

	float textOffsetX = 40.0f;
	float textOffsetY = 20.0f;
	int speedValue = static_cast<int>(carSpeedKPH);

	canvas.SetPosition(Vector2F{ center.X + textOffsetX, center.Y + textOffsetY - 30.0f });
	canvas.DrawString(std::to_string(speedValue), 3.5f, 3.5f, false, false);

	canvas.SetPosition(Vector2F{ center.X + textOffsetX + 5.0f, center.Y + textOffsetY + 10.0f });
	canvas.DrawString("KM/H", 1.8f, 1.8f, false, false);
}








