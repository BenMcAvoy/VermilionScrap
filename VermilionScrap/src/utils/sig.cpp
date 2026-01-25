#include "sig.h"

#include <stdexcept>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace Vermilion;

SigScanner::SigScanner(const std::string_view moduleName)
	: moduleName_(moduleName) {
	const auto moduleHandle = reinterpret_cast<std::uintptr_t>(
		GetModuleHandleA(moduleName_.c_str())
		);

	if (!moduleHandle) {
		throw std::runtime_error("Failed to get module handle for " + moduleName_);
	}

	const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleHandle);
	const auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
		moduleHandle + dosHeader->e_lfanew
		);

	const auto firstSection = IMAGE_FIRST_SECTION(ntHeaders);
	PIMAGE_SECTION_HEADER textSection = nullptr;

	for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
		const auto section = &firstSection[i];

		if (std::memcmp(section->Name, ".text", 5) == 0) {
			textSection = section;
			break;
		}
	}

	std::uintptr_t textStart = moduleHandle + textSection->VirtualAddress;
	std::uintptr_t textEnd = textStart + textSection->Misc.VirtualSize;

	textRange_ = {
		textStart,
		textEnd
	};
}

static void parseSignature(const std::string& sig, std::vector<std::uint8_t>& bytes, std::string& mask) {
    size_t i = 0;
    while (i < sig.size()) {
        if (sig[i] == ' ') {
            ++i;
            continue;
        }
        if (sig[i] == '?') {
            bytes.push_back(0x00);
            mask.push_back('?');
            if (i + 1 < sig.size() && sig[i + 1] == '?') ++i;
        } else {
            bytes.push_back(static_cast<std::uint8_t>(std::stoul(sig.substr(i, 2), nullptr, 16)));
            mask.push_back('x');
            ++i;
        }
        // skip past the current byte
        if (sig[i] != ' ' && sig[i] != '\0') ++i;
    }
}

std::uintptr_t SigScanner::Scan(const Signature& signature) const {
    std::vector<std::uint8_t> patternBytes;
    std::string mask;
    parseSignature(signature, patternBytes, mask);

    const auto* start = reinterpret_cast<std::uint8_t*>(textRange_.first);
    const auto* end = reinterpret_cast<std::uint8_t*>(textRange_.second);

    for (auto* current = start; current + patternBytes.size() <= end; ++current) {
        bool found = true;
        for (size_t i = 0; i < patternBytes.size(); ++i) {
            if (mask[i] == 'x' && current[i] != patternBytes[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<std::uintptr_t>(current);
        }
    }

    throw std::runtime_error("Signature not found in module " + moduleName_);
}