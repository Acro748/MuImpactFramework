#pragma once

namespace Mus {
	typedef std::pair<RE::BSFixedString, std::uint32_t> Pair;
	extern std::unordered_map<std::string, std::vector<Pair>> locationalNodeMap;
	extern std::vector<Pair> defaultNodeNames;
}