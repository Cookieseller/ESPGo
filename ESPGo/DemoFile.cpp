#include "stdafx.h"
#include "DemoFile.h"

/**
* Other than the name indicates, open actually opens the file, checks if it's valid
* and than reads everything into a buffer before closing the file. Every subsequent action will then be done on the cached filecontent.
*/
bool CDemoFile::Open(const char *fileName)
{
	FILE *filePointer = NULL;

	// Try to open the given file
	fopen_s(&filePointer, fileName, "rb");
	if (filePointer == NULL)
	{
		fprintf(stderr, "Unable to open the given File");
		return false;
	}

	// Check that the file size is at least the size that the demo header should be
	demoheader demoHeader;
	size_t fileSize = getFileSize(filePointer);
	if (fileSize < sizeof(demoHeader))
	{
		fprintf(stderr, "File is smaller than the demo header should be.");
		fclose(filePointer);
		return false;
	}

	// Read the first chunk of the file, which should be the demo header
	fread(&demoHeader, 1, sizeof(demoHeader), filePointer);
	fileSize -= sizeof(demoHeader);

	if (!isValidDemoFile(demoHeader))
	{
		fprintf(stderr, "The demo header does not contain the expected ID.");
		fclose(filePointer);
		return false;
	}

	if (!isValidProtocolVersion(demoHeader))
	{
		fprintf(stderr, "The demo version is not correct");
		fclose(filePointer);
		return false;
	}

	// This is a little nasty, as we should only open the file and not read and close it. Refactor this later
	m_fileBufferPos = 0;
	m_fileBuffer.clear();
	m_fileBuffer.resize(fileSize);
	fread(&m_fileBuffer[0], 1, fileSize, filePointer);

	fclose(filePointer);
	filePointer = NULL;

	return true;
}

/**
* Writes the current command into the cmd parameter.
*/
void CDemoFile::GetCommandAtPosition(unsigned char &cmd, int32 &tick)
{
	if (!m_fileBuffer.size())
		return;

	cmd = *(unsigned char *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(unsigned char);

	// No end tag found but no more commands exist, exit
	if (cmd <= 0)
	{
		fprintf(stderr, "Missing end tag in demo file.\n");
		cmd = dem_stop;
		return;
	}

	// Timestamp, not to sure how that is relevant as of now
	tick = *(__int32 *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(__int32);

	// Playerslot, not to sure how that is relevant as of now
	unsigned char playerSlot = *(unsigned char *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(unsigned char);
}

/**
* Retains further details about the command, see the democmdinfo struct
*/
void CDemoFile::GetCommandInfo(democmdinfo &info)
{
	if (!m_fileBuffer.size())
		return;

	memcpy(&info, &m_fileBuffer[m_fileBufferPos], sizeof(democmdinfo));
	m_fileBufferPos += sizeof(democmdinfo);
}

/**
* Just some stuff we do totally not care about but have to deal with, as our file position would be off otherwise
*/
void CDemoFile::GetSequenceInfo(int32 &nSeqNrIn, int32 &nSeqNrOut)
{
	if (!m_fileBuffer.size())
		return;

	nSeqNrIn = *(int32 *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(int32);

	nSeqNrOut = *(int32 *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(int32);
}

/**
*/
int32 CDemoFile::GetUserCmd(char *buffer, int32 &size)
{
	if (!m_fileBuffer.size())
		return 0;

	int32 sequence = *(int32 *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(int32);

	size = GetRawData(buffer, size);

	return sequence;
}

/**
* Read n bytes of raw data from the current filepos into the given buffer.
* This might fail if the given buffer is too small, but does not fail if no buffer is given at all.
* In the latter case the method just skips the reading part and returns the potential size, 
* moving the file position by the size of the file block.
*/
int32 CDemoFile::GetRawData(char *buffer, int32 length)
{
	if (!m_fileBuffer.size())
		return 0;

	// read length of data block
	int32 size = *(int32 *)(&m_fileBuffer[m_fileBufferPos]);
	m_fileBufferPos += sizeof(int32);

	// Check if the Datablock is larger than the buffer we got
	if (buffer && (length < size))
	{
		fprintf(stderr, "GetRawData: buffer overflow (%i).\n", size);
		return -1;
	}

	if (buffer)
		memcpy(buffer, &m_fileBuffer[m_fileBufferPos], size);

	m_fileBufferPos += size;

	return size;
}

/**
* Returns the size of the given file
*/
size_t CDemoFile::getFileSize(FILE *filePointer)
{
	fseek(filePointer, 0, SEEK_END);
	size_t fileSize = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);

	return fileSize;
}

/**
* Checks wether the demoHeader contains the correct filestamp, indicating a supported demo file
*/
bool CDemoFile::isValidDemoFile(demoheader demoHeader)
{
	if (strcmp(demoHeader.demofilestamp, DEMO_HEADER_ID))
		return false;
	return true;
}

/**
* Checks wether the demoHeader contains the correct protocol version
*/
bool CDemoFile::isValidProtocolVersion(demoheader demoHeader)
{
	if (demoHeader.demoprotocol != DEMO_PROTOCOL)
		return false;
	return true;
}
