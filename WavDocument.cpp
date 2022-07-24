#include "WavDocument.h"
#include <cassert>
#include "File.h"

void WavDocument::read(File * file)
{
	uint32_t riffTag = file->readUint32();
	assert(riffTag == 'FFIR');
	uint32_t riffSize = file->readUint32();
	uint32_t riffFormat = file->readUint32();
	assert(riffFormat == 'EVAW');
	uint32_t fmtTag = file->readUint32();
	assert(fmtTag == ' tmf');
	uint32_t fmtSize = file->readUint32();
	assert(fmtSize >= 0x10);
	formatTag = file->readUint16();
	//assert(formatTag == 1); // WAVE_FORMAT_PCM
	numChannels = file->readUint16();
	samplesPerSec = file->readUint32();
	avgBytesPerSec = file->readUint32();
	blockAlign = file->readUint16();
	pcmBitsPerSample = file->readUint16();
	file->seek(fmtSize - 0x10, SEEK_CUR);
	uint32_t dataTag = file->readUint32();
	assert(dataTag == 'atad');
	uint32_t dataSize = file->readUint32();
	data.resize(dataSize);
	file->read(data.data(), dataSize);
}

void WavDocument::write(File * file)
{
	file->writeUint32('FFIR');
	file->writeUint32(12 + 16 + 8 + data.size());
	file->writeUint32('EVAW');
	file->writeUint32(' tmf');
	file->writeUint32(0x10);
	file->writeUint16(formatTag);
	file->writeUint16(numChannels);
	file->writeUint32(samplesPerSec);
	file->writeUint32(avgBytesPerSec);
	file->writeUint16(blockAlign);
	file->writeUint16(pcmBitsPerSample);
	file->writeUint32('atad');
	file->writeUint32(data.size());
	file->write(data.data(), data.size());
}

float WavSampleReader::nextSample()
{
	float fs = 0.0f;
	switch (_wav->formatTag) {
	case 1:
		if (_wav->pcmBitsPerSample == 8)
			fs = *(uint8_t*)_pnt / 128.0f - 1.0f;
		else if (_wav->pcmBitsPerSample == 16)
			fs = *(int16_t*)_pnt / 32768.0f;
		break;
	case 3:
		if (_wav->pcmBitsPerSample == 32)
			fs = *(float*)_pnt;
	}
	_pnt += _wav->blockAlign;
	return fs;
}

bool WavSampleReader::available()
{
	return (size_t)(_pnt - _wav->data.data()) < _wav->data.size();
}
