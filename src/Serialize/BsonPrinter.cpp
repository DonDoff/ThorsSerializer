#include "SerializeConfig.h"
#include "BsonPrinter.h"
#include "ThorsIOUtil/Utility.h"
#include "GitUtility/ieee754_types.h"
#include <iomanip>
#include <algorithm>
#include <boost/endian/conversion.hpp>

using namespace ThorsAnvil::Serialize;

using IntTypes = std::tuple<std::int32_t, std::int64_t>;

char  intKey[]      = {'\x10', '\x12'};
char  floatKey[]    = {'\x01', '\x13'};

HEADER_ONLY_INCLUDE
BsonPrinter::BsonPrinter(std::ostream& output, PrinterConfig config)
    : PrinterInterface(output, config)
{}

// MAP
HEADER_ONLY_INCLUDE
std::size_t BsonPrinter::getSizeMap(std::size_t count)
{
    /*
     * A map is a document:
     *      <size 4bytes> <Element-List> <Terminator 1byte>
     * Each element int the <Element-List> consists of:
     *      <type 1byte> <e-name (size accounted for) + '\0' 1byte> <object (size accounted for)>
     */
    return 4 + (count * (1 + 1)) + 1;
}

// ARRAY
HEADER_ONLY_INCLUDE
std::size_t BsonPrinter::getSizeArray(std::size_t count)
{
    /*
     * A map is a document:
     *      <size 4bytes> <Element-List> <Terminator 1byte>
     * Each element int the <Element-List> consists of:
     *      <type 1byte> <e-name (which is a string of the index number) + '\0' 1byte> <object (size accounted for)>
     */
     /*
      * This is the same as Map (see above)
      * The difference we have not accounted for the string that define the index
      * So we must calculate the string length need to store all the index strings.
      */
    std::size_t indexTotalStringLen     = 0;
    std::size_t sizeLeft                = count;
    std::size_t nextLevel               = 10;
    std::size_t numberOfDigitsThisLevel = 1;
    std::size_t accountedFor            = 0;

    while (sizeLeft > 0)
    {
        if (count <= nextLevel)
        {
            indexTotalStringLen += (count - accountedFor) * numberOfDigitsThisLevel;
            break;
        }
        indexTotalStringLen += (nextLevel - accountedFor) * numberOfDigitsThisLevel;
        accountedFor            += nextLevel;
        numberOfDigitsThisLevel += 1;
        nextLevel               *= 10;
    }

    return getSizeMap(count) + indexTotalStringLen;
}

// Add a new Key
HEADER_ONLY_INCLUDE
void BsonPrinter::addKey(std::string const& key)
{
    if (currentContainer.back() != BsonContainer::Map)
    {
        throw std::runtime_error(
                        ThorsAnvil::Utility::buildErrorMessage("ThorsAnvil::Serialize::BsonPrinter", "addKey",
                                                                "Adding a Key to a non map object")
                                                              );
    }
    currentKey = key;
}

HEADER_ONLY_INCLUDE
void BsonPrinter::writeKey(char value, std::size_t size)
{
    if (!currentContainer.empty())
    {
        output.write(&value, 1);
        if (currentContainer.back() == BsonContainer::Array)
        {
            output << arrayIndex.back();
            output.write("", 1);
            ++arrayIndex.back();
        }
        else
        {
            output.write(currentKey.c_str(), currentKey.size() + 1);
        }
    }
    else if (size != static_cast<std::size_t>(-1))
    {
        // This happens if you try and write a basic type directly to the stream.
        // BSON only supports Map/Array as the top level object.
        // So when we write a single value we wrap it just like an array.
        //
        // <4 byte Doc Size> <1 byte Type info> <2 byte Index "0"> <value> <1 byte doc term>
        std::int32_t totalSize = 4 + 1 + 2 + size + 1;
        writeSize(totalSize);
        output.write(&value, 1);
        output.write("0", 2);
        // The value will now write itself.
        // then the docClose() will at the document terminator.
    }
}

template<typename Int>
void BsonPrinter::writeSize(Int size)
{
    Int docSize = boost::endian::native_to_little(size);
    output.write(reinterpret_cast<char*>(&docSize), sizeof(docSize));
}

HEADER_ONLY_INCLUDE void BsonPrinter::openDoc()
{}

HEADER_ONLY_INCLUDE void BsonPrinter::closeDoc()
{
    if (config.parserInfo == static_cast<long>(BsonContainer::Value))
    {
        // The Map and Array close themselves.
        // But values need to be closed here.
        // See:  writeKey() for details.
        output.write("", 1);
    }
}
HEADER_ONLY_INCLUDE
void BsonPrinter::openMap(std::size_t size)
{
    writeKey('\x03', -1);
    writeSize<std::int32_t>(size);
    currentContainer.emplace_back(BsonContainer::Map);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::closeMap()
{
    if (currentContainer.back() != BsonContainer::Map)
    {
        throw std::runtime_error(
                        ThorsAnvil::Utility::buildErrorMessage("ThorsAnvil::Serialize::BsonPrinter", "closeMap",
                                                             "Closing an unopened Map")
                                                              );
    }
    output.write("",1);
    currentContainer.pop_back();
}

HEADER_ONLY_INCLUDE
void BsonPrinter::openArray(std::size_t size)
{
    writeKey('\x04', -1);
    writeSize<std::int32_t>(size);
    currentContainer.emplace_back(BsonContainer::Array);
    arrayIndex.emplace_back(0);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::closeArray()
{
    if (currentContainer.back() != BsonContainer::Array)
    {
        throw std::runtime_error(
                        ThorsAnvil::Utility::buildErrorMessage("ThorsAnvil::Serialize::BsonPrinter", "closeArray",
                                                               "Closing an unopened Array")
                                                              );
    }
    output.write("",1);
    currentContainer.pop_back();
    arrayIndex.pop_back();
}

HEADER_ONLY_INCLUDE
template<std::size_t Size, typename Int>
void BsonPrinter::writeInt(Int value)
{
    using IntType = typename std::tuple_element<Size/4 - 1, IntTypes>::type;

    IntType             output = value;
    writeKey(intKey[Size/4 - 1], Size);
    writeSize<IntType>(output);
}

HEADER_ONLY_INCLUDE
template<std::size_t Size, typename Float>
void BsonPrinter::writeFloat(Float value)
{
    IEEE_754::_2008::Binary<Size * 8>   outputValue = value;
    writeKey(floatKey[Size/8 - 1], Size);
    output.write(reinterpret_cast<char*>(&outputValue), Size);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::writeBool(bool value)
{
    writeKey('\x08', 1);
    char outVal = (value ? '\x01' : '\x00');
    output.write(&outVal, 1);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::writeString(std::string const& value)
{
    writeKey('\x02', 4 + value.size() + 1);
    writeSize<std::int32_t>(value.size() + 1);
    output << EscapeString(value);
    output.write("", 1);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::writeNull()
{
    writeKey('\x0A', 0);
}

HEADER_ONLY_INCLUDE
void BsonPrinter::writeBinary(std::string const& value)
{
    writeKey('\x05', 4 + 1 + value.size());    // binary
    writeSize<std::int32_t>(value.size());
    output.write("\x80", 1);
    output.write(value.c_str(), value.size());
}
