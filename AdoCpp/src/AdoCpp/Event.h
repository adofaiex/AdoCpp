#pragma once
#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/document.h>

// ReSharper disable CppUnusedIncludeDirective
#include "Easing.h"
#include "Events/Base.h"
#include "Events/Dlc.h"
#include "Events/GamePlay.h"
#include "Events/Modifiers.h"
#include "Events/Track.h"
#include "Events/Visual.h"
#include "Utils.h"
// ReSharper restore CppUnusedIncludeDirective

/**
 * @brief Event namespace.
 */
namespace AdoCpp::Event
{
    Event* newEvent(const rapidjson::Value& json);
}
