#pragma once
#include <unordered_map>
#include <iterator>

namespace Suora
{

	template<class K, class V>
	class Map
	{
		std::unordered_map<K, V> m_Map;
	public:

		bool ContainsKey(const K& key) const
		{
			return m_Map.find(key) != m_Map.end();
		}

		void Remove(const K& key)
		{
			m_Map.erase(key);
		}

		V& operator[](const K& key)
		{
			return m_Map[key];
		}
		V& operator[](K&& key)
		{
			return m_Map[key];
		}
		const V& At(const K& key)
		{
			return m_Map.at(key);
		}
		V GetOrDefault(const K& key, V&& InDefault)
		{
			return ContainsKey(key) ? At(key) : InDefault;
		}

		void Clear()
		{
			m_Map.clear();
		}

		size_t Size()
		{
			return m_Map.size();
		}

		// Iterators

		typename std::unordered_map<K, V>::iterator begin() { return m_Map.begin(); }
		typename std::unordered_map<K, V>::const_iterator begin() const { return m_Map.begin(); }
		typename std::unordered_map<K, V>::const_iterator cbegin() const { return m_Map.cbegin(); }
		typename std::unordered_map<K, V>::iterator end() { return m_Map.end(); }
		typename std::unordered_map<K, V>::const_iterator end() const { return m_Map.end(); }
		typename std::unordered_map<K, V>::const_iterator cend() const { return m_Map.cend(); }
	};

}