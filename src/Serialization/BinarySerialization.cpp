#include "Serialization/BinarySerialization.hpp"
#include <iostream>

namespace Serialization
{
    // BinaryWriter implementations
    void BinaryWriter::WriteInt8(int8_t value) { WriteRaw(value); }
    void BinaryWriter::WriteInt16(int16_t value) { WriteRaw(value); }
    void BinaryWriter::WriteInt32(int32_t value) { WriteRaw(value); }
    void BinaryWriter::WriteInt64(int64_t value) { WriteRaw(value); }
    void BinaryWriter::WriteUInt8(uint8_t value) { WriteRaw(value); }
    void BinaryWriter::WriteUInt16(uint16_t value) { WriteRaw(value); }
    void BinaryWriter::WriteUInt32(uint32_t value) { WriteRaw(value); }
    void BinaryWriter::WriteUInt64(uint64_t value) { WriteRaw(value); }
    void BinaryWriter::WriteFloat(float value) { WriteRaw(value); }
    void BinaryWriter::WriteDouble(double value) { WriteRaw(value); }
    void BinaryWriter::WriteBool(bool value) { WriteUInt8(value ? 1 : 0); }

    void BinaryWriter::WriteString(const std::string& value)
    {
        WriteUInt32(static_cast<uint32_t>(value.size()));
        data.insert(data.end(), value.begin(), value.end());
    }

    bool BinaryWriter::SaveToFile(const std::string& filename) const
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to save binary file: " << filename << std::endl;
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        return true;
    }

    // BinaryReader implementations
    bool BinaryReader::ReadInt8(int8_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadInt16(int16_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadInt32(int32_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadInt64(int64_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadUInt8(uint8_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadUInt16(uint16_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadUInt32(uint32_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadUInt64(uint64_t& value) { return ReadRaw(value); }
    bool BinaryReader::ReadFloat(float& value) { return ReadRaw(value); }
    bool BinaryReader::ReadDouble(double& value) { return ReadRaw(value); }

    bool BinaryReader::ReadBool(bool& value)
    {
        uint8_t byte;
        if (!ReadUInt8(byte)) return false;
        value = (byte != 0);
        return true;
    }

    bool BinaryReader::ReadString(std::string& value)
    {
        uint32_t size;
        if (!ReadUInt32(size)) return false;
        if (pos + size > data.size()) return false;

        value.assign(reinterpret_cast<const char*>(data.data() + pos), size);
        pos += size;
        return true;
    }

    bool BinaryReader::LoadFromFile(const std::string& filename, std::vector<uint8_t>& data)
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            std::cerr << "Failed to load binary file: " << filename << std::endl;
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        file.close();
        return true;
    }
}
