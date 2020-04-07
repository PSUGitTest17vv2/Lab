#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <cstdlib>
#include <string>
#include <cstring>

using namespace std;

struct SupportingInfo
{
	bool startStopRecording = false;
	const char SEText = -2;
	bool isEndText = true;
	int arrKey[8] = { 2, 4, 0, 7, 1, 5, 6, 3 };
};

struct BitMapHeader
{
	int size;		// ������ �����
	int offsetBits; // �������� ����������� �� ������ �����
	short bitCount; // ������� �����
	int sizeImage;  // ������ ����������� (����)
};

short getTypeValueShort(const char arr[]);
int getTypeValueInteger(const char arr[]);
char getCharacter(const char arr[], const int SIZE);

bool isBitMapFile(string directory, BitMapHeader& bmh);
int getfileSize(string directory);

void WriteToFile(const char inputData[], const int SIZE, string directory);
void WriteToFile(char symbol, string directory);
void DeleteFileContents(string directory);

void steganographyLSB(char inputData[], const int SIZE, char symbol);

char characterEncryption(char symbol, const int key[]);
char characterDecryption(char symbol, const int key[]);

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "rus");

	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "/?"))
		{
			cout << "\n[-embed] [-retrieve] [����1:][����1]���_�����1 [����2:][����2]���_�����2\n[-check] [����1:][����1]���_�����1\n"
				<< "\n  -embed      �������� \"���������\" ������� ���_�����1 � ���_�����2.bmp"
				<< "\n  -retrieve   ��������� \"���������\" ������� �� ���_�����1.bmp � �������� ��� � ���_�����2"
				<< "\n  -check      ��������� ������� \"����������\" ����������� � ���_�����1.bmp\n";
		}
		else if (!strcmp(argv[i], "-embed"))
		{
			if ((argv[i + 1] != NULL && argv[i + 2] != NULL) && strstr(argv[i + 2], ".bmp"))
			{
				BitMapHeader bmh;

				if (isBitMapFile(argv[i + 2], bmh) && (bmh.bitCount == 24) && ((bmh.sizeImage / 4) >= (getfileSize(argv[i + 1]) + 2)))
				{
					ifstream finTXT;
					finTXT.open(argv[i + 1], ios_base::binary);

					if (!finTXT.is_open())
					{
						EXIT_FAILURE;
					}

					ifstream finBMP;
					finBMP.open(argv[i + 2], ios_base::binary);

					if (!finBMP.is_open())
					{
						EXIT_FAILURE;
					}

					//////////////////////////////////////////////////////////////////////////////////////////

					char* arrBMH = new char[bmh.offsetBits];
					finBMP.read(arrBMH, bmh.offsetBits);
					DeleteFileContents("steganography.bmp");
					WriteToFile(arrBMH, bmh.offsetBits, "steganography.bmp");
					delete[] arrBMH;

					SupportingInfo si;

					const int N = 4;
					char arrBuffer[4];
					char symbolBuffer;

					finBMP.read(arrBuffer, N);
					steganographyLSB(arrBuffer, N, characterEncryption(si.SEText, si.arrKey)); // �������� ������� (������ ������)
					WriteToFile(arrBuffer, N, "steganography.bmp");

					do
					{
						finBMP.read(arrBuffer, N);
						symbolBuffer = finTXT.get();

						if (!finTXT.eof())
						{
							steganographyLSB(arrBuffer, N, characterEncryption(symbolBuffer, si.arrKey));
						}
						else if (si.isEndText)
						{
							steganographyLSB(arrBuffer, N, characterEncryption(si.SEText, si.arrKey)); // �������� ������� (����� ������)
							si.isEndText = false;
						}

						WriteToFile(arrBuffer, N, "steganography.bmp");

					} while (!finBMP.eof());

					//////////////////////////////////////////////////////////////////////////////////////////

					finBMP.close();
					finTXT.close();

					ifstream finStegBMP;
					finStegBMP.open("steganography.bmp", ios_base::binary);

					if (!finStegBMP.is_open())
					{
						EXIT_FAILURE;
					}

					//////////////////////////////////////////////////////////////////////////////////////////

					DeleteFileContents(argv[i + 2]);
					
					finStegBMP.seekg(0, finStegBMP.end);
					const int SIZE = (int)finStegBMP.tellg();
					char* buffer = new char[SIZE];
					finStegBMP.seekg(0, finStegBMP.beg);

					finStegBMP.read(buffer, SIZE);
					WriteToFile(buffer, SIZE, argv[i + 2]);

					/*char buffer;

					do
					{
						buffer = finStegBMP.get();
						WriteToFile(buffer, argv[i + 2]);
					} while (!finStegBMP.eof());*/

					//////////////////////////////////////////////////////////////////////////////////////////

					finStegBMP.close();
				}
				else
				{
					cout << "\n��������� ������ ��� ���������� ��������...\n";
				}
			}
			else
			{
				cout << "\n-embed [����1:][����1]���_�����1 [����2:][����2]���_�����2\n";
			}
		}
		else if (!strcmp(argv[i], "-retrieve"))
		{
			if ((argv[i + 1] != NULL && argv[i + 2] != NULL) && strstr(argv[i + 1], ".bmp"))
			{
				BitMapHeader bmh;

				if (isBitMapFile(argv[i + 1], bmh) && (bmh.bitCount == 24))
				{
					ifstream finBMP;
					finBMP.open(argv[i + 1], ios_base::binary);

					if (!finBMP.is_open())
					{
						EXIT_FAILURE;
					}

					//////////////////////////////////////////////////////////////////////////////////////////

					char* arrBMH = new char[bmh.offsetBits];
					finBMP.read(arrBMH, bmh.offsetBits);
					delete[] arrBMH;
					DeleteFileContents(argv[i + 2]);

					SupportingInfo si;

					const int N = 4;
					char arrBuffer[4];
					char symbolBuffer;

					do
					{
						finBMP.read(arrBuffer, N);

						symbolBuffer = characterDecryption(getCharacter(arrBuffer, N), si.arrKey);

						if (symbolBuffer == -2)
						{
							si.startStopRecording = !si.startStopRecording;
							if (!si.startStopRecording)
								break;
						}
						else if (si.startStopRecording)
						{
							WriteToFile(symbolBuffer, argv[i + 2]);
						}

					} while (!finBMP.eof());

					//////////////////////////////////////////////////////////////////////////////////////////

					finBMP.close();
				}
				else
				{
					cout << "\n��������� ������ ��� ���������� ��������...\n";
				}
			}
			else
			{
				cout << "\n-retrieve [����1:][����1]���_�����1 [����2:][����2]���_�����2\n";
			}
		}
		else if (!strcmp(argv[i], "-check"))
		{
			if (argv[i + 1] != NULL && strstr(argv[i + 1], ".bmp"))
			{
				BitMapHeader bmh;

				if (isBitMapFile(argv[i + 1], bmh) && (bmh.bitCount == 24))
				{
					namespace fs = std::experimental::filesystem;
					int fileSizeBMP;

					try {
						fileSizeBMP = (int)fs::file_size(argv[i + 1]);

						if (bmh.size != fileSizeBMP)
							cout << "\n� ����� ������������ ������� \"��������\" �����������\n";
						else
							cout << "\n� ����� ����������� ������� \"��������\" �����������\n";
					}
					catch (fs::filesystem_error& ex) {
						cout << ex.what() << '\n';
					}
				}
				else
				{
					cout << "\n��������� ������ ��� ���������� ��������...\n";
				}
			}
			else
			{
				cout << "\n-check [����1:][����1]���_�����1\n";
			}
		}
	}

	return 0;
}


/* �������� �������� ���� short */
short getTypeValueShort(const char arr[])
{
	int value = 0;

	for (int i = 1; i > 0; i--)
	{
		if (arr[i] != NULL)
		{
			value += (unsigned char)arr[i];
			value <<= 8;
		}
	}

	return value += (unsigned char)arr[0];
}
/* �������� �������� ���� int */
int getTypeValueInteger(const char arr[])
{
	int value = 0;

	for (int i = 3; i > 0; i--)
	{
		if (arr[i] != NULL)
		{
			value |= (unsigned char)arr[i];
			value <<= 8;
		}
	}

	return value |= (unsigned char)arr[0];
}
/* �������� ������ �� ������� BMP ����� */
char getCharacter(const char arr[], const int SIZE)
{
	char symbol = NULL;

	for (int i = 0; i < (SIZE - 1); i++)
	{
		symbol = (symbol + (bool)((1 << 1) & arr[i])) << 1;
		symbol = (symbol + (bool)((1 << 0) & arr[i])) << 1;
	}

	symbol = (symbol + (bool)((1 << 1) & arr[SIZE - 1])) << 1;
	symbol = (symbol + (bool)((1 << 0) & arr[SIZE - 1]));

	return symbol;
}

/* ��� BMP ���� */
bool isBitMapFile(string directory, BitMapHeader& bmh)
{
	ifstream fin;
	fin.open(directory, ios_base::binary);

	if (!fin.is_open())
	{
		EXIT_FAILURE;
	}

	fin.seekg(0, fin.end);
	int fileLengthBMP = (int)fin.tellg();
	fin.seekg(0, fin.beg);

	char arrType[2];
	char arrSize[4];

	fin.read(arrType, 2);
	fin.read(arrSize, 4);

	bmh.size = getTypeValueInteger(arrSize);

	while (fin.tellg() < 38)
	{
		fin.get();

		switch (fin.tellg())
		{
		case 10:
		{
			char arrOffsetBits[4];
			fin.read(arrOffsetBits, 4);
			bmh.offsetBits = getTypeValueInteger(arrOffsetBits);
		}
		break;
		case 28:
		{
			char arrBitCount[2];
			fin.read(arrBitCount, 2);
			bmh.bitCount = getTypeValueShort(arrBitCount);
		}
		break;
		case 34:
		{
			char arrSizeImage[4];
			fin.read(arrSizeImage, 4);
			bmh.sizeImage = getTypeValueInteger(arrSizeImage);
		}
		break;
		}
	}

	fin.close();

	namespace fs = std::experimental::filesystem;
	int fileSizeBMP;

	try {
		fileSizeBMP = (int)fs::file_size(directory);
	}
	catch (fs::filesystem_error& ex) {
		cout << ex.what() << '\n';
	}

	return ((arrType[0] == 'B' && arrType[1] == 'M') && (fileSizeBMP == fileLengthBMP));
}
/* �������� ������ TXT ����� */
int getfileSize(string directory)
{
	ifstream fin;
	fin.open(directory, ios_base::binary);

	if (!fin.is_open())
	{
		EXIT_FAILURE;
	}

	fin.seekg(0, fin.end);
	int fileLengthTXT = (int)fin.tellg();
	fin.close();

	return fileLengthTXT;
}

/* �������� � ���� ������ */
void WriteToFile(const char inputData[], const int SIZE, string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::app);

	if (!fout.is_open())
	{
		EXIT_FAILURE;
	}

	fout.write(inputData, SIZE);
	fout.close();
}
/* �������� � ���� ������� */
void WriteToFile(char symbol, string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::app);

	if (!fout.is_open())
	{
		EXIT_FAILURE;
	}

	fout << symbol;
	fout.close();
}
/* �������� ���������� ����� */
void DeleteFileContents(string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::trunc);

	if (!fout.is_open())
	{
		EXIT_FAILURE;
	}

	fout.close();
}

/* �������� ���������� */
void steganographyLSB(char inputData[], const int SIZE, char symbol)
{
	int index = 7;

	for (int i = 0; i < SIZE; i++)
	{
		inputData[i] >>= 2;
		inputData[i] <<= 1;
		inputData[i] = (inputData[i] + (bool)((1 << index) & symbol)) << 1;
		index--;
		inputData[i] += (bool)((1 << index) & symbol);
		index--;
	}
}

/* ����������� ������ */
char characterEncryption(char symbol, const int key[])
{
	char symbolEncryption = NULL;

	for (int i = 0; i < 7; i++)
	{
		symbolEncryption = (symbolEncryption + (bool)((1 << key[i]) & symbol)) << 1;
	}

	symbolEncryption = symbolEncryption + (bool)((1 << key[7]) & symbol);

	return symbolEncryption;
}
/* ������������ ������ */
char characterDecryption(char symbol, const int key[])
{
	char symbolDecryption = NULL;

	for (int i = 7; i >= 0; i--)
	{
		int index = 0;
		for (; key[index] != i; index++);

		if (!i)
			symbolDecryption = (symbolDecryption + (bool)((1 << (7 - index)) & symbol));
		else
			symbolDecryption = (symbolDecryption + (bool)((1 << (7 - index)) & symbol)) << 1;
	}

	return symbolDecryption;
}