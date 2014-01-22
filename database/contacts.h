#ifndef __DATABASE_CONTACTS_H__
#define __DATABASE_CONTACTS_H__

#include "common.h"

typedef char UserName[8];
typedef char CompanyName[8];
typedef char PhoneNumber[24];
typedef char Timestamp[16];
typedef char FileName[64];
typedef char RegionPrefix[7];
typedef char Location[10];

enum ContactType {
	tNormal,
	tVIP,
	tBlocked,
	tLocal,
	tOther
};

enum StateFlag {
	fDelete,
	fOther
};

typedef struct Contact Contact;
struct Contact {
	short id;
	UserName name;
	PhoneNumber phone;
	CompanyName company;
	short ringId;
	ContactType type;
	StateFlag deleteMark;
	char reserved[2];
};

bool const ContactInit(int const recordCount);
bool const InsertContact(Contact const* const contact);
void DeleteContact(int const position);
void UpdateContact(int const position, Contact const* const newContact);

#endif //__DATABASE_CONTACTS_H__
