#ifndef LISTENER_H
#define LISTENER_H

#include <Arduino.h>
#include <USB.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "CRC.h"
#include "event_center.h"

namespace simia
{
class Listener
{
  private:
    enum State
    {
        WaitHeader1,
        WaitHeader2,
        WaitLen,
        WaitData,
        WaitCRC,
        Received
    };

    State _state{State::WaitHeader1};
    const uint8_t Header[2]{0x59, 0x49};
    const uint8_t _data_size{3};
    std::vector<uint8_t> _data{Header[0], Header[1], 0x00};

    auto get_data() -> void;
    auto verify_crc() -> bool;

  public:
    Listener(int baudRate);
    auto listen() -> void;
    auto trigger() -> void;
    auto send(byte data[], uint8_t size) -> void;
    ~Listener() = default;
};

inline auto Listener::get_data() -> void
{
    _data.resize(_data_size + _data[2]);
    Serial.readBytes(_data.data() + _data_size, _data[2]);
}

inline auto Listener::verify_crc() -> bool
{
    byte crc[2]{};
    Serial.readBytes(crc, sizeof(crc));

    byte tmp_crc[2]{};
    auto tmp = calcCRC16(_data.data(), _data.size(), CRC16_XMODEM_POLYNOME, CRC16_XMODEM_INITIAL, CRC16_XMODEM_XOR_OUT,
                         CRC16_XMODEM_REV_IN, CRC16_XMODEM_REV_OUT);

    tmp_crc[0] = static_cast<byte>(tmp >> 8);
    tmp_crc[1] = static_cast<byte>(tmp & 0xFF);

    if (crc[0] == tmp_crc[0] && crc[1] == tmp_crc[1])
    {
        return true;
    }

    return false;
}

inline Listener::Listener(int baudRate)
{
    Serial.begin(baudRate);
}

inline auto Listener::listen() -> void
{
    switch (_state)
    {
    case State::WaitHeader1: {
        if (Serial.available())
        {
            auto incoming_byte = Serial.read();
            if (incoming_byte == Header[0])
            {
                _state = State::WaitHeader2;
            }
        }
        break;
    }

    case State::WaitHeader2: {
        if (Serial.available())
        {
            auto incoming_byte = Serial.read();
            if (incoming_byte == Header[1])
            {
                _state = State::WaitLen;
            }
            else
            {
                _state = State::WaitHeader1;
            }
        }

        break;
    }

    case State::WaitLen: {
        if (Serial.available())
        {
            auto incoming_byte = Serial.read();
            _data[2] = incoming_byte;
            _state = State::WaitData;
        }

        break;
    }

    case State::WaitData: {
        get_data();
        _state = State::WaitCRC;
        break;
    }

    case State::WaitCRC: {
        if (verify_crc())
        {
            _state = State::Received;
        }
        else
        {
            _state = State::WaitHeader1;
        }
        break;
    }

    case State::Received:
        this->trigger();
        _state = State::WaitHeader1;
        _data.erase(_data.begin() + _data_size, _data.end());
        break;

    default:
        break;
    }
}

inline auto Listener::trigger() -> void
{
    if (_data[3] == CMD::SetSpeed)
    {
        EventCenter::instance().trigger<uint8_t>(static_cast<CMD>(_data[3]), _data[4]);
    }
    else
    {
        EventCenter::instance().trigger(static_cast<CMD>(_data[3]));
    }
}

inline auto Listener::send(byte data[], uint8_t size) -> void
{
    std::vector<uint8_t> buf{Header[0], Header[1], size};
    buf.insert(buf.end(), data, data + size);

    byte tmp_crc[2]{};
    auto tmp = calcCRC16(buf.data(), buf.size(), CRC16_XMODEM_POLYNOME, CRC16_XMODEM_INITIAL, CRC16_XMODEM_XOR_OUT,
                         CRC16_XMODEM_REV_IN, CRC16_XMODEM_REV_OUT);
    tmp_crc[0] = static_cast<byte>(tmp >> 8);
    tmp_crc[1] = static_cast<byte>(tmp & 0xFF);

    buf.insert(buf.end(), tmp_crc, tmp_crc + 2);
    Serial.write(buf.data(), buf.size());
}

} // namespace simia

#endif