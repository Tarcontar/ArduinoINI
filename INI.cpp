#include "INI.hpp"
#include <string.h>

INI::INI(bool caseSensitive) : m_caseSensitive(caseSensitive)
{
	
}

INI::~INI()
{
	close();
}

bool INI::open(String filename, uint8_t mode)
{
	m_pos = 0;
	m_file = SD.open(filename, mode);
}

void INI::writeSection(String section)
{
	m_file.print("[");
	m_file.print(section);
	m_file.print("]");
	m_file.println();
}

void INI::write(String key, String val)
{
	m_file.print(key);
	m_file.print(" = ");
	m_file.print(val);
	m_file.println();
}

void INI::write(String key, int val)
{
	m_file.print(key);
	m_file.print(" = ");
	m_file.print(val);
	m_file.println();
}

void INI::write(String key, bool val)
{
	m_file.print(key);
	m_file.print(" = ");
	if (val)
		m_file.print("true");
	else
		m_file.print("false");
	m_file.println();
}

void INI::write(String key, const char *val)
{
	m_file.print(key);
	m_file.print(" = ");
	int i = 0;
	char c;
	do
	{
		c = val[i++];
		m_file.print(c);
	}
	while(c != '\0');
	m_file.println();
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

bool INI::getValues(String section, String **keys, String **values, int *count)
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
			if (c != '\0')
				(*count)++;
		}
		*keys = new String[*count];
		*values = new String[*count];
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
				int j = 0;
				pos = m_buffPos;
				while(m_buffer[m_buffPos] != '=' && m_buffPos < (INI_BUFFER_SIZE - 1)) 
				{ 
					m_buffPos++;
				}
				m_buffer[m_buffPos - 1] = '\0';
				(*keys)[i] = String(&m_buffer[pos]);
				if (m_buffPos < (INI_BUFFER_SIZE - 1))
					m_buffPos++;
				skipWhiteSpaces();
				(*values)[i++] = String(&m_buffer[m_buffPos]);
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
			{
				m_buffPos++; //skip the ]
				return true;
			}
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
	size_t bytesRead = m_file.read(m_buffer, INI_BUFFER_SIZE);
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