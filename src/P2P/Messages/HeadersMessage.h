#pragma once

#include "Message.h"

#include <Core/Models/BlockHeader.h>

class HeadersMessage : public IMessage
{
public:
	//
	// Constructors
	//
	HeadersMessage(std::vector<BlockHeaderPtr>&& headers)
		: m_headers(std::move(headers))
	{

	}
	HeadersMessage(const HeadersMessage& other) = default;
	HeadersMessage(HeadersMessage&& other) noexcept = default;

	//
	// Destructor
	//
	virtual ~HeadersMessage() = default;

	//
	// Operators
	//
	HeadersMessage& operator=(const HeadersMessage& other) = default;
	HeadersMessage& operator=(HeadersMessage&& other) noexcept = default;

	//
	// Clone
	//
	virtual IMessagePtr Clone() const override final { return IMessagePtr(new HeadersMessage(*this)); }

	//
	// Getters
	//
	virtual MessageTypes::EMessageType GetMessageType() const override final { return MessageTypes::Headers; }
	const std::vector<BlockHeaderPtr>& GetHeaders() const { return m_headers; }

	//
	// Deserialization
	//
	static HeadersMessage Deserialize(ByteBuffer& byteBuffer)
	{
		std::vector<BlockHeaderPtr> headers;

		const uint16_t numHeaders = byteBuffer.ReadU16();
		for (uint16_t i = 0; i < numHeaders; i++)
		{
			headers.emplace_back(std::make_shared<const BlockHeader>(BlockHeader::Deserialize(byteBuffer)));
		}

		return HeadersMessage(std::move(headers));
	}

protected:
	virtual void SerializeBody(Serializer& serializer) const override final
	{
		serializer.Append<uint16_t>((uint16_t)m_headers.size());
		for (auto iter = m_headers.cbegin(); iter != m_headers.cend(); iter++)
		{
			(*iter)->Serialize(serializer);
		}
	}

private:
	std::vector<BlockHeaderPtr> m_headers;
};