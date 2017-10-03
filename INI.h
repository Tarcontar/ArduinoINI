#pragma once

#include "Arduino.h"
#include "SD.h"

#define BUFFER_SIZE 64

class INI
{
public:
	INI(bool caseSensitive = false);
	~INI();
	
	bool open(String filename);
	bool getValue(String section, String key, String &val);
	bool getValue(String section, String key, int &val);
	bool getValue(String section, String key, unsigned long &val);
	bool getValue(String section, String key, float &val);
	bool getValue(String section, String key, bool &val);
	
	bool getValues(String section, String data[]);
private:
	void close();
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
	char m_buffer[BUFFER_SIZE];
	bool m_caseSensitive;
};
