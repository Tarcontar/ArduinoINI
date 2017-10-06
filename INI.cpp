#include "INI.hpp"
#include <string.h>

INI::INI(bool caseSensitive) : m_caseSensitive(caseSensitive)
{
	
}

INI::~INI()
{
	close();
}

bool INI::open(String filename)
{
	m_pos = 0;
	m_file = SD.open(filename, FILE_READ);
}

bool INI::getValue(String section, String key, String &val)
{
	if (findSection(section))
	{
		if (findKey(key, val))
		{
			return true;
		}
	}
	return false;
}

bool INI::getValue(String section, String key, int &val)
{
	String v;
	if (getValue(section, key, v))
	{
		val = atoi(v.c_str()); 
		return true;
	}
	return false;
}

bool INI::getValue(String section, String key, unsigned long &val)
{
	String v;
	if (getValue(section, key, v))
	{
		char *endptr;
		val = strtoul(v.c_str(), &endptr, 10); 
		if (*endptr == v[0]) //no conversion
			return false;
		if (*endptr == '\0')
			return true;
	}
	return false;
}

bool INI::getValue(String section, String key, float &val)
{
	String v;
	if (getValue(section, key, v))
	{
		char *endptr;
		val = strtod(v.c_str(), &endptr); 
		if (*endptr == v[0]) //no conversion
			return false;
		if (*endptr == '\0')
			return true;
	}
	return false;
}

bool INI::getValue(String section, String key, bool &val)
{
	String v;
	if (getValue(section, key, v))
	{
		if (match("T") || match("t") || match("Y") || match("y") || match("1"))
		{
			val = true;
			return true;
		}
		if (match("F") || match("f") || match("N") || match("n") || match("0"))
		{
			val = false;
			return true;
		}
	}
	return false;
}

bool INI::getValues(String section, String **data, int *count)
{
	*count = 0;
	if (findSection(section))
	{
		int i = 0;
		int pos = m_pos; //save position of found section
		while(readLine())
		{
			char c = m_buffer[m_buffPos];
			if (isSectionStart(c))
			{
				break;
			}
			//make sure we add no empty lines !!!
			if (m_buffer[m_buffPos] != '\0')
				(*count)++;
		}
		*data = new String[*count];
		m_pos = pos;
		while(readLine())
		{
			char c = m_buffer[m_buffPos];
			if (c != '\0')
			{
				if (isSectionStart(c))
				{
					break;
				}
				while(m_buffer[m_buffPos] != '='  && m_buffPos < BUFFER_SIZE) { m_buffPos++; }
				if (m_buffPos < BUFFER_SIZE - 1)
					m_buffPos++;
				skipWhiteSpaces();
				(*data)[i++] = String(&m_buffer[m_buffPos]);
			}
		}
		return true;
	}
	return false;
}

void INI::close()
{
	m_file.close();
}

bool INI::findSection(String section)
{
	m_pos = 0;
	while(readLine())
	{
		char c = m_buffer[m_buffPos];
		if (isSectionStart(c))
		{
			m_buffPos++; // skip the [
			if (match(section))
				return true;
		}
	}
	return false;
}

bool INI::findKey(String key, String &val)
{
	while(readLine())
	{
		char c = m_buffer[m_buffPos];
		if (isSectionStart(c))
		{
			return false;
		}
		if (match(key))
		{
			m_buffPos += key.length();
			skipWhiteSpaces();
			m_buffPos++; //skip '='
			skipWhiteSpaces();
			val = String(&m_buffer[m_buffPos]);
			return true;
		}
	}
	return false;
}

bool INI::readLine()
{
	if (!m_file.seek(m_pos))
		return false;
	size_t bytesRead = m_file.read(m_buffer, BUFFER_SIZE);
	m_buffPos = 0;
	
	if (!bytesRead) 
	{
		m_buffer[0] = '\0';
		return false;
	}
	skipWhiteSpaces();
	removeTrailingWhiteSpaces(bytesRead);
	
	for (size_t i = m_buffPos; i < bytesRead; i++)
	{
		char c = m_buffer[i];
		if (isCommentChar(c))
		{
			m_buffer[i] = '\0';
		}
		if (isNewLine(c)) 
		{
			m_buffer[i] = '\0';
			m_pos += (i + 1); // skip past newline
			return true;
		}
	}
	if (!m_file.available()) 
	{
		// end of file without a newline
		m_buffer[bytesRead] = '\0';
	}
	return true;
}

bool INI::match(String str)
{
	skipWhiteSpaces();
	for (int i = 0; i < str.length(); i++)
	{
		if (m_caseSensitive)
		{
			if (m_buffer[m_buffPos + i] != str[i])
				return false;
		}
		else
		{
			if (toLower(m_buffer[m_buffPos + i]) != toLower(str[i]))
				return false;
		}
	}
	return true;
}

void INI::skipWhiteSpaces()
{
	while (isSpace(m_buffer[m_buffPos])) {m_buffPos++;}
}

void INI::removeTrailingWhiteSpaces(size_t bytesRead)
{
	int i = bytesRead - 2;
	while (isSpace(m_buffer[i]))
		m_buffer[i--] = '\n';
} 

bool INI::isNewLine(char c)
{
	return c == '\n';
}

bool INI::isSpace(char c)
{
	return c == ' ';
}

bool INI::isCommentChar(char c)
{
	return (c == ';' || c == '#');
}

bool INI::isSectionStart(char c)
{
	return c == '[';
}

char INI::toLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}