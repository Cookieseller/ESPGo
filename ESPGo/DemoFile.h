#pragma once
class CDemoFile
{
public:
	bool  Open(const char *fileName);
	void  GetCommandAtPosition(unsigned char &cmd, int32 &tick);
	void  GetCommandInfo(democmdinfo &info);
	void  GetSequenceInfo(int32 &nSeqNrIn, int32 &nSeqNrOut);
	int32 GetUserCmd(char *buffer, int32 &size);
	int32 GetRawData(char *buffer, int32 length);

private:
	size_t getFileSize(FILE *filePointer);
	bool isValidDemoFile(demoheader demoHeader);
	bool isValidProtocolVersion(demoheader demoHeader);

	std::string m_fileBuffer;
	size_t		m_fileBufferPos;
};

