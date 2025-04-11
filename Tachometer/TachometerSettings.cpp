#include "pch.h"
#include "Tachometer.h"

void Tachometer::RenderSettings() {
    ImGui::TextUnformatted("Tachometer");

    // Display Tachometer
    CVarWrapper enableCvar = cvarManager->getCvar("plugin_enabled");
    if (!enableCvar) { return; }
    bool enabled = enableCvar.getBoolValue();
    if (ImGui::Checkbox("Display Tachometer", &enabled)) {
        enableCvar.setValue(enabled);
    }

}

