#pragma once

#define BASEBUFFERSIZE 100

typedef unsigned char DATASIZE_VECTOR;

struct Header
{
	Header() {}
	Header(const size_t& size, const PacketType& type)
		: m_size(static_cast<unsigned short>(size + sizeof(Header))), m_type(type) {}
	unsigned short	m_size = 0;
	PacketType		m_type = PacketType::TEST;
};


class Serializer
{
public:
	Serializer()
	{
		char* m_buffer = (char*)malloc(sizeof(char) * BASEBUFFERSIZE);
		m_head = &m_buffer[0];
		m_inputOffset	= m_head + sizeof(Header);
		m_outputOffset	= m_head + sizeof(Header);

		//memcpy()
	}
	Serializer(char* buffer)
	{
		m_head = buffer;
		m_inputOffset	= buffer + sizeof(Header);
		m_outputOffset	= buffer + sizeof(Header);
	}
	Serializer(Serializer& se)
	{
		//m_offset = se.;
	}

	~Serializer()
	{
		std::cout << "~Serializer" << std::endl;
		free(m_head);
	}

	void SetHeader(const PacketType& type)
	{

	}

	size_t GetSize() const
	{
		return m_currentSize;
	}

	char* GetBuffer() const
	{
		return m_head;
	}

	void CheckOverflow(const size_t& dataSize)
	{
		if (m_currentSize + dataSize >= m_maxSize)
		{
			Realloc(dataSize);
		}
	}

	void Realloc(const size_t& dataSize)
	{
		size_t newMaxSize;

		if (dataSize >= m_maxSize / 2)
		{
			newMaxSize = static_cast<size_t>((m_currentSize + dataSize) * 1.5);
		}
		else
		{
			newMaxSize = static_cast<size_t>(m_maxSize * 1.5);
		}
		m_maxSize = newMaxSize;

		std::cout << "Realloc" << std::endl;
		std::cout << "New Buffer Size : "<< newMaxSize << std::endl;

		char* m_tempBuffer = (char*)malloc(newMaxSize);

		if (m_tempBuffer != NULL)
		{
			memcpy(m_tempBuffer, m_head, m_currentSize);

			free(m_head);
		
			m_head = m_tempBuffer;
		}
		else
		{
			//무한루프 걸릴라나?
			Realloc(dataSize);
		}
	}

	//Primitive, Enum only
	/*template<class T>
	void WriteData(T& data)
	{
		static_assert(
			std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Only support primitive or enum data types"
			);

		memcpy(m_inputOffset, &data, sizeof(data));
		
		m_inputOffset += sizeof(data);
		m_currentSize += sizeof(data);
	}*/


	/////// WRITE OPERATOR ///////

	template<class T>
	void operator<<(const T& data)
	{
		CheckOverflow(sizeof(T));

		memcpy(m_inputOffset, &data, sizeof(T));

		m_inputOffset += sizeof(T);
		m_currentSize += sizeof(T);
	}

	template<>
	void operator<<(const Header& header)
	{
		CheckOverflow(sizeof(Header));

		memcpy(m_head, &header, sizeof(Header));

		//m_inputOffset += sizeof(Header);
		m_currentSize += sizeof(Header);
	}

	template<>
	void operator<<(const std::string& data)
	{
		unsigned short length = static_cast<unsigned short>(data.length());

		CheckOverflow(sizeof(length) + data.size());

		memcpy(m_inputOffset, &length, sizeof(length));

		m_inputOffset += sizeof(length);
		m_currentSize += sizeof(length);

		strncpy(m_inputOffset, data.c_str(), data.size());

		m_inputOffset += length;
		m_currentSize += length;
	}

	template<class T>
	void operator<<(const std::vector<T>& data)
	{
		static_assert(
			std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Only support primitive or enum data types"
			);

		unsigned char elementCount = static_cast<unsigned char>(data.size());

		CheckOverflow(sizeof(elementCount) + (elementCount * sizeof(T)));

		memcpy(m_inputOffset, &elementCount, sizeof(elementCount));

		m_inputOffset += sizeof(elementCount);
		m_currentSize += sizeof(elementCount);

		for (const auto& element : data)
		{
			memcpy(m_inputOffset, &element, sizeof(T));
			m_inputOffset += sizeof(T);
		}
		m_currentSize += elementCount * sizeof(T);
	}

	template<class T1, class T2>
	void operator<<(const std::unordered_map<T1, T2> data)
	{
		static_assert(
			std::is_arithmetic<T1>::value ||
			std::is_enum<T1>::value ||
			std::is_arithmetic<T2>::value ||
			std::is_enum<T2>::value,
			"Only support primitive or enum data types"
			);

		unsigned short elementCount = static_cast<unsigned short>(data.size());

		CheckOverflow(sizeof(elementCount) + (elementCount * (sizeof(T1) + sizeof(T2))));

		memcpy(m_inputOffset, &elementCount, sizeof(elementCount));
		m_inputOffset += sizeof(elementCount);
		m_currentSize += sizeof(elementCount);

		for (const std::pair<T1, T2>& element : data)
		{
			memcpy(m_inputOffset, &element.first, sizeof(T1));
			m_inputOffset += sizeof(T1);
			memcpy(m_inputOffset, &element.second, sizeof(T2));
			m_inputOffset += sizeof(T2);
		}

		m_currentSize += (sizeof(T1) + sizeof(T2)) * elementCount;
	}




	/////// READ OPERATOR ///////

	template<class T>
	void operator>>(T& data)
	{
		static_assert(
			std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Only support primitive or enum data types"
			);

		memcpy(&data, m_outputOffset, sizeof(T));

		m_outputOffset += sizeof(T);
	}

	template<>
	void operator>>(Header& header)
	{
		memcpy(&header, m_head, sizeof(Header));

		/*memcpy(&header.m_size, m_head, sizeof(Header::m_size));
		memcpy(&header.m_type, m_head + sizeof(Header::m_size), sizeof(Header::m_type));*/
	}

	template<>
	void operator>>(std::string& data)
	{
		unsigned short length;

		memcpy(&length, m_outputOffset, sizeof(length));

		m_outputOffset += sizeof(length);

		data.resize(length);

		strncpy(&data[0], m_outputOffset, length);

		m_outputOffset += length;
	}

	template<class T>
	void operator>>(std::vector<T>& data)
	{
		unsigned char elementCount;

		memcpy(&elementCount, m_outputOffset, sizeof(elementCount));

		m_outputOffset += sizeof(elementCount);
		
		data.resize(elementCount);

		for (int i = 0; i < elementCount; i++)
		{
			memcpy(&data[i], m_outputOffset, sizeof(T));
			m_outputOffset += sizeof(T);
		}
	}

	template<class T1, class T2>
	void operator>>(std::unordered_map<T1, T2>& data)
	{
		unsigned short elementCount;

		memcpy(&elementCount, m_outputOffset, sizeof(elementCount));
		m_outputOffset += sizeof(elementCount);

		for (int i = 0; i < elementCount; i++)
		{
			T1 key;
			T2 value;

			memcpy(&key, m_outputOffset, sizeof(T1));
			m_outputOffset += sizeof(T1);
			memcpy(&value, m_outputOffset, sizeof(T2));
			m_outputOffset += sizeof(T2);

			data.insert(std::make_pair(key, value));
		}
	}



	

private:
	char* m_inputOffset;
	char* m_outputOffset;
	char* m_head;

	size_t m_currentSize = 0;
	size_t m_maxSize = BASEBUFFERSIZE;
};

