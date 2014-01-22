#include "common.h"

int headSize = sizeof(Head);

bool const DatabaseInit(char const* const filename, int const recordSize, int const recordCount) {
	bool result = false;
	FILE* database = fopen(filename, "w+");

	if (database) {
		Head head;
		head.RecordLength = recordSize;
		head.RecordCount = recordCount;
		head.HeadNumber = head.TailNumber = 0;
		WriteHead(database, &head);
		fseek(database, sizeof(Head)+recordSize * recordCount, SEEK_SET);
		fclose(database);
		result = true;
	}

	return result;
}

void ReadHead(FILE* const database, Head const* const head) {
	fseek(datebase, 0, SEEK_SET);
	fread(&head, headSize, 1, database);
}

void WriteHead(FILE* const database, Head const* const head) {
	fseek(datebase, 0, SEEK_SET);
	fwrite(&head, headSize, 1, database);
}

void ReadData(FILE* const database, void const* const data, int const size) {
	fread(data, size, 1, database);
}

void WriteDate(FILE* const database, void const* const data, int const size) {
	fwrite(data, size, 1, database);
}

void ReadPositionData(FILE* const database, Head const* const head, int const position, void const* const data) {
	fseek(database, headSize + position * head->RecordLength, SEEK_SET);
	fread(data, head->RecordLength, 1, database);
}

void WritePositionData(FILE* const database, Head const* const head, int const position, void const* const data) {
	fseek(database, headSize + position * head->RecordLength, SEEK_SET);
	fwrite(data, head->RecordLength, 1, database);
}

bool const InsertData(char const* const filename, void const* const data, IsDeletePredication const isDelete) {
	bool result = false;
	FILE* database = fopen(filename, "w+");

	if (database) {
		Head head;
		ReadHead(database, &head);

		if ((head.TailNumber > head.HeadNumber) && (head.TailNumber < head.RecordCount)) { //tail not full
			WritePositionData(database, &head, head.TailNumber, data);
			result = true;
			++head.TailNumber;
			WriteHead(database, &head);
		}
		else {
			void* readedData = malloc(head.RecordLength);
			int pos = headSize;
			fseek(database, pos, SEEK_SET);

			for (int index = 0; index < head.RecordCount; ++index) {
				ReadData(database, readedData);

				if (isDelete(readedData)) {
					fseek(database, -head.RecordLength, SEEK_CUR);
					WriteData(database, data);
					result = true;
					break;
				}
			}
		}

		fclose(database);
	}

	return result;
}

bool const RingInsertData(char const* const filename, void const* const data) {
	bool result = false;
	FILE* database = fopen(filename, "w+");

	if (database) {
		Head head;
		ReadHead(database, &head);

		if ((head.TailNumber > head.HeadNumber) && (head.TailNumber < head.RecordCount)) { //tail not full
			WritePositionData(database, &head, head.TailNumber, data);
			result = true;
			++head.TailNumber;

			if (head.TailNumber == head.RecordCount) {
				head.TailNumber = 0;
				head.HeadNumber = head.TailNumber + 1;
			}

			WriteHead(database, &head);
		}
		else {
			WritePositionData(database, &head, head.TailNumber, data);
			result = true;
			++head.TailNumber;
			++head.HeadNumber;

			if (head.HeadNumber == head.RecordCount) {
				head.HeadNumber = 0;
			}

			WriteHead(database, &head);
		}

		fclose(database);
	}

	return result;
}

void DeleteData(char const* const filename, int const position, MarkDelete const markDelete) {
	FILE* database = fopen(filename, "w+");

	if (database) {
		Head head;
		ReadHead(database, &head);
		void* data = malloc(head.RecordLength);
		ReadPositionData(database, position, data);
		WritePositionData(database, position, markDelete(data));
		free(data);
		fclose(database);
	}
}

void UpdateData(char const* const filename, int const position, void const* const data) {
	FILE* database = fopen(filename, "w+");

	if (database) {
		WritePositionData(database, position, data);
		fclose(database);
	}
}

FindInfo* FindOpen(AcceptPredication const accept, Contact const* const example, IsDeletePredication const isDelete, char const* const dataFilename) {
	FindInfo* result = (FindInfo*)malloc(sizeof(FindInfo));
	result->index = -1;
	result->accept = accept;
	result->example = example;
	result->isDelete = isDelete;
	result->database = fopen(dataFilename, "r");

	if (result->database) {
		Head head;
		ReadHead(findInfo->database, &result->head);

		if (head.HeadNumber > head.TailNumber) {
			result->index = head.HeadNumber;
		}
	}
	else {
		free(result);
		result = 0;
	}

	return result;
}

void const* const FindNext(FindInfo* findInfo) {
	void* result = 0;
	fseek(findInfo->database, headSize + findInfo->index * head.RecordLength, SEEK_SET);
	void* readedData = malloc(head.RecordLength);

	if (findInfo->head.HeadNumber < findInfo->head.TailNumber) {
		++findInfo->index;
		for (; findInfo->index < head.TailNumber; ++findInfo->index) {
			ReadData(findInfo->database, readedData);

			if (!findInfo->isDelete(readedData) && findInfo->accept(readedData)) {
				result = malloc(head.RecordLength);
				memcpy(result, readedData, head.RecordLength);
				break;
			}
		}
		--findInfo->index;
	}
	else {
		bool finded = false;
		++findInfo->index;

		for (; findInfo->index < head.RecordCount; ++findInfo->index) {
			ReadData(findInfo->database, readedData);

			if (!findInfo->isDelete(readedData) && findInfo->accept(readedData)) {
				result = malloc(head.RecordLength);
				memcpy(result, readedData, head.RecordLength);
				finded = true;
				break;
			}
		}

		if (!finded) {
			findInfo->index = 0;

			for (; findInfo->index < head.RecordCount; ++findInfo->index) {
				ReadData(findInfo->database, readedData);

				if (!findInfo->isDelete(readedData) && findInfo->accept(readedData)) {
					result = malloc(head.RecordLength);
					memcpy(result, readedData, head.RecordLength);
					break;
				}
			}

			--findInfo->index;
		}
	}

	free(readedData);
	return result;
}

void FindClose(FindInfo* findInfo) {
	fclose(findInfo->database);
	free(findInfo);
}

void const* const BinarySearch(FindInfo const* const findInfo) {
	//
}
