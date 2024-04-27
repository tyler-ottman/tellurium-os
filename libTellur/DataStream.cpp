#include "DataStream.hpp"

DataStream::DataStream(const uint8_t *data, size_t streamLen)
    : data(data), index(0), streamLen(streamLen) {}

DataStream::DataStream(const uint8_t *data)
    : data(data), index(0), streamLen(-1) {}

DataStream::~DataStream() {}

uint8_t DataStream::decodeUInt8() {
    uint8_t res = data[index];
    index++;
    return res;
}

uint16_t DataStream::decodeUInt16() {
    uint16_t res = data[index] | data[index + 1] << 8;
    index += 2;
    return res;
}

uint32_t DataStream::decodeUInt32() {
    uint32_t res = data[index] | data[index + 1] << 8 | data[index + 2] << 16 |
                   data[index + 3] << 24;
    index += 4;
    return res;
}

int64_t DataStream::decodeLeb128() {
    size_t result = 0;
    size_t shift = 0;
    uint8_t byte;
    
    for (;;) {
        byte = data[index++];
        result |= (0x7f & byte) << shift;
        shift += 7;
        if ((0x80 & byte) == 0) { break; }
    }
    
    if (0x40 & byte) {
        result |= -(1 << shift);
    }

    return result;
}

size_t DataStream::decodeULeb128() {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & data[index]) << shift;
        if (!(0x80 & data[index++])) { break; }
        shift += 7;
    }

    return result;
}

bool DataStream::isStreamable() { return data && (index < streamLen); }

const uint8_t *DataStream::getData() { return data; }

const uint8_t *DataStream::getData(size_t idx) {
    return (idx >= 0 && idx < streamLen) ? data + idx : nullptr;
}

size_t DataStream::getIndex() { return index; }

void DataStream::setData(const uint8_t *data) { this->data = data; }

void DataStream::setOffset(size_t offset) { index = offset; }

void DataStream::setLen(size_t len) { streamLen = len; }