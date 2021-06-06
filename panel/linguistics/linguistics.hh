#pragma once
#include <map>
#include <string>
#include <config.hh>

struct string_data {
public:
	const char* chinese = "";
	const char* english = "";
};

extern void build_strings();
extern const char* strings(const char* english);