#pragma once

#include <cstdio>
#include <cstdint>
#include <string>

struct File {
	virtual void read(void *out, size_t length) = 0;
	virtual void write(const void *out, size_t length) = 0;
	virtual void seek(size_t pos, int mode) = 0;
	virtual size_t tell() = 0;
	virtual ~File() {}

	inline uint8_t readUint8() { uint8_t res; read(&res, 1); return res; }
	inline uint16_t readUint16() { uint16_t res; read(&res, 2); return res; }
	inline uint32_t readUint32() { uint32_t res; read(&res, 4); return res; }
	inline float readFloat() { float res; read(&res, 4); return res; }
	inline std::string readString(int numChars) {
		std::string s;
		s.reserve(numChars);
		for (int i = 0; i < numChars; i++)
			s.push_back(readUint8());
		return s;
	}
	inline std::string readStringZ() {
		std::string s;
		while (uint8_t ch = readUint8())
			s.push_back(ch);
		return s;
	}

	inline void writeUint8(uint8_t val) { write(&val, 1); }
	inline void writeUint16(uint16_t val) { write(&val, 2); }
	inline void writeUint32(uint32_t val) { write(&val, 4); }
	inline void writeFloat(float val) { write(&val, 4); }
};

struct IOFile : File {
	FILE *file = nullptr;
	void read(void *out, size_t length) override;
	void write(const void *out, size_t length) override;
	void seek(size_t pos, int mode) override;
	size_t tell() override;
	void close();
	void open(const char *name, const char *mode);
	IOFile() : file(nullptr) {}
	IOFile(const char *name, const char *mode) { open(name, mode); }
	IOFile(IOFile &&af) { file = af.file; af.file = nullptr; }
	~IOFile() { close(); }
};

struct MemFile : File {
	uint8_t *_startptr = nullptr;
	uint8_t *_curptr = nullptr;
	void read(void *out, size_t length) override;
	void write(const void *out, size_t length) override;
	void seek(size_t pos, int mode) override;
	size_t tell() override;
	MemFile(void *ptr) : _startptr((uint8_t*)ptr), _curptr((uint8_t*)ptr) {};
};

File * GetResourceFile(const char *resName);