// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Create a formatted text string for debugging purposes.
/// Note that this function has an internal buffer of 1024 characters, so long strings will be trimmed.
JPH_EXPORT JoltString JoltStringFormat(const char *inFMT, ...);

/// Convert type to string
template<typename T>
JoltString ConvertToJoltString(const T &inValue)
{
	using OJoltStringStream = std::basic_ostringstream<char, std::char_traits<char>, STLAllocator<char>>;
    OJoltStringStream oss;
    oss << inValue;
    return oss.str();
}

/// Calculate the FNV-1a hash of inJoltString.
/// @see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
constexpr uint64 HashJoltString(const char *inJoltString)
{
	uint64 hash = 14695981039346656037UL;
	for (const char *c = inJoltString; *c != 0; ++c)
	{
		hash ^= *c;
		hash = hash * 1099511628211UL;
	}
	return hash;
}

/// Replace substring with other string
JPH_EXPORT void JoltStringReplace(JoltString &ioJoltString, const string_view &inSearch, const string_view &inReplace);

/// Convert a delimited string to an array of strings
JPH_EXPORT void JoltStringToVector(const string_view &inJoltString, Array<JoltString> &outVector, const string_view &inDelimiter = ",", bool inClearVector = true);

/// Convert an array strings to a delimited string
JPH_EXPORT void VectorToJoltString(const Array<JoltString> &inVector, JoltString &outJoltString, const string_view &inDelimiter = ",");

/// Convert a string to lower case
JPH_EXPORT JoltString ToLower(const string_view &inJoltString);

/// Converts the lower 4 bits of inNibble to a string that represents the number in binary format
JPH_EXPORT const char *NibbleToBinary(uint32 inNibble);

JPH_NAMESPACE_END
