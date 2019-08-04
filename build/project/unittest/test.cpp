#include "pch.h"

_CrtMemState G_MEMORY_STATE = { 0 };

class MisoTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtMemCheckpoint(&G_MEMORY_STATE);
    }
    virtual void TearDown()
    {
        _CrtMemDumpAllObjectsSince(&G_MEMORY_STATE);
    }
};

void unittest_read(miso::BinaryReader &reader)
{
    uint8_t buffer[100];
    auto read_size = reader.ReadBlock(buffer, sizeof(buffer));
    EXPECT_EQ(reader.GetSize(), read_size);
    EXPECT_EQ(0x01, buffer[1]);

    reader.SetPosition(4);
    buffer[0] = buffer[4] = buffer[5] = 0xCC;
    read_size = reader.ReadBlock(buffer, sizeof(buffer));
    EXPECT_EQ(read_size, 5);
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

TEST(Test, Initialize)
{
    // Create a local static instance for avoid false detection of memory leak.
    // miso::Scalar::GetUnitToSuffixMap
    miso::Scalar a("0%");
}

TEST_F(MisoTest, BinaryReader_SizeAndPosition)
{
    TEST_TRACE("");
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    miso::BinaryReader reader(data, sizeof(data));

    EXPECT_TRUE(reader.CanRead());
    EXPECT_EQ(9, reader.GetSize());
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

    // reader.Close();

    // EXPECT_FALSE(reader.CanRead());
    // EXPECT_EQ(0, reader.GetSize());
    // EXPECT_EQ(0, reader.GetPosition());
    // EXPECT_EQ(miso::Endian::Little, reader.GetEndian());
}

TEST_F(MisoTest, BinaryReader_Fail)
{
    TEST_TRACE("");
    char buffer[100];
    miso::BinaryReader reader(" ");
    EXPECT_FALSE(reader.CanRead());
    EXPECT_EQ(0, reader.GetSize());
    EXPECT_EQ(0, reader.GetPosition());
    EXPECT_EQ(0, reader.Read<char>());
    EXPECT_EQ(0, reader.ReadBlock(buffer, sizeof(buffer)));
}

TEST_F(MisoTest, BinaryReader_FromFile)
{
    TEST_TRACE("");
    miso::BinaryReader reader("test.bin");
    EXPECT_EQ(true, reader.CanRead());
    unittest_read(reader);
}

TEST_F(MisoTest, BinaryReader_FromMemory)
{
    TEST_TRACE("");
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    miso::BinaryReader reader(data, sizeof(data));
    EXPECT_EQ(true, reader.CanRead());
    unittest_read(reader);
}

// TEST_F(MisoTest, BinaryReader_CloseAfter)
//{
//    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
//    miso::BinaryReader reader(data, sizeof(data));
//    reader.Close();
//    EXPECT_EQ(false, reader.CanRead());
//}

TEST_F(MisoTest, BinaryReader_Misc)
{
    TEST_TRACE("");
    const uint8_t data[] = { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    {
        miso::BinaryReader reader(data, sizeof(data));
        uint16_t n = 0;
        size_t count = 0;
        EXPECT_EQ(9, reader.GetSize());
        while (reader.CanRead(sizeof(uint16_t))) {
            n = reader.Read<uint16_t>(999);
            count++;
        }
        EXPECT_EQ(0xCDAB, n);
        EXPECT_EQ(4, count);
        EXPECT_EQ(8, reader.GetPosition());
        // EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        auto buffer = reader.ReadBlock(reader.GetSize());
        EXPECT_EQ(9, buffer.GetSize());
        // EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        reader.SetPosition(2);
        auto buffer = reader.ReadBlock(5);
        EXPECT_EQ(5, buffer.GetSize());
        EXPECT_EQ(0x23, buffer[0]);
        EXPECT_EQ(0xAB, buffer[4]);
        // EXPECT_EQ(false, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        auto buffer = reader.ReadBlock(100);
        EXPECT_EQ(9, buffer.GetSize());
        EXPECT_EQ(0x00, buffer[0]);
        EXPECT_EQ(0xEF, buffer[8]);
        // EXPECT_EQ(true, reader.IsOverrunOccurred());
    }
    {
        miso::BinaryReader reader(data, sizeof(data));
        reader.SetPosition(4);
        auto v = reader.Peek<int32_t>();
        EXPECT_EQ(4, reader.GetPosition());
        // EXPECT_EQ(false, reader.IsOverrunOccurred());
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
        } combined = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                      12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 };
#pragma pack()
        miso::BinaryReader reader(&combined, sizeof(combined), miso::Endian::Big);
        EXPECT_EQ(1, miso::EndianUtils::Flip(reader.Read<char>()));
        EXPECT_EQ(2, miso::EndianUtils::Flip(reader.Read<short>()));
        EXPECT_EQ(3, miso::EndianUtils::Flip(reader.Read<int>()));
        EXPECT_EQ(4, miso::EndianUtils::Flip(reader.Read<long>()));
        EXPECT_EQ(5, miso::EndianUtils::Flip(reader.Read<long long>()));
        EXPECT_EQ(6, miso::EndianUtils::Flip(reader.Read<signed char>()));
        EXPECT_EQ(7, miso::EndianUtils::Flip(reader.Read<unsigned char>()));
        EXPECT_EQ(8, miso::EndianUtils::Flip(reader.Read<unsigned short>()));
        EXPECT_EQ(9, miso::EndianUtils::Flip(reader.Read<unsigned int>()));
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

TEST_F(MisoTest, StringUtils_ReadWrite)
{
    TEST_TRACE("");
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    miso::StringUtils::WriteFile("test_string.ignore.txt", str);
    auto output = miso::StringUtils::ReadFile("test_string.ignore.txt");
    EXPECT_EQ(str, output);
}

TEST_F(MisoTest, StringUtils_Split)
{
    auto t = miso::StringUtils::Split("1,2,3", ",");
    EXPECT_EQ(3, t.size());
    t = miso::StringUtils::Split(",1,2,3", ",");
    EXPECT_EQ(4, t.size());
    t = miso::StringUtils::Split("1,2,3,", ",");
    EXPECT_EQ(4, t.size());
    t = miso::StringUtils::Split("1,,2,3", ",");
    EXPECT_EQ(4, t.size());
    t = miso::StringUtils::Split("1,,2,3", ",", true);
    EXPECT_EQ(3, t.size());
    t = miso::StringUtils::Split(",", ",");
    EXPECT_EQ(2, t.size());
    t = miso::StringUtils::Split(",", ",", true);
    EXPECT_EQ(0, t.size());
    t = miso::StringUtils::Split("", ",");
    EXPECT_EQ(1, t.size());
    t = miso::StringUtils::Split("", ",", true);
    EXPECT_EQ(0, t.size());
    t = miso::StringUtils::Split("", "");
    EXPECT_EQ(1, t.size());
    t = miso::StringUtils::Split("", "", true);
    EXPECT_EQ(0, t.size());
    t = miso::StringUtils::Split("1,2", "");
    EXPECT_EQ(1, t.size());
}

TEST_F(MisoTest, StringUtils_SplitJoin)
{
    TEST_TRACE("");
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    auto tokens = miso::StringUtils::Split(str, "<");
    EXPECT_EQ(12, tokens.size());
    auto joined = miso::StringUtils::Join(tokens, "<<<");
    EXPECT_EQ(397, joined.length());
}

TEST_F(MisoTest, StringUtils_Trim)
{
    TEST_TRACE("");
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

TEST_F(MisoTest, StringUtils_Repeat)
{
    TEST_TRACE("");
    EXPECT_EQ("", miso::StringUtils::Repeat("n", 0));
    EXPECT_EQ("n", miso::StringUtils::Repeat("n", 1));
    EXPECT_EQ("nnnnn", miso::StringUtils::Repeat("n", 5));
}

TEST_F(MisoTest, StringUtils_Replace)
{
    TEST_TRACE("");
    auto str = miso::StringUtils::ReadFile("test_string.txt");
    auto replaced = miso::StringUtils::ReplaceAll(str, "<", "<<<");
    replaced = miso::StringUtils::ReplaceAll(str, "<<<", "!!!!!!");
    replaced = miso::StringUtils::ReplaceAll(str, "!!!!!!", "<");
    EXPECT_EQ(str, replaced);
}

TEST_F(MisoTest, StringUtils_UpperLower)
{
    TEST_TRACE("");
    auto upper = miso::StringUtils::ToUpper("upper");
    EXPECT_EQ("UPPER", upper);
    auto lower = miso::StringUtils::ToLower("LOWER");
    EXPECT_EQ("lower", lower);
}

TEST_F(MisoTest, StringUtils_Format)
{
    TEST_TRACE("");
    EXPECT_EQ("0.143:test:9999:0000270F",
        miso::StringUtils::Format("%.3f:%s:%d:%08X", 1 / 7.0f, "test", 9999, 9999));
}

TEST_F(MisoTest, StringUtils_Comapare)
{
    TEST_TRACE("");
    EXPECT_FALSE(miso::StringUtils::StartsWith("", "Tokyo"));
    EXPECT_FALSE(miso::StringUtils::StartsWith("Toky", "Tokyo"));
    EXPECT_FALSE(miso::StringUtils::StartsWith(" Tokyo", "Tokyo"));
    EXPECT_TRUE(miso::StringUtils::StartsWith("Tokyo", "Tokyo"));
    EXPECT_TRUE(miso::StringUtils::StartsWith("TokyoStation", "Tokyo"));

    EXPECT_FALSE(miso::StringUtils::EndsWith("", "Station"));
    EXPECT_FALSE(miso::StringUtils::EndsWith("tation", "Station"));
    EXPECT_FALSE(miso::StringUtils::EndsWith("Station ", "Station"));
    EXPECT_TRUE(miso::StringUtils::EndsWith("Station", "Station"));
    EXPECT_TRUE(miso::StringUtils::EndsWith("TokyoStation", "Station"));

    EXPECT_FALSE(miso::StringUtils::Contains("", "Station"));
    EXPECT_FALSE(miso::StringUtils::Contains("tation", "Station"));
    EXPECT_TRUE(miso::StringUtils::Contains("Station ", "Station"));
    EXPECT_TRUE(miso::StringUtils::Contains("TokyoStation", "Station"));
    EXPECT_TRUE(miso::StringUtils::Contains("TokyoStationHotel", "Station"));

    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("StatioN", "station"));
    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("station", "Station"));
    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("station", "STATIC", 3));
    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("station", "STA", 3));
    EXPECT_TRUE(0 < miso::StringUtils::CompareIgnoreCase("station", "ST", 3));
    EXPECT_TRUE(0 < miso::StringUtils::CompareIgnoreCase("station", "S", 3));
    EXPECT_TRUE(0 < miso::StringUtils::CompareIgnoreCase("station", "", 3));
    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("", "", 3));
    EXPECT_TRUE(0 > miso::StringUtils::CompareIgnoreCase("", "Station", 3));
    EXPECT_TRUE(0 > miso::StringUtils::CompareIgnoreCase("s", "Station", 3));
    EXPECT_TRUE(0 > miso::StringUtils::CompareIgnoreCase("st", "Station", 3));
    EXPECT_TRUE(0 == miso::StringUtils::CompareIgnoreCase("sta", "Station", 3));
}

TEST_F(MisoTest, StringUtils_Slice)
{
    TEST_TRACE("");
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

TEST_F(MisoTest, Buffer_Construct)
{
    TEST_TRACE("");
    miso::Buffer<> a(100);
    EXPECT_EQ(100, a.GetSize());
    miso::Buffer<> b(a);
    EXPECT_EQ(100, b.GetSize());
    EXPECT_TRUE(a.GetPointer() != b.GetPointer());
    a[1] = 10;
    a[2] = 20;
    {
        miso::Buffer<> c(a);
        miso::Buffer<> d = a;
        EXPECT_EQ(10, a[1]);
        EXPECT_EQ(20, a[2]);
        unsigned char *p = d.GetPointer();
        c[1] = 11;
        p[2] = 22;
    }
    EXPECT_EQ(10, a[1]);
    EXPECT_EQ(20, a[2]);
}
TEST_F(MisoTest, Buffer_Resize)
{
    TEST_TRACE("");
    unsigned char x[] = { 0, 10, 20 };
    miso::Buffer<> a(x, sizeof(x));
    a.Resize(5);
    EXPECT_EQ(20, a[2]);

    miso::Buffer<> b(x, sizeof(x));
    b.Resize(5, false);
    EXPECT_TRUE(20 != b[2]);
}
TEST_F(MisoTest, Buffer_Empty)
{
    TEST_TRACE("");
    miso::Buffer<> a;
    EXPECT_EQ(true, a.IsEmpty());
    EXPECT_EQ(0, a.GetSize());
    EXPECT_EQ(nullptr, a.GetPointer());
    a.Resize(100);
    EXPECT_EQ(false, a.IsEmpty());
    EXPECT_EQ(100, a.GetSize());
}

TEST_F(MisoTest, XmlReader_Normal)
{
    TEST_TRACE("");

    miso::XmlReader reader("test.xml");
    EXPECT_EQ(true, reader.CanRead());

    EXPECT_EQ("", reader.GetElementName());
    EXPECT_EQ("", reader.GetAttributeValueString("id"));
    EXPECT_EQ("", reader.GetContentText());
    EXPECT_EQ(0, reader.GetNestingLevel());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::StartElement, reader.GetNodeType());
    EXPECT_EQ("root", reader.GetElementName());
    EXPECT_EQ("root_name", reader.GetAttributeValueString("name"));
    EXPECT_EQ("", reader.GetAttributeValueString("type"));
    EXPECT_EQ("", reader.GetContentText());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::EmptyElement, reader.GetNodeType());
    EXPECT_EQ("element", reader.GetElementName());
    EXPECT_EQ("element1", reader.GetAttributeValueString("id"));
    EXPECT_EQ("", reader.GetAttributeValueString("name"));
    EXPECT_EQ("", reader.GetContentText());
    EXPECT_EQ(1, reader.GetNestingLevel());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::StartElement, reader.GetNodeType());
    EXPECT_EQ("element", reader.GetElementName());
    EXPECT_EQ("element2", reader.GetAttributeValueString("id"));
    EXPECT_EQ("element2_name", reader.GetAttributeValueString("name"));
    auto attributes = reader.GetAllAttributes();
    EXPECT_EQ(2, attributes.size());
    EXPECT_EQ("id", attributes[0].GetName());
    EXPECT_EQ("element2", attributes[0].GetValue());
    EXPECT_EQ("name", attributes[1].GetName());
    EXPECT_EQ("element2_name", attributes[1].GetValue());
    EXPECT_EQ("", reader.GetContentText());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::Text, reader.GetNodeType());
    EXPECT_EQ("", reader.GetElementName());
    EXPECT_EQ("", reader.GetAttributeValueString("id"));
    EXPECT_EQ("\n    TEXT1\n    ", reader.GetContentText());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::StartElement, reader.GetNodeType());
    EXPECT_EQ("sub-element", reader.GetElementName());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::Text, reader.GetNodeType());
    EXPECT_EQ("TEXT2", reader.GetContentText());
    EXPECT_EQ(3, reader.GetNestingLevel());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
    EXPECT_EQ("sub-element", reader.GetElementName());
    EXPECT_EQ(2, reader.GetNestingLevel());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::Text, reader.GetNodeType());
    EXPECT_EQ("\n    TEXT3\n  ", reader.GetContentText());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
    EXPECT_EQ("element", reader.GetElementName());

    EXPECT_EQ(true, reader.Read());
    EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
    EXPECT_EQ("root", reader.GetElementName());
    EXPECT_EQ(0, reader.GetNestingLevel());

    EXPECT_EQ(false, reader.Read());
    EXPECT_EQ(false, reader.CanRead());
    EXPECT_EQ(false, reader.HasError());
}

TEST_F(MisoTest, XmlReader_MoveToElement)
{
    TEST_TRACE("");
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(true, reader.MoveToElement());
        EXPECT_EQ("root", reader.GetElementName());
        EXPECT_EQ(true, reader.MoveToElement("sub-element"));
        EXPECT_EQ("sub", reader.GetAttributeValueString("type"));
        EXPECT_EQ(miso::XmlNodeType::StartElement, reader.GetNodeType());
        EXPECT_EQ(false, reader.MoveToElement("element"));
        EXPECT_EQ(false, reader.CanRead());
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(true, reader.MoveToElement("element"));
        EXPECT_EQ(miso::XmlNodeType::EmptyElement, reader.GetNodeType());
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(true, reader.MoveToElement("element", "name"));
        EXPECT_EQ("element2", reader.GetAttributeValueString("id"));
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(true, reader.MoveToElement("element", "id", "element2"));
        EXPECT_EQ("element2", reader.GetAttributeValueString("id"));
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(true, reader.MoveToElement(nullptr, "type", "sub"));
        EXPECT_EQ("sub", reader.GetAttributeValueString("type"));
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(false, reader.MoveToElement(nullptr, "hoge"));
    }
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(false, reader.MoveToElement(nullptr, "name", "element1_name"));
    }
    {
        // XmlNodeType::Text���猟��
        miso::XmlReader reader("test.xml");
        reader.MoveToElement("element", "id", "element2");
        reader.Read();
        EXPECT_EQ(true, reader.MoveToElement(nullptr, "type", "sub"));
        EXPECT_EQ("sub", reader.GetAttributeValueString("type"));
    }
}

TEST_F(MisoTest, XmlReader_MoveToEndElement)
{
    TEST_TRACE("");
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(false, reader.MoveToEndElement());
        reader.Read();
        EXPECT_EQ(true, reader.MoveToEndElement());
        EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
        EXPECT_EQ("root", reader.GetElementName());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        reader.Read();
        EXPECT_EQ(false, reader.MoveToEndElement());
        reader.Read();
        EXPECT_EQ(true, reader.MoveToEndElement());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read(); // root
        reader.Read(); // element1
        reader.Read(); // element2
        reader.Read(); // TEXT1
        reader.Read(); // sub-element
        reader.Read(); // TEXT2
        EXPECT_EQ(false, reader.MoveToEndElement());
        reader.Read(); // /sub-element
        EXPECT_EQ(false, reader.MoveToEndElement());
        reader.Read(); // TEXT3
        EXPECT_EQ(false, reader.MoveToEndElement());
    }
}

TEST_F(MisoTest, XmlReader_MoveToEndOfParentElement)
{
    TEST_TRACE("");
    {
        miso::XmlReader reader("test.xml");
        EXPECT_EQ(false, reader.MoveToEndOfParentElement());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        EXPECT_EQ(false, reader.MoveToEndOfParentElement());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        reader.Read(); // element1
        EXPECT_EQ(true, reader.MoveToEndOfParentElement());
        EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
        EXPECT_EQ("root", reader.GetElementName());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        reader.Read();
        reader.Read(); // element2
        EXPECT_EQ(true, reader.MoveToEndOfParentElement());
        EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
        EXPECT_EQ("root", reader.GetElementName());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        reader.Read();
        reader.Read();
        reader.Read(); // TEXT1
        EXPECT_EQ(true, reader.MoveToEndOfParentElement());
        EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
        EXPECT_EQ("element", reader.GetElementName());
    }
    {
        miso::XmlReader reader("test.xml");
        reader.Read();
        reader.Read();
        reader.Read();
        reader.Read();
        reader.Read();
        reader.Read();
        reader.Read(); // /sub-element
        EXPECT_EQ(true, reader.MoveToEndOfParentElement());
        EXPECT_EQ(miso::XmlNodeType::EndElement, reader.GetNodeType());
        EXPECT_EQ("element", reader.GetElementName());
    }
}

TEST_F(MisoTest, XmlReader_ErrorAttributeDupulicated)
{
    TEST_TRACE("");
    miso::XmlReader reader("test_error_attribute_duplicated.xml");
    EXPECT_EQ(true, reader.CanRead());

    EXPECT_EQ(false, reader.Read());
    EXPECT_EQ(true, reader.HasError());
    auto errors = reader.GetErrors();
    EXPECT_EQ(1, errors.size());
    EXPECT_EQ("[ERROR] Attribute id redefined", errors[0]);
}

TEST_F(MisoTest, XmlReader_ErrorFileNotFound)
{
    TEST_TRACE("");
    miso::XmlReader reader("test_error_file_not_found.xml");
    EXPECT_EQ(false, reader.CanRead());
    EXPECT_EQ(false, reader.Read());
    EXPECT_EQ("", reader.GetElementName());
    EXPECT_EQ("", reader.GetContentText());
    EXPECT_EQ("", reader.GetAttributeValueString("id"));
    EXPECT_EQ(0, reader.GetAllAttributes().size());
    EXPECT_EQ(miso::XmlNodeType::None, reader.GetNodeType());
    EXPECT_EQ(true, reader.HasError());
}

TEST_F(MisoTest, Scalar)
{
    TEST_TRACE("");
    // From NumberFormatterTest.cpp of the Poco
    EXPECT_EQ(0.0, miso::Scalar("0px").GetValue());
    EXPECT_EQ(1.0, miso::Scalar("1px").GetValue());
    EXPECT_EQ(-1.0, miso::Scalar("-1px").GetValue());
    EXPECT_EQ(1.0, miso::Scalar("+1px").GetValue());
    EXPECT_EQ(123.0, miso::Scalar("123px").GetValue());
    EXPECT_EQ(-123.0, miso::Scalar("-123px").GetValue());
    EXPECT_EQ(1, miso::Scalar("001px").GetValue());
    EXPECT_EQ(-1, miso::Scalar("-001px").GetValue());
    EXPECT_EQ(1.23, miso::Scalar("1.23px").GetValue());
    EXPECT_EQ(-1.23, miso::Scalar("-1.23px").GetValue());
    EXPECT_EQ(12.345, miso::Scalar("12.345px").GetValue());
    EXPECT_EQ(-12.345, miso::Scalar("-12.345px").GetValue());
    EXPECT_EQ(0.0, miso::Scalar("-0.0px").GetValue());
    EXPECT_EQ(0.1, miso::Scalar("0.1px").GetValue());
    EXPECT_EQ(-0.1, miso::Scalar("-0.1px").GetValue());
    EXPECT_EQ(0.1, miso::Scalar("0.100px").GetValue());
    EXPECT_EQ(1.0, miso::Scalar("1.px").GetValue());
    EXPECT_EQ(0.1, miso::Scalar(".1px").GetValue());
    EXPECT_EQ(-1.0, miso::Scalar("-1.px").GetValue());
    EXPECT_EQ(-0.1, miso::Scalar("-.1px").GetValue());

    EXPECT_TRUE(miso::Scalar("1px ").IsValid());
    EXPECT_TRUE(miso::Scalar("1px+").IsValid());
    EXPECT_TRUE(miso::Scalar("1px)").IsValid());

    EXPECT_FALSE(miso::Scalar("abc1px").IsValid());
    EXPECT_FALSE(miso::Scalar("+++1px").IsValid());
    EXPECT_FALSE(miso::Scalar("--1px").IsValid());
    EXPECT_FALSE(miso::Scalar("  1px  ").IsValid());

    EXPECT_FALSE(miso::Scalar(nullptr).IsValid());
    EXPECT_FALSE(miso::Scalar("").IsValid());
    EXPECT_FALSE(miso::Scalar("px").IsValid());
    EXPECT_FALSE(miso::Scalar("-px").IsValid());
    EXPECT_FALSE(miso::Scalar("+px").IsValid());
    EXPECT_FALSE(miso::Scalar("apx").IsValid());
    EXPECT_FALSE(miso::Scalar("0..1px").IsValid());
    //EXPECT_FALSE(miso::Scalar("1+px").IsValid());
    //EXPECT_FALSE(miso::Scalar("1px++").IsValid());
    EXPECT_FALSE(miso::Scalar("1pxx").IsValid());

    EXPECT_EQ(miso::ScalarUnit::Pixel, miso::Scalar("1px").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::ScaledPixel, miso::Scalar("1sp").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Parcent, miso::Scalar("1%").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Vw, miso::Scalar("1vw").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Vh, miso::Scalar("1vh").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Vmin, miso::Scalar("1vmin").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Vmax, miso::Scalar("1vmax").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Second, miso::Scalar("1s").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Millisecond, miso::Scalar("1ms").GetUnit());
    EXPECT_EQ(miso::ScalarUnit::Unitless, miso::Scalar("1").GetUnit());

    EXPECT_EQ("", miso::Scalar("").ToString());
    EXPECT_EQ("1px", miso::Scalar("1px").ToString());
    EXPECT_EQ("0.100px", miso::Scalar("0.1px").ToString());
    EXPECT_EQ("1%", miso::Scalar("1%").ToString());
    EXPECT_EQ("1ms", miso::Scalar("1ms").ToString());
    EXPECT_EQ("1", miso::Scalar("1").ToString());

    EXPECT_FALSE(miso::Scalar("0").IsFloat());
    EXPECT_FALSE(miso::Scalar("1").IsFloat());
    EXPECT_FALSE(miso::Scalar("-1").IsFloat());
    EXPECT_TRUE(miso::Scalar("0.0").IsFloat());
    EXPECT_TRUE(miso::Scalar("1.0").IsFloat());
    EXPECT_TRUE(miso::Scalar("-1.0").IsFloat());
    EXPECT_TRUE(miso::Scalar("1.").IsFloat());
    EXPECT_TRUE(miso::Scalar(".1").IsFloat());
}

TEST_F(MisoTest, Scalar_Convert)
{
    TEST_TRACE("");
    auto n = miso::Scalar("10px");
    EXPECT_EQ(10, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10sp");
    EXPECT_EQ(20, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10vw");
    EXPECT_EQ(64.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10vh");
    EXPECT_EQ(48.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10vmax");
    EXPECT_EQ(64.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10vmin");
    EXPECT_EQ(48.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10%");
    EXPECT_EQ(10.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(0.1, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10s");
    EXPECT_EQ(-1.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(10000.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10ms");
    EXPECT_EQ(-1.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(-1.0, n.ToRatio(-1.0));
    EXPECT_EQ(10.0, n.ToMilliseconds(-1.0));
    n = miso::Scalar("10");
    EXPECT_EQ(1000.0, n.ToLength(640, 480, 2.0f, 100.0, -1.0));
    EXPECT_EQ(10.0, n.ToRatio(-1.0));
    EXPECT_EQ(-1.0, n.ToMilliseconds(-1.0));
}

TEST_F(MisoTest, Numeric)
{
    TEST_TRACE("");
    auto numerics = miso::Numeric::FromString("  0 1 \t \t 2 rgb ( 12,34 , 56 )10px 20% ");
    EXPECT_EQ(6, numerics.size());
    EXPECT_EQ(0, numerics[0].GetScalar().GetValue());
    EXPECT_EQ(1, numerics[1].GetScalar().GetValue());
    EXPECT_EQ(2, numerics[2].GetScalar().GetValue());
    EXPECT_TRUE(numerics[3].GetColor().IsValid());
    EXPECT_EQ(10, numerics[4].GetScalar().GetValue());
    EXPECT_EQ(20, numerics[5].GetScalar().GetValue());
}

TEST_F(MisoTest, Color)
{
    TEST_TRACE("");
    miso::Color h3("#123");
    EXPECT_TRUE(h3.IsValid());
    EXPECT_EQ(0x33 / 255.0f, h3.GetRgba().B);
    EXPECT_EQ(1.0f, h3.GetRgba().A);
    EXPECT_EQ("#112233ff", h3.ToString("hex8"));

    miso::Color h4("#abcd");
    EXPECT_TRUE(h4.IsValid());
    EXPECT_EQ(0xCC / 255.0f, h4.GetRgba().B);
    EXPECT_EQ(0xDD / 255.0f, h4.GetRgba().A);
    EXPECT_EQ("#aabbccdd", h4.ToString("hex8"));

    miso::Color h6("#123456");
    EXPECT_TRUE(h6.IsValid());
    EXPECT_EQ(0x56 / 255.0f, h6.GetRgba().B);
    EXPECT_EQ(1.0f, h6.GetRgba().A);
    EXPECT_EQ("#123456ff", h6.ToString("hex8"));

    miso::Color h8("#AABBCCDD");
    EXPECT_TRUE(h8.IsValid());
    EXPECT_EQ(0xCC / 255.0f, h8.GetRgba().B);
    EXPECT_EQ(0xDD / 255.0f, h8.GetRgba().A);
    EXPECT_EQ("#aabbccdd", h8.ToString("hex8"));

    {
        miso::Color a(miso::Rgba(0.25f, 0.75f, 0.75f, 0.5f));
        miso::Color b(miso::Hsla(0.5f, 0.5f, 0.5f, 0.5f));
        miso::Color c(miso::Hsva(0.5f, 2 / 3.0f, 0.75f, 0.5f));
        EXPECT_EQ("#40bfbf80", a.ToString("hex8"));
        EXPECT_EQ("#40bfbf80", b.ToString("hex8"));
        EXPECT_EQ("#40bfbf80", c.ToString("hex8"));
    }
    {
        miso::Color rgb("rgb(12,34,56)");
        EXPECT_TRUE(rgb.IsValid());
        EXPECT_EQ("#0c2238ff", rgb.ToString("hex8"));
        miso::Color rgba("rgba(12,34,56,78)");
        EXPECT_TRUE(rgba.IsValid());
        EXPECT_EQ("#0c22384e", rgba.ToString("hex8"));
        miso::Color hsl("hsl(12,34,56)");
        EXPECT_TRUE(hsl.IsValid());
        EXPECT_EQ("#b57869ff", hsl.ToString("hex8"));
        miso::Color hsla("hsla(12,34,56,78)");
        EXPECT_TRUE(hsla.IsValid());
        EXPECT_EQ("#b57869c7", hsla.ToString("hex8"));
        miso::Color hsv("hsv(12,34,56)");
        EXPECT_TRUE(hsv.IsValid());
        EXPECT_EQ("#8f685eff", hsv.ToString("hex8"));
        miso::Color hsva("hsva(12,34,56,78)");
        EXPECT_TRUE(hsva.IsValid());
        EXPECT_EQ("#8f685ec7", hsva.ToString("hex8"));

        miso::Color rgbp("rgb(12%,34%,56%)");
        EXPECT_TRUE(rgbp.IsValid());
        EXPECT_EQ("#1f578fff", rgbp.ToString("hex8"));
        miso::Color rgbr("rgb(0.12,0.34,0.56)");
        EXPECT_TRUE(rgbr.IsValid());
        EXPECT_EQ("#1f578fff", rgbr.ToString("hex8"));
    }
    {
        miso::Color a;
        size_t count;
        EXPECT_TRUE(miso::Color::TryParse("rgb(12%,34%,56%)", a, &count));
        EXPECT_EQ(16, count);
        EXPECT_TRUE(miso::Color::TryParse("rgb ( 12% , 34% , 56%  ) ", a, &count));
        EXPECT_EQ(24, count);
        EXPECT_TRUE(miso::Color::TryParse("rgb 12% , 34% , 56%   ", a, &count));
        EXPECT_EQ(19, count);
        EXPECT_TRUE(miso::Color::TryParse("rgb 12% 34% 56%   ", a, &count));
        EXPECT_EQ(15, count);
        EXPECT_FALSE(miso::Color::TryParse("rgb ( 12% , 34% ) 56% ) ", a));
        EXPECT_FALSE(miso::Color::TryParse("rgb ) 12% 34% 56% ) ", a));
        EXPECT_FALSE(miso::Color::TryParse("rgb ( 12% 34% 56% ( ", a));
        EXPECT_TRUE(miso::Color::TryParse("rgb 12% 34% 56% ( ", a, &count));
        EXPECT_EQ(15, count);
        EXPECT_FALSE(miso::Color::TryParse("rgb ( 12% 34% 56%   ", a));
    }
}

TEST_F(MisoTest, Numeric_ParsePerformance)
{
    for (int i = 0; i < 1000; ++i) {
        volatile auto n = miso::Numeric::FromString(
            "123.456px 123.456sp 123.456vw 123.456vh 123.456vmax 123.456vmin "
            "123.456% 123.456s 123.456ms 123.456 #012 #3456 #789abc #def0123456 "
            "rgb 12 34 56 rgba(12,34,56,78) "
            "hsl 12 34 56 hsla(12,34,56,78) "
            "hsv 12 34 56 hsva(12,34,56,78) "
            "0"
        );
    }
}

// BinaryReader Performance
#if 0
TEST_F(Performace, OneRead1MCrt)
{
    size_t size = 0;
    std::vector<char> v;
    {
        miso::BinaryReader reader("1m.bin");
        size = reader.GetSize();
    }
    v.resize(size);
    for (int n = 0; n < 100; ++n) {
        FILE* fp = fopen("1m.bin", "rb");
        fread(v.data(), 1, size, fp);
        fclose(fp);
    }
}

TEST_F(Performace, OneRead1M)
{
    size_t size = 0;
    std::vector<char> v;
    {
        miso::BinaryReader reader("1m.bin");
        size = reader.GetSize();
    }
    v.resize(size);
    for (int n = 0; n < 100; ++n) {
        miso::BinaryReader reader("1m.bin");
        reader.ReadBlock(v.data(), size);
    }
}

TEST_F(Performace, ReadAll1M)
{
    for (int n = 0; n < 100; ++n) {
        volatile auto buffer = miso::FileStream::ReadAll("1m.bin");
    }
}

TEST_F(Performace, SequencialRead1M8BCrt)
{
    size_t size = 0;
    std::vector<char> v;
    {
        miso::BinaryReader reader("1m.bin");
        size = reader.GetSize();
    }
    volatile char buffer[8];
    for (int n = 0; n < 100; ++n) {
        FILE* fp = fopen("1m.bin", "rb");
        for (size_t i = 0; i < size; i += sizeof(buffer)) {
            fread((void*)buffer, 1, sizeof(buffer), fp);
        }
        fclose(fp);
    }
}

TEST_F(Performace, SequencialRead1M8B)
{
    volatile char buffer[8];
    for (int n = 0; n < 100; ++n) {
        miso::BinaryReader reader("1m.bin");
        auto size = reader.GetSize();
        for (size_t i = 0; i < size; i += sizeof(buffer)) {
            volatile auto x = reader.Read<int64_t>();
        }
    }
}

TEST_F(Performace, RandomRead1M8B)
{
    for (int n = 0; n < 100; ++n) {
        srand(0);
        miso::BinaryReader reader("1m.bin");
        volatile char buffer[8];
        auto size = reader.GetSize();
        for (size_t i = 0; i < 100; ++i) {
            auto position = (size_t)((size - 1) * (float)rand() / RAND_MAX);
            reader.SetPosition(position);
            reader.ReadBlock((void*)buffer, sizeof(buffer));
        }
    }
}
#endif

// XmlReader Output
#if 0
TEST_F(MisoTest, XmlReader_OutputXml)
{
    TEST_TRACE("");
    miso::XmlReader reader("test.xml");
    while (reader.Read()) {
        printf(miso::StringUtils::Repeat("    ", reader.GetNestingLevel()).c_str());
        auto type = reader.GetNodeType();
        if (type == miso::XmlNodeType::StartElement || type == miso::XmlNodeType::EmptyElement) {
            printf("<%s", reader.GetElementName().c_str());
            for (auto a : reader.GetAllAttributes()) {
                printf(" %s='%s'", a.GetName().c_str(), a.GetScalar().GetValue().c_str());
            }
            if (type == miso::XmlNodeType::EmptyElement) printf("/");
            printf(">");
        } else if (type == miso::XmlNodeType::EndElement) {
            printf("</%s>", reader.GetElementName().c_str());
        } else if (type == miso::XmlNodeType::Text) {
            printf("%s", miso::StringUtils::Trim(reader.GetContentText()).c_str());
        } else {
        }
        printf("\n");
    }
    if (reader.HasError()) {
        printf("Errors:\n");
        for (auto& error : reader.GetErrors()) {
            printf("%s\n", error.c_str());
        }
    }
}
#endif

// XmlReader Performance
#if 0
TEST_F(MisoTest, libxml2_Performance)
{
    TEST_TRACE("");
    auto xml = miso::StringUtils::ReadFile("Azuki.xml");
    for (int i = 0; i < 100; i++) {
        volatile auto doc = miso::libxml::xmlReadMemory(xml.data(), (int)xml.length(), 0, 0, 0);
        miso::libxml::xmlFreeDoc(doc);
    }
}

TEST_F(MisoTest, XmlReader_Performance)
{
    TEST_TRACE("");
    auto xml = miso::StringUtils::ReadFile("Azuki.xml");
    for (int i = 0; i < 100; i++) {
        miso::XmlReader reader(xml.data(), xml.length());
        while (reader.Read()) {
            auto type = reader.GetNodeType();
            if (type == miso::XmlNodeType::StartElement || type == miso::XmlNodeType::EmptyElement) {
                auto& name = reader.GetElementName();
                if (name != "doc" &&
                    name != "members" &&
                    (name != "member" ||
                        reader.GetAttributeValueString("name") != "P:Sgry.Azuki.IUserInterface.ConvertsTabToSpaces")) {
                    reader.MoveToEndElement();
                    continue;
                }
                for (auto a : reader.GetAllAttributes()) {
                    volatile auto a_name = a.GetName();
                    volatile auto a_value = a.GetScalar().GetValue();
                }
            } else if (type == miso::XmlNodeType::EndElement) {
                volatile auto name = reader.GetElementName();
            } else if (type == miso::XmlNodeType::Text) {
                volatile auto text = reader.GetContentText();
            } else {
                ;
            }
        }
    }
}
#endif
