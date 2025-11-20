#include "Serialization/Serialization.hpp"
#include <iostream>

namespace Serialization
{
    // SerializationContext implementations
    void SerializationContext::WriteInt(const std::string& key, int value)
    {
        data << "\"" << key << "\": " << value << ",\n";
    }

    void SerializationContext::WriteFloat(const std::string& key, float value)
    {
        data << "\"" << key << "\": " << value << ",\n";
    }

    void SerializationContext::WriteString(const std::string& key, const std::string& value)
    {
        data << "\"" << key << "\": \"" << value << "\",\n";
    }

    void SerializationContext::WriteBool(const std::string& key, bool value)
    {
        data << "\"" << key << "\": " << (value ? "true" : "false") << ",\n";
    }

    template<typename T>
    void SerializationContext::WriteVector(const std::string& key, const std::vector<T>& vec)
    {
        data << "\"" << key << "\": [\n";
        for (size_t i = 0; i < vec.size(); ++i)
        {
            // Serialize element
            SerializationContext elemCtx;
            Serializer<T>::Serialize(elemCtx, vec[i]);
            data << elemCtx.ToString();
            if (i < vec.size() - 1) data << ",";
            data << "\n";
        }
        data << "],\n";
    }

    template<typename K, typename V>
    void SerializationContext::WriteMap(const std::string& key, const std::map<K, V>& map)
    {
        data << "\"" << key << "\": {\n";
        size_t count = 0;
        for (const auto& [k, v] : map)
        {
            // Serialize key-value pair
            SerializationContext keyCtx, valCtx;
            Serializer<K>::Serialize(keyCtx, k);
            Serializer<V>::Serialize(valCtx, v);
            data << keyCtx.ToString() << ": " << valCtx.ToString();
            if (count < map.size() - 1) data << ",";
            data << "\n";
            ++count;
        }
        data << "},\n";
    }

    // DeserializationContext implementations
    bool DeserializationContext::ReadInt(const std::string& key, int& value)
    {
        if (!FindKey(key)) return false;
        std::string val = ReadValue();
        value = std::stoi(val);
        return true;
    }

    bool DeserializationContext::ReadFloat(const std::string& key, float& value)
    {
        if (!FindKey(key)) return false;
        std::string val = ReadValue();
        value = std::stof(val);
        return true;
    }

    bool DeserializationContext::ReadString(const std::string& key, std::string& value)
    {
        if (!FindKey(key)) return false;
        value = ReadValue();
        // Remove quotes
        if (!value.empty() && value[0] == '\"' && value[value.size()-1] == '\"')
        {
            value = value.substr(1, value.size() - 2);
        }
        return true;
    }

    bool DeserializationContext::ReadBool(const std::string& key, bool& value)
    {
        if (!FindKey(key)) return false;
        std::string val = ReadValue();
        value = (val == "true");
        return true;
    }

    template<typename T>
    bool DeserializationContext::ReadVector(const std::string& key, std::vector<T>& vec)
    {
        if (!FindKey(key)) return false;
        // Simple implementation - would need proper JSON parsing for production
        vec.clear();
        return true;
    }

    template<typename K, typename V>
    bool DeserializationContext::ReadMap(const std::string& key, std::map<K, V>& map)
    {
        if (!FindKey(key)) return false;
        // Simple implementation - would need proper JSON parsing for production
        map.clear();
        return true;
    }

    bool DeserializationContext::FindKey(const std::string& key)
    {
        std::string search = "\"" + key + "\":";
        size_t found = content.find(search, pos);
        if (found != std::string::npos)
        {
            pos = found + search.length();
            return true;
        }
        return false;
    }

    std::string DeserializationContext::ReadValue()
    {
        // Skip whitespace
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n'))
        {
            ++pos;
        }

        std::string value;
        bool inString = false;

        while (pos < content.size())
        {
            char c = content[pos];

            if (c == '\"')
            {
                inString = !inString;
                value += c;
            }
            else if (!inString && (c == ',' || c == '}' || c == ']'))
            {
                break;
            }
            else
            {
                value += c;
            }

            ++pos;
        }

        return value;
    }

    // File I/O helpers
    bool SaveToFile(const std::string& filename, const std::string& data)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        file << data;
        file.close();
        return true;
    }

    bool LoadFromFile(const std::string& filename, std::string& data)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for reading: " << filename << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        data = buffer.str();
        file.close();
        return true;
    }
}
