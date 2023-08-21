#pragma once

#include <array>
#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <atomic>

#ifdef MK_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifndef LAMBDA
#define LAMBDA(...) std::function<void(__VA_ARGS__)> const&
#endif // LAMBDA