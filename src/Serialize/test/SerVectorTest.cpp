#include "SerializeConfig.h"
#include "gtest/gtest.h"
#include "JsonThor.h"
#include "BsonThor.h"
#include "SerUtil.h"
#include <algorithm>

using namespace ThorsAnvil::Serialize;

TEST(SerVectorTest, JsonArrayEmpty)
{
    std::stringstream   stream("[]");
    std::vector<int>    data;

    stream >> jsonImporter(data, false);

    EXPECT_EQ(data.empty(), true);
}

TEST(SerVectorTest, JsonVectorOfIntSerialize)
{
    std::vector<int>    data{1,2,3,4,5,6,7,8,101,102,9,10};

    std::stringstream   stream;
    stream << jsonExporter(data, false);

    std::string result  = stream.str();
    result.erase(std::remove_if(std::begin(result), std::end(result), [](char x){return ::isspace(x);}), std::end(result));

    EXPECT_EQ(result, R"([1,2,3,4,5,6,7,8,101,102,9,10])");
}
TEST(SerVectorTest, JsonVectorOfIntDeSerialize)
{
    std::vector<int>    data;
    std::stringstream   stream("[1,2,3,4,5,6,7,8,101,102,9,10]");

    stream >> jsonImporter(data, false);
    auto loop = data.begin();

    EXPECT_EQ(*loop++, 1);
    EXPECT_EQ(*loop++, 2);
    EXPECT_EQ(*loop++, 3);
    EXPECT_EQ(*loop++, 4);
    EXPECT_EQ(*loop++, 5);
    EXPECT_EQ(*loop++, 6);
    EXPECT_EQ(*loop++, 7);
    EXPECT_EQ(*loop++, 8);
    EXPECT_EQ(*loop++, 101);
    EXPECT_EQ(*loop++, 102);
    EXPECT_EQ(*loop++, 9);
    EXPECT_EQ(*loop++, 10);
}

TEST(SerVectorTest, BsonArrayEmpty)
{
    std::stringstream   stream("[]");
    std::vector<int>    data;

    stream >> bsonImporter(data, false);

    EXPECT_EQ(data.empty(), true);
}

TEST(SerVectorTest, BsonVectorOfIntSerialize)
{
    std::vector<int>    data{1,2,3,4,5,6,7,8,101,102,9,10};

    std::stringstream   stream;
    stream << bsonExporter(data, false);

    std::string result  = stream.str();
    result.erase(std::remove_if(std::begin(result), std::end(result), [](char x){return ::isspace(x);}), std::end(result));

    EXPECT_EQ(result, R"([1,2,3,4,5,6,7,8,101,102,9,10])");
}
TEST(SerVectorTest, BsonVectorOfIntDeSerialize)
{
    std::vector<int>    data;
    std::stringstream   stream("[1,2,3,4,5,6,7,8,101,102,9,10]");

    stream >> bsonImporter(data, false);
    auto loop = data.begin();

    EXPECT_EQ(*loop++, 1);
    EXPECT_EQ(*loop++, 2);
    EXPECT_EQ(*loop++, 3);
    EXPECT_EQ(*loop++, 4);
    EXPECT_EQ(*loop++, 5);
    EXPECT_EQ(*loop++, 6);
    EXPECT_EQ(*loop++, 7);
    EXPECT_EQ(*loop++, 8);
    EXPECT_EQ(*loop++, 101);
    EXPECT_EQ(*loop++, 102);
    EXPECT_EQ(*loop++, 9);
    EXPECT_EQ(*loop++, 10);
}

