/******************** Start of BITIO.C ********************/
/*
* This utility file contains all of the routines needed to implement
* bit oriented routines under either ANSI or K&R C. It needs to be
* linked with every program used in the book.
*/

#include "bitio.h"
//#include "errhand.h"
#include <fstream>

#define PACIFIER_COUNT 2047

BIT_FILE* OpenOutputBitFile(char* name)
{
	BIT_FILE* bit_file;
	bit_file = (BIT_FILE*)calloc(1, sizeof(BIT_FILE));
	if (bit_file == NULL)
		return(bit_file);
	FILE* fp;
	errno_t err = fopen_s(&fp, name, "wb");
	bit_file->file = fp;
	
	//ofstream outputFile(name, ios::binary);
	//bit_file->file = &outputFile;
	bit_file->rack = 0;
	bit_file->mask = 0x80;
	bit_file->pacifier_counter = 0;
	bit_file->byte_count = 0;

	return(bit_file);
}

BIT_FILE* OpenInputBitFile(char* name)
{
	BIT_FILE* bit_file;
	bit_file = (BIT_FILE*)calloc(1, sizeof(BIT_FILE));
	if (bit_file == NULL)
		return(bit_file);
	FILE* fp;
	errno_t err = fopen_s(&fp, name, "rb");
	bit_file->file = fp;
	bit_file->rack = 0;
	bit_file->mask = 0x80;
	bit_file->pacifier_counter = 0;
	return(bit_file);
}

void CloseOutputBitFile(BIT_FILE* bit_file)
{
	if (bit_file->mask != 0x80)
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack)
			fatal_error("Fatal error in CloseBitFile!\n");
	fclose(bit_file->file);
	free((char*)bit_file);
}

void CloseInputBitFile(BIT_FILE* bit_file)
{
	fclose(bit_file->file);
	free((char*)bit_file);
}

void OutputBit(BIT_FILE* bit_file, int bit)
{
	if (bit)
		bit_file->rack |= bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0) {
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack)
			fatal_error("Fatal error in OutputBit!\n");
		else if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
			putc('.', stdout);
			bit_file->byte_count += 1;
		bit_file->rack = 0;
		bit_file->mask = 0x80;
	}
}

//for bit plane
void OutputBit(vector<bool>* bit_plane, BIT_FILE* bit_file, int bit)
{
	if (bit)
		bit_file->rack |= bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0) {
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack)
			fatal_error("Fatal error in OutputBit!\n");
		else if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
			putc('.', stdout);
			bit_plane->push_back(bit_file->rack);
			bit_file->byte_count += 1;
		bit_file->rack = 0;
		bit_file->mask = 0x80;
	}
}

void OutputBits(BIT_FILE* bit_file, unsigned long code, int count)
{
	unsigned long mask;

	mask = 1L << (count - 1);
	while (mask != 0) {
		if (mask & code)
			bit_file->rack |= bit_file->mask;
		bit_file->mask >>= 1;

		if (bit_file->mask == 0) {
			if (putc(bit_file->rack, bit_file->file) != bit_file->rack)
				fatal_error("Fatal error in OutputBit!\n");
			else if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
				bit_file->byte_count += 1;
			bit_file->rack = 0;
			bit_file->mask = 0x80;

		}
		mask >>= 1;
	}
}
int InputBit(BIT_FILE* bit_file)
{
	int value;

	if (bit_file->mask == 0x80) {
		bit_file->rack = getc(bit_file->file);
		if (bit_file->rack == EOF)
			//fatal_error("Fatal error in InputBit!\n");
			return EOF;
		if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
			cout << ".";
	}
	value = bit_file->rack & bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0)
		bit_file->mask = 0x80;
	return (value ? 1 : 0);
}


////bit plane input
//int InputPlaneBit(BIT_FILE* bit_file, int pos)
//{
//	int value;
//
//	bit_file->rack = getc(bit_file->file);
//	if (bit_file->rack == EOF)
//		//fatal_error("Fatal error in InputBit!\n");
//		return EOF;
//	if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
//		cout << ".";
//	
//	value = (bit_file->rack >> (8-pos)) & 0x01;
//
//	return value;
//}

unsigned long InputBits(BIT_FILE* bit_file, int bit_count)
{
	unsigned long mask;
	unsigned long return_value;

	mask = 1L << (bit_count - 1);
	return_value = 0;
	while (mask != 0) {
		if (bit_file->mask == 0x80) {
			bit_file->rack = getc(bit_file->file);
			if (bit_file->rack == EOF)
				//fatal_error("Fatal error in InputBits!\n");
				return EOF;
			if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
		}

		if (bit_file->rack & bit_file->mask)
			return_value |= mask;
		mask >>= 1;
		bit_file->mask >>= 1;
		if (bit_file->mask == 0)
			bit_file->mask = 0x80;
	}
	return(return_value);
}
void FilePrintBinary(FILE* file, unsigned int code, int bits)
{
	unsigned int mask;
	mask = 1 << (bits - 1);
	while (mask != 0) {
		if (code & mask)
			fputc('1', file);
		else
			fputc('0', file);
		mask >>= 1;
	}
}

void fatal_error(string error_message) {
	cout << error_message;
}
/********************** End of BITIO.C **********************/

