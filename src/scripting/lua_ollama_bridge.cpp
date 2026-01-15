/*
	Copyright (C) 2017 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_ollama_bridge.hpp"
#include "ollama.hpp"

#include "lua/wrapper_lauxlib.h"

/**
 * Generates text using Ollama LLM.
 * - Arg 1: Prompt text as a string
 * - Ret 1: Generated text as a string
 */
int lua_ollama_bridge::intf_generate_ollama(lua_State* L)
{
	const std::string prompt = luaL_checkstring(L, 1);

	ollama llm;
	std::string message = llm.generate(prompt);

	lua_pushlstring(L, message.c_str(), message.size());
	return 1;
}
