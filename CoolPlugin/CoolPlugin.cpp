#include "pch.h"
#include "CoolPlugin.h"
#include "bakkesmod/wrappers/includes.h"
#include "bakkesmod/wrappers/wrapperstructs.h"

BAKKESMOD_PLUGIN(CoolPlugin, "SpeedGUI", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

bool pluginEnabled = true;

void CoolPlugin::onLoad()
{
	// This line is required for LOG to work and must be before any use of LOG()
	_globalCvarManager = cvarManager;
	LOG("Plugin loaded!");

	cvarManager->registerCvar("plugin_enabled", "0", "Enable Plugin", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		pluginEnabled = cvar.getBoolValue();
			});

	

	gameWrapper->RegisterDrawable(std::bind(&CoolPlugin::Render, this, std::placeholders::_1));

	
}


void CoolPlugin::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInFreeplay() || !pluginEnabled) { return; }	// Only load if in free play

	CarWrapper car = gameWrapper->GetLocalCar(); if (!car) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState(); if (!server) return;

	BallWrapper ball = server.GetBall(); if (!ball) return;

	// Speeds in km/h
	float carSpeedKPH = car.GetVelocity().magnitude() * 0.036f;
	float ballSpeedKPH = ball.GetVelocity().magnitude() * 0.036f;

	std::string carSpeedStr = "Car Speed:  " + std::to_string(static_cast<int>(carSpeedKPH)) + " km/h";
	std::string ballSpeedStr = "Ball Speed: " + std::to_string(static_cast<int>(ballSpeedKPH)) + " km/h";

	// Layout
	float fontScale = 3.5f;
	float lineHeight = 40.0f;
	float textWidth = carSpeedStr.length() * 20.0f * fontScale; // rough estimate

	Vector2 screenSize = canvas.GetSize();
	float centerX = screenSize.X / 2.0f;
	float centerY = screenSize.Y / 2.0f;
	float offsetY = -200.0f;

	// Color Logic

	float diff = fabs(carSpeedKPH - ballSpeedKPH);

	// Color thresholds (in km/h)
	const float greenThreshold = 0.5f;
	const float yellowThreshold = 7.0f;
	const float redThreshold = 15.0f;

	// Normalize color gradient (0 = red, 1 = green)
	float t = 0.0f;
	if (diff <= greenThreshold)
		t = 1.0f; // pure green
	else if (diff <= yellowThreshold)
		t = (yellowThreshold - diff) / (yellowThreshold - greenThreshold); // yellow ? green
	else if (diff <= redThreshold)
		t = (redThreshold - diff) / (redThreshold - yellowThreshold); // red ? yellow
	else
		t = 0.0f; // pure red

	LinearColor speedMatchColor;

	if (diff <= greenThreshold)
	{
		// "Glow" green when very close match
		speedMatchColor = LinearColor{ 0.0f, 1.0f, 0.0f, 1.0f }; // full green
	}
	else
	{
		// Interpolated red-yellow-green
		float r = 1.0f - t;
		float g = t;
		speedMatchColor = LinearColor{ r, g, 0.0f, 1.0f };
	}


	// Draw car speed 
	canvas.SetColor(LinearColor{ 255.0f, 215.0f, 0.0f, 255.0f });
	canvas.SetPosition(Vector2F{ centerX - textWidth / 6.0f, centerY + offsetY + lineHeight });
	canvas.DrawString(carSpeedStr, fontScale, fontScale, true, false);

	// Draw ball speed (always gold)
	canvas.SetColor(LinearColor{ 255.0f, 215.0f, 0.0f, 255.0f });
	canvas.SetPosition(Vector2F{ centerX - textWidth / 6.0f, centerY + offsetY });
	canvas.DrawString(ballSpeedStr, fontScale, fontScale, true, false);


	if (gameWrapper->IsInFreeplay())
	{
		car.SetCarColor(speedMatchColor, speedMatchColor);
	}

}



