#pragma once

#include <utility>
#include <cstdint>

#include <string>

namespace Vermilion {
	// IDA-pattern style signature
	using Signature = const char*;

	class SigScanner {
	public:
		explicit SigScanner(std::string_view moduleName);
		std::uintptr_t Scan(const Signature& signature) const;

	private:
		std::pair<std::uintptr_t, std::uintptr_t> textRange_;
		std::string moduleName_;
	};
} // namespace Vermilion
