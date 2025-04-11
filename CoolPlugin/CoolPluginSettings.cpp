#include "pch.h"
#include "CoolPlugin.h"

void Speedometer::RenderSettings() {
    ImGui::TextUnformatted("Speedometer");

    // Display text toggle
    CVarWrapper enableCvar = cvarManager->getCvar("plugin_enabled");
    if (!enableCvar) { return; }
    bool enabled = enableCvar.getBoolValue();
    if (ImGui::Checkbox("Enable Text", &enabled)) {
        enableCvar.setValue(enabled);
    }

}

