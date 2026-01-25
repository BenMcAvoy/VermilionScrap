#pragma once

#define VERMILION_PROP(type, name, offset) \
    __declspec(property(get = get_##name, put = put_##name)) type name; \
    type get_##name() const { return *reinterpret_cast<(type)*>(reinterpret_cast<std::uintptr_t>(this) + (offset)); } \
    void put_##name(const type& value) { *reinterpret_cast<(type)*>(reinterpret_cast<std::uintptr_t>(this) + (offset)) = value; }

#define VERMILION_REBASE(offset) (Raw::offset + (std::uintptr_t)GetModuleHandle(0))
#define VERMILION_REBASE_OFFSET(offset) const std::uintptr_t offset = VERMILION_REBASE(offset)

#define PRI private:
#define PUB public:

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)
#define PAD(size) char CONCAT(pad_, CONCAT(size, __LINE__))[size]
