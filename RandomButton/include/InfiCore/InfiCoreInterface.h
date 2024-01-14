#pragma once

#include <YYToolkit/Shared.hpp>
using namespace Aurie;

struct InfiCoreInterface : AurieInterfaceBase {
	virtual AurieStatus Create();
	virtual void Destroy();
	virtual void QueryVersion(
		OUT short& Major,
		OUT short& Minor,
		OUT short& Patch
	);

	virtual AurieStatus SetReadConfigCallback(
		IN const std::string ModFileName,
		IN const std::function<void()> ReadConfigFunction
	);

	virtual AurieStatus CallConfigCallback(
		IN const std::string ModFileName
	);

private:
	std::unordered_map<std::string, std::function<void()>> m_ConfigFunctionMap;
};