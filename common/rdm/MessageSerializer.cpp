/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * MessageSerializer.cpp
 * Prints the test representation of a Message.
 * Copyright (C) 2011 Simon Newton
 */

#include <ola/messaging/Message.h>
#include <ola/network/NetworkUtils.h>
#include <ola/rdm/MessageSerializer.h>
#include <string.h>
#include <algorithm>

namespace ola {
namespace rdm {

MessageSerializer::MessageSerializer(unsigned int initial_size)
    : m_data(NULL),
      m_offset(0),
      m_buffer_size(0),
      m_initial_buffer_size(initial_size) {
}

MessageSerializer::~MessageSerializer() {
  if (m_data)
    delete[] m_data;
}


/**
 * Serialize a message and return a pointer to the message in memory
 */
const uint8_t *MessageSerializer::SerializeMessage(
    const ola::messaging::Message *message,
    unsigned int *length) {

  if (!m_data) {
    m_buffer_size = m_initial_buffer_size;
    m_data = new uint8_t[m_buffer_size];
  }
  m_offset = 0;

  message->Accept(*this);

  *length = m_offset;
  return m_data;
}


void MessageSerializer::Visit(
    const ola::messaging::BoolMessageField *message) {
  CheckForFreeSpace(1);
  m_data[m_offset++] = message->Value();
}


void MessageSerializer::Visit(
    const ola::messaging::StringMessageField *message) {
  unsigned int size = std::min(
      static_cast<unsigned int>(message->Value().size()),
      message->GetDescriptor()->MaxSize());
  unsigned int used_size = std::max(
      size,
      message->GetDescriptor()->MinSize());
  CheckForFreeSpace(size);
  memcpy(m_data + m_offset, message->Value().c_str(), size);
  memset(m_data + m_offset + size, 0, used_size - size);
  m_offset += used_size;
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<uint8_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<uint16_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<uint32_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<int8_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<int16_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::BasicMessageField<int32_t> *message) {
  IntVisit(message);
}


void MessageSerializer::Visit(
    const ola::messaging::GroupMessageField *message) {
  (void) message;
}


void MessageSerializer::PostVisit(
    const ola::messaging::GroupMessageField *message) {
  (void) message;
}



/**
 * Check that there is at least required_size bytes of space left, if not
 * expand the memory so the new data can fit.
 */
void MessageSerializer::CheckForFreeSpace(unsigned int required_size) {
  if (m_buffer_size - m_offset > required_size)
    return;

  uint8_t *old_buffer = m_data;
  m_data = new uint8_t[2 * m_buffer_size];
  memcpy(m_data, old_buffer, m_offset);
  delete[] old_buffer;
}


/**
 * Serialize an integer value, converting to little endian if needed
 */
template <typename int_type>
void MessageSerializer::IntVisit(
    const ola::messaging::BasicMessageField<int_type> *message) {
  CheckForFreeSpace(sizeof(int_type));
  int_type value;
  if (message->GetDescriptor()->IsLittleEndian())
    value = ola::network::HostToLittleEndian(message->Value());
  else
    value = ola::network::HostToNetwork(message->Value());


  uint8_t *ptr = reinterpret_cast<uint8_t*>(&value);
  memcpy(m_data + m_offset, ptr, sizeof(int_type));
  m_offset += sizeof(int_type);
}
}  // rdm
}  // ola
