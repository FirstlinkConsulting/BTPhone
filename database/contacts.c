#include "contacts.h"

static char const* const dataFilename = "contacts.bin";

bool const ContactInit(int const recordCount) {
	//#define offsetof(s, m)   (size_t)&(((s *)0)->m)
	return DatabaseInit(dataFilename, sizeof(Contact), recordCount);
}

static bool const isContactDelete(Contact const* const data) {
	bool result = false;

	if (data->deleteMark == fDelete) {
		result = true;
	}

	return result;
}

bool const InsertContact(Contact const* const contact) {
	return InsertData(dataFilename, contact, isContactDelete);
}

static void markDelete(Contact const* const data) {
	data->deleteMark = fDelete;
}

void DeleteContact(int const position) {
	DeleteData(dataFilename, position, markDelete);
}

void UpdateContact(int const position, Contact const* const newContact) {
	UpdateData(dataFilename, position, newContact);
}

static bool const contactNameFilter(Contact const* const data, Contact const* const example) {
	bool result = false;

	if (memcmp(data->name, example->name) == 0) { //you can do anything compare
		result = true;
	}

	return result;
}
