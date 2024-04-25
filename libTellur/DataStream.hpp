#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <stddef.h>
#include <stdint.h>

class DataStream {
public:
    DataStream(const uint8_t *data, size_t streamLen);
    DataStream(const uint8_t *data);
    ~DataStream();

    uint8_t decodeUInt8(void);
    uint16_t decodeUInt16(void);
    uint32_t decodeUInt32(void);
    int64_t decodeLeb128(void);
    size_t decodeULeb128(void);

    bool isStreamable(void);
    const uint8_t *getData(void);
    const uint8_t *getData(size_t idx);
    size_t getIndex(void);

    void setData(const uint8_t *data);
    void setOffset(size_t offset);
    void setLen(size_t len);

private:
    const uint8_t *data;
    size_t index;
    size_t streamLen;
};

#endif // DATASTREAM_H