#pragma once

#define VERMILION_PROP(type, name, offset) \
    __declspec(property(get = get_##name, put = put_##name)) type name; \
    type get_##name() const { return *reinterpret_cast<type*>(reinterpret_cast<std::uintptr_t>(this) + offset); } \
    void put_##name(const type& value) { *reinterpret_cast<type*>(reinterpret_cast<std::uintptr_t>(this) + offset) = value; }