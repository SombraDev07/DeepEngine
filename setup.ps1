$thirdparty = "$PSScriptRoot\thirdparty"

Write-Host "=== Engine Third-Party Setup ===" -ForegroundColor Cyan

function Download-Zip($url, $dest) {
	Write-Host "Downloading $url ..."
	$zip = "$env:TEMP\engine_dep.zip"
	Invoke-WebRequest -Uri $url -OutFile $zip
	Expand-Archive -Path $zip -DestinationPath $dest -Force
	Remove-Item $zip
}

# EnTT - ECS (header-only)
if (-not (Test-Path "$thirdparty\EnTT\src\entt\entt.hpp")) {
	Write-Host "[1/4] EnTT (ECS)..." -ForegroundColor Yellow
	New-Item -ItemType Directory -Force -Path "$thirdparty\EnTT\src"
	Download-Zip "https://github.com/skypjack/entt/archive/refs/tags/v3.13.2.zip" "$thirdparty\EnTT"
	Rename-Item "$thirdparty\EnTT\entt-3.13.2" "$thirdparty\EnTT\tmp" -Force
	Move-Item "$thirdparty\EnTT\tmp\src\*" "$thirdparty\EnTT\src" -Force
	Remove-Item "$thirdparty\EnTT\tmp" -Recurse -Force
	Write-Host "  EnTT installed" -ForegroundColor Green
} else {
	Write-Host "[1/4] EnTT - already installed" -ForegroundColor Gray
}

# Dear ImGui
if (-not (Test-Path "$thirdparty\imgui\imgui.h")) {
	Write-Host "[2/4] Dear ImGui (Editor UI)..." -ForegroundColor Yellow
	Download-Zip "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.0.zip" "$thirdparty\imgui"
	Rename-Item "$thirdparty\imgui\imgui-1.91.0" "$thirdparty\imgui\tmp" -Force
	Get-ChildItem "$thirdparty\imgui\tmp\*.h", "$thirdparty\imgui\tmp\*.cpp" | Move-Item -Destination "$thirdparty\imgui" -Force
	Copy-Item "$thirdparty\imgui\tmp\backends\imgui_impl_win32.*" "$thirdparty\imgui" -Force
	Copy-Item "$thirdparty\imgui\tmp\backends\imgui_impl_dx11.*" "$thirdparty\imgui" -Force
	Remove-Item "$thirdparty\imgui\tmp" -Recurse -Force
	Write-Host "  ImGui installed" -ForegroundColor Green
} else {
	Write-Host "[2/4] ImGui - already installed" -ForegroundColor Gray
}

# Jolt Physics
if (-not (Test-Path "$thirdparty\JoltPhysics\Jolt\Jolt.h")) {
	Write-Host "[3/4] Jolt Physics..." -ForegroundColor Yellow
	Download-Zip "https://github.com/jrouwe/JoltPhysics/archive/refs/tags/v5.0.0.zip" "$thirdparty\JoltPhysics"
	Rename-Item "$thirdparty\JoltPhysics\JoltPhysics-5.0.0" "$thirdparty\JoltPhysics\src" -Force
	Write-Host "  Jolt installed" -ForegroundColor Green
} else {
	Write-Host "[3/4] Jolt Physics - already installed" -ForegroundColor Gray
}

# MiniAudio
if (-not (Test-Path "$thirdparty\miniaudio\miniaudio.h")) {
	Write-Host "[4/4] MiniAudio..." -ForegroundColor Yellow
	New-Item -ItemType Directory -Force -Path "$thirdparty\miniaudio"
	Invoke-WebRequest "https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h" -OutFile "$thirdparty\miniaudio\miniaudio.h"
	Write-Host "  MiniAudio installed" -ForegroundColor Green
} else {
	Write-Host "[4/4] MiniAudio - already installed" -ForegroundColor Gray
}

Write-Host ""
Write-Host "=== Setup complete! ===" -ForegroundColor Green
Write-Host "Run: cmake -B build && cmake --build build" -ForegroundColor White
