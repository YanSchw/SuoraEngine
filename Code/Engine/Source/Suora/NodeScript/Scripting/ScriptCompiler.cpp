#include "Precompiled.h"
#include "ScriptVM.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/ScriptClass.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Common/VectorUtils.h"
#include "Platform/Platform.h"
#include "Suora/Common/Common.h"

namespace Suora
{

	enum class TokenType : uint64_t
	{
		None = 0,
		Specifier,
		Typespecifier,
		OpenBracketRound,
		CloseBracketRound,
		OpenBracketCurly,
		CloseBracketCurly,
		Comma,
		Semicolon,
		Dot,
		Colon,
		Extends,
		Var,
		Function, Event
	};

	struct Token
	{
		TokenType Type = TokenType::None;
		std::string Value;
		Token() {}
		Token(TokenType type, const std::string& value) :Type(type), Value(value) {}
	};

	static TokenType IsSingleCharToken(char c)
	{
		switch (c)
		{
		case '(': return TokenType::OpenBracketRound;
		case ')': return TokenType::CloseBracketRound;
		case '{': return TokenType::OpenBracketCurly;
		case '}': return TokenType::CloseBracketCurly;
		case ',': return TokenType::Comma;
		case ';': return TokenType::Semicolon;
		case '.': return TokenType::Dot;
		case ':': return TokenType::Colon;
		default: break;
		}
		return TokenType::None;
	}
	static TokenType IsComplexStringToken(const std::string& str, char nextChar)
	{
		if ((nextChar >= 'a' && nextChar <= 'z') || (nextChar >= 'A' && nextChar <= 'Z') || (nextChar >= '0' && nextChar <= '9')) return TokenType::None;
		if (str == "extends") return TokenType::Extends;
		if (str == "var") return TokenType::Var;
		if (str == "func" || str == "function") return TokenType::Function;
		if (str == "event") return TokenType::Event;

		return TokenType::None;
	}

	static std::vector<Token> Tokenize(const std::string& src)
	{
		std::vector<Token> Tokens;
		std::string Current;
		bool skipTokenAccumulation = false;

		for (char c : src)
		{
			skipTokenAccumulation = false;
			if (IsComplexStringToken(Current, c) != TokenType::None)
			{
				Tokens.push_back(Token(IsComplexStringToken(Current, c), Current));
				Current = "";
				skipTokenAccumulation = true;
			}

			if (c == ' ' || c == '\n' || c == '\t')
			{
				if (Current.size() > 0) Tokens.push_back(Token(TokenType::Specifier, Current));
				Current = "";
				continue;
			}

			if (IsSingleCharToken(c) != TokenType::None)
			{
				if (Current.size() > 0) Tokens.push_back(Token(TokenType::Specifier, Current));
				Tokens.push_back(Token(IsSingleCharToken(c), std::string()+c));
				Current = "";
				continue;
			}

			if (!skipTokenAccumulation) Current += c;
		}

		return Tokens;
	}

	static Class StringToClass(const std::string& str)
	{
		return Object::StaticClass();
	}
	static bool IsSpecifierATypeSpecifier(const std::string& str)
	{
		return false;
	}

	void ScriptEngine::CompileAllScriptClasses()
	{
		return;
		/// TODO: Refactor


		Array<ScriptClass*> classes = AssetManager::GetAssets<ScriptClass>();
		std::unordered_map<ScriptClass*, std::vector<Token>> ScriptTokens;

		// 1. Tokenize
		for (ScriptClass* sclass : classes)
		{
			sclass->m_ScriptClass->m_ScriptVars.clear();
			sclass->m_ScriptClass->m_Functions.clear();

			std::string source = Platform::ReadFromFile(sclass->m_Path.string());
			Util::RemoveCommentsFromString(source);

			ScriptTokens[sclass] = Tokenize(source);

			for (auto& it : ScriptTokens[sclass])
			{
				SuoraLog("{0}", it.Value);
			}
			SuoraLog("\n");
		}

		// 2. Reflect
		for (ScriptClass* sclass : classes)
		{
			// ParentClass
			sclass->m_ParentClass = Object::StaticClass();
			for (int64_t i = 0; i < ScriptTokens[sclass].size() - 1; i++)
			{
				if (ScriptTokens[sclass][i].Type == TokenType::Extends)
				{
					sclass->m_ParentClass = StringToClass(ScriptTokens[sclass][i + 1].Value);
					break;
				}
			}
			// Typespecifiers
			for (Token& token : ScriptTokens[sclass])
			{
				if (token.Type == TokenType::Specifier && IsSpecifierATypeSpecifier(token.Value)) 
					token.Type = TokenType::Typespecifier;
			}

			/// TODO: Reflect ScriptVars...

			// Functions
			for (int64_t i = 0; i < ScriptTokens[sclass].size() - 1; i++)
			{
				if (ScriptTokens[sclass][i].Type == TokenType::Function || ScriptTokens[sclass][i].Type == TokenType::Event)
				{
					const bool event = (ScriptTokens[sclass][i].Type == TokenType::Event);
					sclass->m_ScriptClass->m_Functions.push_back(ScriptFunction());
					ScriptFunction& f = sclass->m_ScriptClass->m_Functions[sclass->m_ScriptClass->m_Functions.size() - 1];
					f.m_IsEvent = event;
					/// ....
				}
			}

		}

		// 3. Compile
		for (ScriptClass* sclass : classes)
		{

		}
	}

}