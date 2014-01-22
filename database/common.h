#ifndef __DATABASE_COMMON_H__
#define __DATABASE_COMMON_H__

typedef struct Head Head;
struct Head {
	int RecordLength;
	int RecordCount;
	int HeadNumber;
	int TailNumber;
};
extern int headSize;

bool const DatabaseInit(char const* const filename, int const recordSize, int const recordCount/*, int const flagOffset*/);
void ReadHead(FILE* const database, Head const* const head);
void WriteHead(FILE* const database, Head const* const head);
void ReadData(FILE* const database, void const* const data, int const size);
void WriteDate(FILE* const database, void const* const data, int const size);
void ReadPositionData(FILE* const database, Head const* const head, int const position, void const* const data);
void WritePositionData(FILE* const database, Head const* const head, int const position, void const* const data);

typedef bool const (*IsDeletePredication)(void const* const data);
bool const InsertData(char const* const filename, void const* const data, IsDeletePredication isDelete);
bool const RingInsertData(char const* const filename, void const* const data);

typedef void (*MarkDelete)(void const* const data);
void DeleteData(char const* const filename, int const position, MarkDelete const markDelete);
void UpdateData(char const* const filename, int const position, void const* const data);

typedef bool const (*AcceptPredication)(Contact const* const data, Contact const* const example);
typedef struct FindInfo FindInfo;
struct FindInfo {
	int index;
	AcceptPredication accept;
	void* example;
	IsDeletePredication isDelete;
	FILE* database;
	Head head;
};

FindInfo* FindOpen(AcceptPredication const accept, Contact const* const example, IsDeletePredication const isDelete, char const* const dataFilename);
void const* const FindNext(FindInfo* findInfo);
void FindClose(FindInfo* findInfo);

void const* const BinarySearch(FindInfo const* const findInfo);

#endif //__DATABASE_COMMON_H__
