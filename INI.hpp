#pragma once

#include "Arduino.h"
#include "SD.h"

#define INI_BUFFER_SIZE 64

class INI
{
public:
	INI(bool caseSensitive = false);
	~INI();
	
	bool open(String filename, uint8_t mode);
	
	void writeSection(String section);
	void write(String key, String val);
	void write(String key, int val);
	void write(String key, bool val);
	void write(String key, const char *val);
	
	bool getValue(String section, String key, String &val);
	bool getValue(String section, String key, int &val);
	bool getValue(String section, String key, unsigned long &val);
	bool getValue(String section, String key, float &val);
	bool getValue(String section, String key, bool &val);
	
	bool getValues(String section, String **keys, String **values, int *count);
	
	void close();
private:
	bool findSection(String section);
	bool findKey(String key, String &val);
	bool readLine();
	bool match(String section);
	
	void skipWhiteSpaces();
	void removeTrailingWhiteSpaces(size_t bytesRead);
	bool isNewLine(char c);
	bool isSpace(char c);
	bool isCommentChar(char c);
	bool isSectionStart(char c);
	
	char toLower(char c);
	
private:
	mutable File m_file;
	size_t m_pos;
	int m_buffPos;
	char m_buffer[INI_BUFFER_SIZE];
	bool m_caseSensitive;
};
