#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <cstring>

// Binary serialization system - efficient binary format

namespace Serialization
{
    // Binary writer
    class BinaryWriter
    {
    public:
        BinaryWriter() = default;

        // Write primitive types
        void WriteInt8(int8_t value);
        void WriteInt16(int16_t value);
        void WriteInt32(int32_t value);
        void WriteInt64(int64_t value);
        void WriteUInt8(uint8_t value);
        void WriteUInt16(uint16_t value);
        void WriteUInt32(uint32_t value);
        void WriteUInt64(uint64_t value);
        void WriteFloat(float value);
        void WriteDouble(double value);
        void WriteBool(bool value);
        void WriteString(const std::string& value);

        // Write containers
        template<typename T>
        void WriteVector(const std::vector<T>& vec)
        {
            WriteUInt32(static_cast<uint32_t>(vec.size()));
            for (const auto& item : vec)
            {
                WriteValue(item);
            }
        }

        template<typename K, typename V>
        void WriteMap(const std::unordered_map<K, V>& map)
        {
            WriteUInt32(static_cast<uint32_t>(map.size()));
            for (const auto& [key, value] : map)
            {
                WriteValue(key);
                WriteValue(value);
            }
        }

        // Generic write
        template<typename T>
        void WriteValue(const T& value)
        {
            if constexpr (std::is_same_v<T, int8_t>) WriteInt8(value);
            else if constexpr (std::is_same_v<T, int16_t>) WriteInt16(value);
            else if constexpr (std::is_same_v<T, int32_t>) WriteInt32(value);
            else if constexpr (std::is_same_v<T, int64_t>) WriteInt64(value);
            else if constexpr (std::is_same_v<T, uint8_t>) WriteUInt8(value);
            else if constexpr (std::is_same_v<T, uint16_t>) WriteUInt16(value);
            else if constexpr (std::is_same_v<T, uint32_t>) WriteUInt32(value);
            else if constexpr (std::is_same_v<T, uint64_t>) WriteUInt64(value);
            else if constexpr (std::is_same_v<T, float>) WriteFloat(value);
            else if constexpr (std::is_same_v<T, double>) WriteDouble(value);
            else if constexpr (std::is_same_v<T, bool>) WriteBool(value);
            else if constexpr (std::is_same_v<T, std::string>) WriteString(value);
        }

        // Get data
        const std::vector<uint8_t>& GetData() const { return data; }
        void Clear() { data.clear(); }
        size_t Size() const { return data.size(); }

        // File operations
        bool SaveToFile(const std::string& filename) const;

    private:
        std::vector<uint8_t> data;

        template<typename T>
        void WriteRaw(const T& value)
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
            data.insert(data.end(), bytes, bytes + sizeof(T));
        }
    };

    // Binary reader
    class BinaryReader
    {
    public:
        explicit BinaryReader(const std::vector<uint8_t>& data) : data(data), pos(0) {}

        // Read primitive types
        bool ReadInt8(int8_t& value);
        bool ReadInt16(int16_t& value);
        bool ReadInt32(int32_t& value);
        bool ReadInt64(int64_t& value);
        bool ReadUInt8(uint8_t& value);
        bool ReadUInt16(uint16_t& value);
        bool ReadUInt32(uint32_t& value);
        bool ReadUInt64(uint64_t& value);
        bool ReadFloat(float& value);
        bool ReadDouble(double& value);
        bool ReadBool(bool& value);
        bool ReadString(std::string& value);

        // Read containers
        template<typename T>
        bool ReadVector(std::vector<T>& vec)
        {
            uint32_t size;
            if (!ReadUInt32(size)) return false;

            vec.clear();
            vec.reserve(size);

            for (uint32_t i = 0; i < size; ++i)
            {
                T item;
                if (!ReadValue(item)) return false;
                vec.push_back(item);
            }

            return true;
        }

        template<typename K, typename V>
        bool ReadMap(std::unordered_map<K, V>& map)
        {
            uint32_t size;
            if (!ReadUInt32(size)) return false;

            map.clear();

            for (uint32_t i = 0; i < size; ++i)
            {
                K key;
                V value;
                if (!ReadValue(key) || !ReadValue(value)) return false;
                map[key] = value;
            }

            return true;
        }

        // Generic read
        template<typename T>
        bool ReadValue(T& value)
        {
            if constexpr (std::is_same_v<T, int8_t>) return ReadInt8(value);
            else if constexpr (std::is_same_v<T, int16_t>) return ReadInt16(value);
            else if constexpr (std::is_same_v<T, int32_t>) return ReadInt32(value);
            else if constexpr (std::is_same_v<T, int64_t>) return ReadInt64(value);
            else if constexpr (std::is_same_v<T, uint8_t>) return ReadUInt8(value);
            else if constexpr (std::is_same_v<T, uint16_t>) return ReadUInt16(value);
            else if constexpr (std::is_same_v<T, uint32_t>) return ReadUInt32(value);
            else if constexpr (std::is_same_v<T, uint64_t>) return ReadUInt64(value);
            else if constexpr (std::is_same_v<T, float>) return ReadFloat(value);
            else if constexpr (std::is_same_v<T, double>) return ReadDouble(value);
            else if constexpr (std::is_same_v<T, bool>) return ReadBool(value);
            else if constexpr (std::is_same_v<T, std::string>) return ReadString(value);
            return false;
        }

        // Position management
        size_t GetPosition() const { return pos; }
        void SetPosition(size_t position) { pos = position; }
        bool IsAtEnd() const { return pos >= data.size(); }

        // File operations
        static bool LoadFromFile(const std::string& filename, std::vector<uint8_t>& data);

    private:
        const std::vector<uint8_t>& data;
        size_t pos;

        template<typename T>
        bool ReadRaw(T& value)
        {
            if (pos + sizeof(T) > data.size()) return false;
            std::memcpy(&value, data.data() + pos, sizeof(T));
            pos += sizeof(T);
            return true;
        }
    };
}
