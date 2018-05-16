/**
* Name:    Francesco
* Surname: Longo
* ID:      223428
* Lab:     8
* Ex:      3
*
* Exercise 03 (versions A, B, and C)
* ----------------------------------
*
* A data base is given on a single *binary* file with
* *fixed-length* records.
* The format of the file is the one *generated in Exercise 2*.
*
* The program has to implement a user menu, where the user can give the
* following command:
* - R n: Where R is the character "R", and n is an interger value.
*        Read the data of a student number n (with ID = n).
* - W n: Where R is the character "R", and n is an interger value.
*        Write the data of a student number n (with ID = n).
* - E: End the program.
* The input file name is given on the command line.
*
* The following is an example of execution:
*
* user choice: R 3     // the user want to read info for student 3
* 3 200000 Verdi Giacomo 15
* user choice: R 1     // the user want to read info for student 1
* 1 100000 Romano Antonio 25
* user choice: W 1     // the user want to over-write info for student 1
* 1 100000 Romano Antonio 27
* user choice: W 5     // the user want to add data for student 5
* 5 157143 White House 30
* user choice: E       // the user want to end
* stop program
*
* Where noticed that
* 1) the input file is supposed to be the one specified in the
*    previous exercise, but in binary form
* 2) "//" specifies explanatory comments not program I/O lines
*    "R 3", "R 1", "W 1", etc. are the user inputs, all other characters
*    belong to the program output.
*
* Write three versions of the program:
* - Version A
*   read the file using file pointers
* - Version B
*   read the file using an overlapped data structure
* - Version C
*   [FOR FUTURE DEVELOPMENT, i.e., after file locking has been
*   introduced.]
*   lock each record before reading (or writing) it, and release the
*   same record as soon as the operation has been performed.
*   (Notice that locking for now is useless, but it will be useful with
*   multi-threaded applications).
*
**/

// remove security warnings
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// define unicode
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif // !UNICODE

// include
#include <Windows.h>
#include <tchar.h>

#define SECONDS 5

//----------------------------------------
// change this value for switch the read/write called
// can be 'A', 'B' or 'C'
#define VERSION 'C'
//----------------------------------------

// VERSION == 'A'
#if VERSION == 'A'
#define read readWithFilePointers
#define write writeWithFilePointers
#endif

// VERSION == 'B'
#if VERSION == 'B'
#define read readWithOverlappedDataStructures
#define write writeWithOverlappedDataStructures
#endif

// VERSION == 'C'
#if VERSION == 'C'
#define read readWithLocks
#define write writeWithLocks
#endif

// define
#define CMD_MAX_LEN 255
#define STR_MAX_L 30+1	// one extra tchar for the string terminator

// struct for read/write through DB
typedef struct _student {
	INT id;
	DWORD regNum;
	TCHAR surname[STR_MAX_L];
	TCHAR name[STR_MAX_L];
	INT mark;
} STUDENT;

/** PROTOTYPES **/
// A
VOID readWithFilePointers(INT id, HANDLE hIn);
VOID writeWithFilePointers(INT id, HANDLE hIn);

// B
VOID readWithOverlappedDataStructures(INT id, HANDLE hIn);
VOID writeWithOverlappedDataStructures(INT id, HANDLE hIn);

// C
VOID readWithLocks(INT id, HANDLE hIn);
VOID writeWithLocks(INT id, HANDLE hIn);

int Return(int value);
LPWSTR getErrorMessageAsString(DWORD errorCode);

// main
INT _tmain(INT argc, LPTSTR argv[]) {
	// variables
	TCHAR command[CMD_MAX_LEN];
	TCHAR op;
	INT id;
	HANDLE hIn;
	BOOL exit = FALSE;

	// check number of parameters
	if (argc != 2) {
		_ftprintf(stderr, _T("Usage: %s <input_file>\n"), argv[0]);
		return Return(1);
	}

	// open the binary file for reading/writing
	hIn = CreateFile(argv[1],
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// check the HANDLE value
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Cannot open output file %s. Error: \n%s\n"), argv[2], getErrorMessageAsString(GetLastError()));
		return Return(2);
	}

	// main loop of the program
	while (1) {
		// print instructions
		_tprintf(_T("Type a command : \"R id\" or \"W id\" or \"E\"\n> "));

		// read the all the string typed on command line
		_fgetts(command, CMD_MAX_LEN, stdin);


		if (_stscanf(command, _T("%c"), &op) != 1) {
			// read 1 char
			_tprintf(_T("Command contains some errors\n"));
		} else {
			// check command read
			switch (op) {

			case 'R':
				// read id
				if (_stscanf(command, _T("%*c %d"), &id) != 1) {
					_tprintf(_T("Command contains some errors\n"));
				} else {
					read(id, hIn);
				}
				break;

			case 'W':
				// write id
				if (_stscanf(command, _T("%*c %d"), &id) != 1) {
					_tprintf(_T("Command contains some errors\n"));
				} else {
					write(id, hIn);
				}
				break;

			case 'E':
				// prepare for exit the program
				exit = TRUE;
				break;

			default:
				// read and unknown operation
				_ftprintf(stderr, _T("Unknown operation %c\n"), op);
				break;
			}
		}

		// if exit was set, exit from while
		if (exit) {
			break;
		}
	}

	// close and return
	CloseHandle(hIn);
	return Return(0);
}

// A
// this function performs the random access by setting the file pointer
VOID readWithFilePointers(INT id, HANDLE hIn) {
	STUDENT s;
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER position;
	DWORD nRead;

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	position.QuadPart = n * sizeof(s);

	// set file pointer from FILE_BEGIN
	SetFilePointerEx(hIn, position, NULL, FILE_BEGIN);

	// and read
	if (ReadFile(hIn, &s, sizeof(s), &nRead, NULL) && nRead == sizeof(s)) {
		// read was successful
		_tprintf(_T("id: %d\treg_number: %ld\tsurname: %10s\tname: %10s\tmark: %d\n"), s.id, s.regNum, s.surname, s.name, s.mark);
	} else {
		// some errors in the read (out of bounds?)
		_ftprintf(stderr, _T("Error in read\n"));
	}
}

// this function performs the random access by setting the file pointer
VOID writeWithFilePointers(INT id, HANDLE hIn) {
	STUDENT s;
	TCHAR command[CMD_MAX_LEN];
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER position;
	DWORD nWritten;

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	position.QuadPart = n * sizeof(s);

	SetFilePointerEx(hIn, position, NULL, FILE_BEGIN);
	_tprintf(_T("You required to write student with id = %d.\nInsert the following data: \"registration_number surname name mark\"\n> "), id);
	
	// the id was already entered by the user, no need to ask it again
	s.id = id;
	
	// read a line
	while (_fgetts(command, CMD_MAX_LEN, stdin)) {
		// parse the parameters
		if (_stscanf(command, _T("%ld %s %s %d"), &s.regNum, s.surname, s.name, &s.mark) != 4) {
			// if parse fails, ask again to input the data
			_tprintf(_T("Error in the string. The format is the following \"registration_number surname name mark\"\n> "));
		} else {
			// if parse is correct, can go on
			break;
		}
	}

	// write the record
	if (WriteFile(hIn, &s, sizeof(s), &nWritten, NULL) && nWritten == sizeof(s)) {
		// write was successful
		_tprintf(_T("Record with id = %d stored\n"), s.id);
	} else {
		// some errors in the write
		_ftprintf(stderr, _T("Error in write\n"));
	}
}

// B
// this function performs the random acces by using an OVERLAPPED structure
VOID readWithOverlappedDataStructures(INT id, HANDLE hIn) {
	STUDENT s;
	DWORD nRead;
	// create a "clean" OVERLAPPED structure
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER FilePos;

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	FilePos.QuadPart = n * sizeof(s);
	
	// copy the displacement inside the OVERLAPPED structure
	ov.Offset = FilePos.LowPart;
	ov.OffsetHigh = FilePos.HighPart;
	
	_tprintf(_T("You required to read student with id = %d:\n"), id);
	
	// the read uses the OVERLAPPED
	if (ReadFile(hIn, &s, sizeof(s), &nRead, &ov) && nRead == sizeof(s)) {
		// read was successful
		_tprintf(_T("id: %d\treg_number: %ld\tsurname: %10s\tname: %10s\tmark: %d\n"), s.id, s.regNum, s.surname, s.name, s.mark);
	} else {
		// some errors in the read (out of bounds?)
		_ftprintf(stderr, _T("Error in read\n"));
	}
}

// this function performs the random acces by using an OVERLAPPED structure
VOID writeWithOverlappedDataStructures(INT id, HANDLE hIn) {
	STUDENT s;
	TCHAR command[CMD_MAX_LEN];
	DWORD nWritten;
	// create a "clean" OVERLAPPED structure
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER FilePos;

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	FilePos.QuadPart = n * sizeof(s);

	// copy the displacement inside the OVERLAPPED structure
	ov.Offset = FilePos.LowPart;
	ov.OffsetHigh = FilePos.HighPart;

	_tprintf(_T("You required to write student with id = %d.\nInsert the following data: \"registration_number surname name mark\"\n> "), id);

	// the id was already entered by the user, no need to ask it again
	s.id = id;

	// read a line
	while (_fgetts(command, CMD_MAX_LEN, stdin)) {
		// parse the line
		if (_stscanf(command, _T("%ld %s %s %d"), &s.regNum, s.surname, s.name, &s.mark) != 4) {
			// if parse fails, ask again to input the data
			_tprintf(_T("Error in the string. The format is the following \"registration_number surname name mark\"\n> "));
		} else {
			// if parse is correct, can go on
			break;
		}
	}

	// write the record
	if (WriteFile(hIn, &s, sizeof(s), &nWritten, &ov) && nWritten == sizeof(s)) {
		// write was successful
		_tprintf(_T("Record with id = %d stored\n"), s.id);
	} else {
		// some errors in the write
		_ftprintf(stderr, _T("Error in write\n"));
	}
}

// C
// this function is similar to the readOV, but locks the record before reading it
VOID readWithLocks(INT id, HANDLE hIn) {
	STUDENT s;
	// create a "clean" OVERLAPPED structure
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER filePos, size;
	DWORD nRead;

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	filePos.QuadPart = n * sizeof(s);

	// copy the displacement inside the OVERLAPPED structure
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;

	// fill the variable as QuadPart and then read as lowPart and hightPart to lock the file part
	size.QuadPart = sizeof(s);

	_tprintf(_T("You required to read student with id = %d:\n"), id);

	// lock the portion of file. The OVERLAPPED is used to specify the starting displacement of the record to be locked
	// and its size is specified on the 4th and 5th parameter
	if (!LockFileEx(hIn, 0, 0, size.LowPart, size.HighPart, &ov)) {
		_ftprintf(stderr, _T("Error locking file portion. Error: \n%s\n"), getErrorMessageAsString(GetLastError()));
		return;
	}

	// read the record
	if (ReadFile(hIn, &s, sizeof(s), &nRead, &ov) && nRead == sizeof(s)) {
		_tprintf(_T("id: %d\treg_number: %ld\tsurname: %10s\tname: %10s\tmark: %d\n"), s.id, s.regNum, s.surname, s.name, s.mark);
	} else {
		_ftprintf(stderr, _T("Error in read\n"));
	}
	
	// unlock the portion of file (arguments similar to the LockFileEx)
	if (!UnlockFileEx(hIn, 0, size.LowPart, size.HighPart, &ov)) {
		_ftprintf(stderr, _T("Error unlocking file portion. Error: \n%s\n"), getErrorMessageAsString(GetLastError()));
	}
}

VOID writeWithLocks(INT id, HANDLE hIn) {
	STUDENT s;
	// create a "clean" OVERLAPPED structure
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	// the index should be 0-based because at the beginning of the file the student with id = 1 is stored
	LONGLONG n = id - 1;
	LARGE_INTEGER filePos, size;
	DWORD nWritten;
	TCHAR command[CMD_MAX_LEN];

	// the displacement in the file is obtained by multiplying the index by the size of a single element
	filePos.QuadPart = n * sizeof(s);

	// copy the displacement inside the OVERLAPPED structure
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;

	// fill the variable as QuadPart and then read as lowPart and hightPart to lock the file part
	size.QuadPart = sizeof(s);

	_tprintf(_T("You required to write student with id = %d.\nInsert the following data: \"registration_number surname name mark\"\n> "), id);
	
	// the id was already entered by the user, no need to ask it again
	s.id = id;

	// read a line
	while (_fgetts(command, CMD_MAX_LEN, stdin)) {
		if (_stscanf(command, _T("%ld %s %s %d"), &s.regNum, s.surname, s.name, &s.mark) != 4) {
			// if parse fails, ask again to input the data
			_tprintf(_T("Error in the string. The format is the following \"registration_number surname name mark\"\n> "));
		} else {
			// if parse is correct, can go on
			break;
		}
	}

	// lock the portion of file. The OVERLAPPED is used to specify the starting displacement of the record to be locked
	// and its size is specified on the 4th and 5th parameter
	if (!LockFileEx(hIn, LOCKFILE_EXCLUSIVE_LOCK, 0, size.LowPart, size.HighPart, &ov)) {
		_ftprintf(stderr, _T("Error locking file portion. Error: \n%s\n"), getErrorMessageAsString(GetLastError()));
		return;
	}

	// write the record
	if (WriteFile(hIn, &s, sizeof(s), &nWritten, &ov) && nWritten == sizeof(s)) {
		// success writing
		_tprintf(_T("Record with id = %d stored\n"), s.id);
	} else {
		// error in writing
		_ftprintf(stderr, _T("Error in write\n"));
	}

	// unlock the portion of file
	if (!UnlockFileEx(hIn, 0, size.LowPart, size.HighPart, &ov)) {
		_ftprintf(stderr, _T("Error unlocking file portion. Error: \n%s\n"), getErrorMessageAsString(GetLastError()));
	}
}

// return waiting some time
int Return(int value) {
	Sleep(SECONDS * 1000);
	return value;
}

// get last error and convert errorCode to string human readable
LPWSTR getErrorMessageAsString(DWORD errorCode) {
	LPWSTR errString = NULL;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		errorCode,
		0,
		(LPWSTR)&errString,
		0,
		0);

	return errString;
}
