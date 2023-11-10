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

		V& operator[](K&& key)
		{
			return m_Map[key];
		}
		V& At(const K& key)
		{
			return m_Map.at(key);
		}

	};

}