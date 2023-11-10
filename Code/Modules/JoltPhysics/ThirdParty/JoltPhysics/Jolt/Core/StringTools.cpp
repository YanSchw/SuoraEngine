// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/StringTools.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <cstdarg>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

JoltString JoltStringFormat(const char *inFMT, ...)
{
	char buffer[1024];

	// Format the string
	va_list list;
	va_start(list, inFMT);
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	return JoltString(buffer);
}

void JoltStringReplace(JoltString &ioJoltString, const string_view &inSearch, const string_view &inReplace)
{
	size_t index = 0;
	for (;;)
	{
		 index = ioJoltString.find(inSearch, index);
		 if (index == JoltString::npos)
			 break;

		 ioJoltString.replace(index, inSearch.size(), inReplace);

		 index += inReplace.size();
	}
}

void JoltStringToVector(const string_view &inJoltString, Array<JoltString> &outVector, const string_view &inDelimiter, bool inClearVector)
{
	JPH_ASSERT(inDelimiter.size() > 0);

	// Ensure vector empty
	if (inClearVector)
		outVector.clear();

	// No string? no elements
	if (inJoltString.empty())
		return;

	// Start with initial string
	JoltString s(inJoltString);

	// Add to vector while we have a delimiter
	size_t i;
	while (!s.empty() && (i = s.find(inDelimiter)) != JoltString::npos)
	{
		outVector.push_back(s.substr(0, i));
		s.erase(0, i + inDelimiter.length());
	}

	// Add final element
	outVector.push_back(s);
}

void VectorToJoltString(const Array<JoltString> &inVector, JoltString &outJoltString, const string_view &inDelimiter)
{
	// Ensure string empty
	outJoltString.clear();

	for (const JoltString &s : inVector)
	{
		// Add delimiter if not first element
		if (!outJoltString.empty())
			outJoltString.append(inDelimiter);

		// Add element
		outJoltString.append(s);
	}
}

JoltString ToLower(const string_view &inJoltString)
{
	JoltString out;
	out.reserve(inJoltString.length());
	for (char c : inJoltString)
		out.push_back((char)tolower(c));
	return out;
}

const char *NibbleToBinary(uint32 inNibble)
{
	static const char *nibbles[] = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };
	return nibbles[inNibble & 0xf];
}

JPH_NAMESPACE_END
