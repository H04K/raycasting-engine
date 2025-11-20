#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <type_traits>

// Simple JSON-like serialization system
// Supports std::vector, std::map, std::string, and basic types

namespace Serialization
{
    // Forward declarations
    class SerializationContext;
    class DeserializationContext;

    // Serialization interface
    template<typename T>
    struct Serializer
    {
        static void Serialize(SerializationContext& ctx, const T& value);
        static void Deserialize(DeserializationContext& ctx, T& value);
    };

    // Serialization context
    class SerializationContext
    {
    public:
        SerializationContext() = default;

        void WriteInt(const std::string& key, int value);
        void WriteFloat(const std::string& key, float value);
        void WriteString(const std::string& key, const std::string& value);
        void WriteBool(const std::string& key, bool value);

        template<typename T>
        void WriteVector(const std::string& key, const std::vector<T>& vec);

        template<typename K, typename V>
        void WriteMap(const std::string& key, const std::map<K, V>& map);

        std::string ToString() const { return data.str(); }
        void Reset() { data.str(""); data.clear(); }

    private:
        std::ostringstream data;
        int indentLevel = 0;
    };

    // Deserialization context
    class DeserializationContext
    {
    public:
        explicit DeserializationContext(const std::string& content) : content(content), pos(0) {}

        bool ReadInt(const std::string& key, int& value);
        bool ReadFloat(const std::string& key, float& value);
        bool ReadString(const std::string& key, std::string& value);
        bool ReadBool(const std::string& key, bool& value);

        template<typename T>
        bool ReadVector(const std::string& key, std::vector<T>& vec);

        template<typename K, typename V>
        bool ReadMap(const std::string& key, std::map<K, V>& map);

    private:
        std::string content;
        size_t pos;

        bool FindKey(const std::string& key);
        std::string ReadValue();
    };

    // Serialization for basic types
    template<>
    struct Serializer<int>
    {
        static void Serialize(SerializationContext& ctx, const int& value)
        {
            ctx.WriteInt("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, int& value)
        {
            ctx.ReadInt("value", value);
        }
    };

    template<>
    struct Serializer<float>
    {
        static void Serialize(SerializationContext& ctx, const float& value)
        {
            ctx.WriteFloat("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, float& value)
        {
            ctx.ReadFloat("value", value);
        }
    };

    template<>
    struct Serializer<std::string>
    {
        static void Serialize(SerializationContext& ctx, const std::string& value)
        {
            ctx.WriteString("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, std::string& value)
        {
            ctx.ReadString("value", value);
        }
    };

    template<>
    struct Serializer<bool>
    {
        static void Serialize(SerializationContext& ctx, const bool& value)
        {
            ctx.WriteBool("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, bool& value)
        {
            ctx.ReadBool("value", value);
        }
    };

    // Serialization for std::vector
    template<typename T>
    struct Serializer<std::vector<T>>
    {
        static void Serialize(SerializationContext& ctx, const std::vector<T>& value)
        {
            ctx.WriteVector("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, std::vector<T>& value)
        {
            ctx.ReadVector("value", value);
        }
    };

    // Serialization for std::map
    template<typename K, typename V>
    struct Serializer<std::map<K, V>>
    {
        static void Serialize(SerializationContext& ctx, const std::map<K, V>& value)
        {
            ctx.WriteMap("value", value);
        }
        static void Deserialize(DeserializationContext& ctx, std::map<K, V>& value)
        {
            ctx.ReadMap("value", value);
        }
    };

    // Helper functions for saving/loading files
    bool SaveToFile(const std::string& filename, const std::string& data);
    bool LoadFromFile(const std::string& filename, std::string& data);
}
