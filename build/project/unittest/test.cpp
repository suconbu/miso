#include "pch.h"

void unittest_read(miso::BinaryReader& reader)
{
    uint8_t buffer[100];
    auto readSize = reader.ReadBlockTo(buffer, sizeof(buffer));
    EXPECT_EQ(9, readSize);
    EXPECT_EQ(0x01, buffer[1]);

    reader.SetPosition(4);
    buffer[0] = buffer[4] = buffer[5] = 0xCC;
    readSize = reader.ReadBlockTo(buffer, sizeof(buffer));
    EXPECT_EQ(readSize, 5);
    EXPECT_EQ(0x67, buffer[0]);
    EXPECT_EQ(0xEF, buffer[4]);
    EXPECT_EQ(0xCC, buffer[5]);
    reader.SetPosition(0);

    auto i1 = reader.Read<int8_t>();
    EXPECT_EQ(0x00, i1);
    EXPECT_EQ(1, reader.GetPosition());
    reader.SetPosition(0);
    auto i2 = reader.Read<int16_t>();
    EXPECT_EQ(0x0100, i2);
    EXPECT_EQ(2, reader.GetPosition());
    reader.SetPosition(0);
    auto i4 = reader.Read<int32_t>();
    EXPECT_EQ(0x45230100UL, i4);
    EXPECT_EQ(4, reader.GetPosition());
    reader.SetPosition(0);
    auto i8 = reader.Read<int64_t>();
    EXPECT_EQ(0xCDAB896745230100ULL, i8);
    EXPECT_EQ(8, reader.GetPosition());
    reader.SetPosition(0);

    reader.SetEndian(miso::Endian::Big);
    EXPECT_EQ(miso::Endian::Big, reader.GetEndian());

    i1 = reader.Read<int8_t>();
    EXPECT_EQ(0x00, i1);
    reader.SetPosition(0);
    i2 = reader.Read<int16_t>();
    EXPECT_EQ(0x0001, i2);
    reader.SetPosition(0);
    i4 = reader.Read<int32_t>();
    EXPECT_EQ(0x00012345UL, i4);
    reader.SetPosition(0);
    i8 = reader.Read<int64_t>();
    EXPECT_EQ(0x000123456789ABCDULL, i8);
    reader.SetPosition(0);

    reader.SetPosition(4);
    i8 = reader.Read<int64_t>();
    EXPECT_EQ(0x0000000000000000ULL, i8);
    EXPECT_FALSE(reader.CanRead());
    i8 = reader.Read<int64_t>();
    EXPECT_EQ(0x0000000000000000ULL, i8);
    EXPECT_FALSE(reader.CanRead());
}

TEST(BinaryReader, SizeAndPosition)
{
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    miso::BinaryReader reader(data, sizeof(data));

    EXPECT_TRUE(reader.CanRead());
    EXPECT_EQ(9, reader.GetLength());
    EXPECT_EQ(0, reader.GetPosition());
    EXPECT_EQ(miso::Endian::Little, reader.GetEndian());

    reader.SetPosition(4);
    EXPECT_EQ(4, reader.GetPosition());
    EXPECT_TRUE(reader.CanRead());
    reader.SetPosition(9);
    EXPECT_EQ(9, reader.GetPosition());
    EXPECT_FALSE(reader.CanRead());
    reader.SetPosition(10);
    EXPECT_EQ(9, reader.GetPosition());
    EXPECT_FALSE(reader.CanRead());
    reader.SetPosition(0);

    reader.Close();

    EXPECT_FALSE(reader.CanRead());
    EXPECT_EQ(0, reader.GetLength());
    EXPECT_EQ(0, reader.GetPosition());
    EXPECT_EQ(miso::Endian::Little, reader.GetEndian());
}

TEST(BinaryReader, Fail)
{
    char buffer[100];
    miso::BinaryReader reader(" ");
    EXPECT_FALSE(reader.CanRead());
    EXPECT_EQ(0, reader.GetLength());
    EXPECT_EQ(0, reader.GetPosition());
    EXPECT_EQ(0, reader.Read<char>());
    EXPECT_EQ(0, reader.ReadBlockTo(buffer, sizeof(buffer)));
}

#include <fstream>
TEST(BinaryReader, FromFile)
{
    miso::BinaryReader reader("test.bin");
    EXPECT_EQ(true, reader.CanRead());
    unittest_read(reader);
}

TEST(BinaryReader, FromMemory)
{
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    miso::BinaryReader reader(data, sizeof(data));
    EXPECT_EQ(true, reader.CanRead());
    unittest_read(reader);
}

TEST(BinaryReader, CloseAfter)
{
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    miso::BinaryReader reader(data, sizeof(data));
    reader.Close();
    EXPECT_EQ(false, reader.CanRead());
}

TEST(BinaryReader, Misc)
{
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    {
        miso::BinaryReader reader(data, sizeof(data));
        uint16_t n = 0;
        size_t count = 0;
        EXPECT_EQ(9, reader.GetLength());
        while (reader.CanRead(sizeof(uint16_t)))
        {
            n = reader.Read<uint16_t>(999);
            count++;
        }
        EXPECT_EQ(0xCDAB, n);
        EXPECT_EQ(4, count);
        EXPECT_EQ(8, reader.GetPosition());
        EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        auto v = reader.ReadBlock(reader.GetLength());
        EXPECT_EQ(9, v.size());
        EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        reader.SetPosition(2);
        auto v = reader.ReadBlock(5);
        EXPECT_EQ(5, v.size());
        EXPECT_EQ(0x23, v[0]);
        EXPECT_EQ(0xAB, v[4]);
        EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        auto v = reader.ReadBlock(100);
        EXPECT_EQ(9, v.size());
        EXPECT_EQ(0x00, v[0]);
        EXPECT_EQ(0xEF, v[8]);
        EXPECT_EQ(true, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        reader.SetPosition(4);
        auto v = reader.Peek<int32_t>();
        EXPECT_EQ(4, reader.GetPosition());
        EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
#pragma pack(1)
        struct {
            char n01;
            short n02;
            int n03;
            long n04;
            long long n05;
            signed char n06;
            unsigned char n07;
            unsigned short n08;
            unsigned int n09;
            unsigned long n10;
            unsigned long long n11;
            float n12;
            double n13;
            long double n14;
            int8_t n15;
            int16_t n16;
            int32_t n17;
            int64_t n18;
            uint8_t n19;
            uint16_t n20;
            uint32_t n21;
            uint64_t n22;
        } combined = {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
            21, 22
        };
#pragma pack()
        miso::BinaryReader reader(&combined, sizeof(combined), miso::Endian::Big);
        EXPECT_EQ( 1, miso::EndianUtils::Flip(reader.Read<char>()));
        EXPECT_EQ( 2, miso::EndianUtils::Flip(reader.Read<short>()));
        EXPECT_EQ( 3, miso::EndianUtils::Flip(reader.Read<int>()));
        EXPECT_EQ( 4, miso::EndianUtils::Flip(reader.Read<long>()));
        EXPECT_EQ( 5, miso::EndianUtils::Flip(reader.Read<long long>()));
        EXPECT_EQ( 6, miso::EndianUtils::Flip(reader.Read<signed char>()));
        EXPECT_EQ( 7, miso::EndianUtils::Flip(reader.Read<unsigned char>()));
        EXPECT_EQ( 8, miso::EndianUtils::Flip(reader.Read<unsigned short>()));
        EXPECT_EQ( 9, miso::EndianUtils::Flip(reader.Read<unsigned int>()));
        EXPECT_EQ(10, miso::EndianUtils::Flip(reader.Read<unsigned long>()));
        EXPECT_EQ(11, miso::EndianUtils::Flip(reader.Read<unsigned long long>()));
        EXPECT_EQ(12, miso::EndianUtils::Flip(reader.Read<float>()));
        EXPECT_EQ(13, miso::EndianUtils::Flip(reader.Read<double>()));
        EXPECT_EQ(14, miso::EndianUtils::Flip(reader.Read<long double>()));
        EXPECT_EQ(15, miso::EndianUtils::Flip(reader.Read<int8_t>()));
        EXPECT_EQ(16, miso::EndianUtils::Flip(reader.Read<int16_t>()));
        EXPECT_EQ(17, miso::EndianUtils::Flip(reader.Read<int32_t>()));
        EXPECT_EQ(18, miso::EndianUtils::Flip(reader.Read<int64_t>()));
        EXPECT_EQ(19, miso::EndianUtils::Flip(reader.Read<uint8_t>()));
        EXPECT_EQ(20, miso::EndianUtils::Flip(reader.Read<uint16_t>()));
        EXPECT_EQ(21, miso::EndianUtils::Flip(reader.Read<uint32_t>()));
        EXPECT_EQ(22, miso::EndianUtils::Flip(reader.Read<uint64_t>()));
        EXPECT_EQ(false, reader.CanRead());
    }
}

TEST(StringUtils, ReadWrite)
{
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    miso::StringUtils::WriteFile("test_string.ignore.txt", str);
    auto output = miso::StringUtils::ReadFile("test_string.ignore.txt");
    EXPECT_EQ(str, output);
}
TEST(StringUtils, SplitJoin)
{
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    auto tokens = miso::StringUtils::Split(str, "<");
    EXPECT_EQ(12, tokens.size());
    auto joined = miso::StringUtils::Join(tokens, "<<<");
    EXPECT_EQ(397, joined.length());
}

TEST(StringUtils, Trim)
{
    auto trimmed = miso::StringUtils::Trim("\t \r \n test \t \r \n ");
    EXPECT_EQ("test", trimmed);
    trimmed = miso::StringUtils::Trim("\t \r \n test");
    EXPECT_EQ("test", trimmed);
    trimmed = miso::StringUtils::Trim("test\t \r \n ");
    EXPECT_EQ("test", trimmed);
    trimmed = miso::StringUtils::Trim("\t \r \n t e\ns\tt \t \r \n ");
    EXPECT_EQ("t e\ns\tt", trimmed);
    trimmed = miso::StringUtils::Trim("test");
    EXPECT_EQ("test", trimmed);
    trimmed = miso::StringUtils::Trim("test", "ts");
    EXPECT_EQ("e", trimmed);
    trimmed = miso::StringUtils::Trim("testtesttest", "est");
    EXPECT_EQ("", trimmed);
}

TEST(StringUtils, Repeat)
{
    EXPECT_EQ("", miso::StringUtils::Repeat("n", 0));
    EXPECT_EQ("n", miso::StringUtils::Repeat("n", 1));
    EXPECT_EQ("nnnnn", miso::StringUtils::Repeat("n", 5));
}

TEST(StringUtils, Replace)
{
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    auto replaced = miso::StringUtils::ReplaceAll(str, "<", "<<<");
    replaced = miso::StringUtils::ReplaceAll(str, "<<<", "!!!!!!");
    replaced = miso::StringUtils::ReplaceAll(str, "!!!!!!", "<");
    EXPECT_EQ(str, replaced);
}

TEST(StringUtils, UpperLower)
{
    auto upper = miso::StringUtils::ToUpper("upper");
    EXPECT_EQ("UPPER", upper);
    auto lower = miso::StringUtils::ToLower("LOWER");
    EXPECT_EQ("lower", lower);
}

TEST(StringUtils, Format)
{
    EXPECT_EQ("0.143:test:9999:0000270F", miso::StringUtils::Format("%.3f:%s:%d:%08X", 1 / 7.0f, "test", 9999, 9999));
}

TEST(StringUtils, Comapare)
{
    EXPECT_EQ(false, miso::StringUtils::StartsWith("", "Tokyo"));
    EXPECT_EQ(false, miso::StringUtils::StartsWith("Toky", "Tokyo"));
    EXPECT_EQ(false, miso::StringUtils::StartsWith(" Tokyo", "Tokyo"));
    EXPECT_EQ(true, miso::StringUtils::StartsWith("Tokyo", "Tokyo"));
    EXPECT_EQ(true, miso::StringUtils::StartsWith("TokyoStation", "Tokyo"));

    EXPECT_EQ(false, miso::StringUtils::EndsWith("", "Station"));
    EXPECT_EQ(false, miso::StringUtils::EndsWith("tation", "Station"));
    EXPECT_EQ(false, miso::StringUtils::EndsWith("Station ", "Station"));
    EXPECT_EQ(true, miso::StringUtils::EndsWith("Station", "Station"));
    EXPECT_EQ(true, miso::StringUtils::EndsWith("TokyoStation", "Station"));

    EXPECT_EQ(false, miso::StringUtils::Contains("", "Station"));
    EXPECT_EQ(false, miso::StringUtils::Contains("tation", "Station"));
    EXPECT_EQ(true, miso::StringUtils::Contains("Station ", "Station"));
    EXPECT_EQ(true, miso::StringUtils::Contains("TokyoStation", "Station"));
    EXPECT_EQ(true, miso::StringUtils::Contains("TokyoStationHotel", "Station"));
}

TEST(StringUtils, Slice)
{
    EXPECT_EQ("tokyo", miso::StringUtils::Slice("tokyo", 0));
    EXPECT_EQ("yo", miso::StringUtils::Slice("tokyo", 3));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 5));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 6));
    EXPECT_EQ("o", miso::StringUtils::Slice("tokyo", -1));
    EXPECT_EQ("tokyo", miso::StringUtils::Slice("tokyo", -5));
    EXPECT_EQ("tokyo", miso::StringUtils::Slice("tokyo", -6));

    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 0, 0));
    EXPECT_EQ("t", miso::StringUtils::Slice("tokyo", 0, 1));
    EXPECT_EQ("tokyo", miso::StringUtils::Slice("tokyo", 0, 5));
    EXPECT_EQ("tokyo", miso::StringUtils::Slice("tokyo", 0, 6));
    EXPECT_EQ("toky", miso::StringUtils::Slice("tokyo", 0, -1));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 0, -5));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 0, -6));

    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 5, 5));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 5, 6));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 5, 0));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", 5, -1));

    EXPECT_EQ("yo", miso::StringUtils::Slice("tokyo", -2, 5));
    EXPECT_EQ("yo", miso::StringUtils::Slice("tokyo", -2, 6));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", -2, 0));
    EXPECT_EQ("y", miso::StringUtils::Slice("tokyo", -2, -1));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", -2, -2));
    EXPECT_EQ("", miso::StringUtils::Slice("tokyo", -2, -3));
}
