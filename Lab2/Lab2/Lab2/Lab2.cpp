#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <string>

using namespace std;

const unsigned BufferSize = 32768U;
typedef char(*arraypointer)[BufferSize];

struct block
{
	long long wholeBlock; //целый блок
	long long incompleteBlock; //неполный блок
};

void TextEncryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength);
void TextDecryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength);

unsigned GetNumber(char* arrBegin, char* arrEnd);
void SwapAddress(arraypointer* a, arraypointer* b);

void DeleteFileContents(string directory);
void WriteToFile(block& inputData);
void ReadToFile(block& inputData);

void WriteToFile(char arrInputData[], const unsigned inputBufferSize, string directory);
void AsyncReadToFile(ifstream& fin, arraypointer arr[], int& offsetRead);


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "rus");

	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "/?"))
		{
			cout << "[-e] [-d] [диск1:][путь1]имя_файла1 [диск2:][путь2]имя_файла2\n"
				<< "\n  -e    Зашифровать имя_файла1 имя_файла2"
				<< "\n  -d    Расшифровать имя_файла1 имя_файла2\n\n";
		}
		else if (!strcmp(argv[i], "-e"))
		{
			if (argv[i + 1] != NULL && argv[i + 2] != NULL)
			{
				const unsigned N = 32;
				int arrKey[N] = { 6, 29, 17, 3, 26, 13, 7, 0, 8, 15, 12, 19, 21, 5, 28, 16, 23, 24, 1, 2, 25, 30, 31, 10, 9, 14, 27, 18, 22, 4, 11, 20 };

				char arrReadBuffer[BufferSize];
				char arrProcessingBuffer[BufferSize];
				arraypointer arrBuffer[2]{ &arrReadBuffer ,&arrProcessingBuffer };

				/* Шифрование */
				ifstream finE;
				finE.open(argv[i + 1], ios_base::binary);

				if (!finE.is_open())
					exit(EXIT_FAILURE);

				finE.read(**arrBuffer, BufferSize);

				if ((int)finE.gcount())
				{
					int offset = (int)finE.gcount();
					thread threadARF(AsyncReadToFile, ref(finE), arrBuffer, ref(offset));

					DeleteFileContents(argv[i + 2]);
					block b{};

					while (true)
					{
						if (offset)
						{
							SwapAddress(arrBuffer, arrBuffer + 1);
							unsigned arraySize = offset;
							offset = 0;

							b.wholeBlock += arraySize / (N / 8);
							b.incompleteBlock += arraySize % (N / 8);

							while (arraySize % (N / 8) != 0 && arraySize < BufferSize)
							{
								*(**(arrBuffer + 1) + arraySize) = ' ';
								arraySize++;
							}

							TextEncryption(**(arrBuffer + 1), (**(arrBuffer + 1) + arraySize - 1), arrKey, (N / 8));
							WriteToFile(**(arrBuffer + 1), arraySize, argv[i + 2]);

							if (finE.eof())
								break;
						}
					}

					threadARF.join();
					WriteToFile(b);
					finE.close();
				}
			}
			else
			{
				cout << "\n-e [диск1:][путь1]имя_файла1 [диск2:][путь2]имя_файла2\n";
			}
		}
		else if (!strcmp(argv[i], "-d"))
		{
			if (argv[i + 1] != NULL && argv[i + 2] != NULL)
			{
				const unsigned N = 32;
				int arrKey[N] = { 6, 29, 17, 3, 26, 13, 7, 0, 8, 15, 12, 19, 21, 5, 28, 16, 23, 24, 1, 2, 25, 30, 31, 10, 9, 14, 27, 18, 22, 4, 11, 20 };

				char arrReadBuffer[BufferSize];
				char arrProcessingBuffer[BufferSize];
				arraypointer arrBuffer[2]{ &arrReadBuffer ,&arrProcessingBuffer };

				/* Расшифровывание */
				ifstream finD;
				finD.open(argv[i + 1], ios_base::binary);

				if (!finD.is_open())
					exit(EXIT_FAILURE);

				finD.read(**arrBuffer, BufferSize);

				if ((int)finD.gcount())
				{
					int offset = (int)finD.gcount();
					thread threadARF(AsyncReadToFile, ref(finD), arrBuffer, ref(offset));

					DeleteFileContents(argv[i + 2]);
					block b{};
					ReadToFile(b);
					long long fileSize = b.wholeBlock * (N / 8) + b.incompleteBlock;

					while (true)
					{
						if (offset)
						{
							SwapAddress(arrBuffer, arrBuffer + 1);
							unsigned arraySize = offset;
							fileSize -= offset;
							offset = 0;

							while (arraySize % (N / 8) != 0 && arraySize < BufferSize)
							{
								*(**(arrBuffer + 1) + arraySize) = ' ';
								arraySize++;
							}

							TextDecryption(**(arrBuffer + 1), (**(arrBuffer + 1) + arraySize - 1), arrKey, N);

							if (fileSize < 0LL)
								arraySize += (int)fileSize;

							WriteToFile(**(arrBuffer + 1), arraySize, argv[i + 2]);

							if (finD.eof() || fileSize < 0LL)
								break;
						}
					}

					threadARF.detach();
					WriteToFile(b);
					finD.close();
				}
			}
			else
			{
				cout << "\n-d [диск1:][путь1]имя_файла1 [диск2:][путь2]имя_файла2\n";
			}
		}
	}

	return 0;
}

/* Шифрование */
void TextEncryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength)
{
	// keyLength = 4

	const unsigned offsetSize = arrEnd - arrBegin + 1;

	for (; arrBegin < arrEnd; arrBegin += keyLength)
	{
		unsigned number = GetNumber(arrBegin, arrBegin + keyLength);

		for (size_t i = 0; i < keyLength; i++)
		{
			*(arrBegin + i) = 0;

			for (size_t j = 0; j < 7; j++)
			{
				*(arrBegin + i) += (int)(bool)((1 << arrKey[8 * i + j]) & number);
				*(arrBegin + i) <<= 1;
			}

			*(arrBegin + i) += (int)(bool)((1 << arrKey[8 * i + 7]) & number);
		}
	}

	arrBegin -= offsetSize;
}
/* Расшифровывание */
void TextDecryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength)
{
	// keyLength = 32
	// (keyLength / 8) = 4

	const unsigned offsetSize = arrEnd - arrBegin + 1;

	for (; arrBegin < arrEnd; arrBegin += (keyLength / 8))
	{
		unsigned number = GetNumber(arrBegin, arrBegin + (keyLength / 8));

		for (size_t index, i = 0; i < (keyLength / 8); i++)
		{
			*(arrBegin + i) = 0;

			for (size_t j = 0; j < 7; j++)
			{
				index = 0;
				for (; arrKey[index] != ((keyLength - 1) - (8 * i + j)); index++);

				*(arrBegin + i) += (int)(bool)((1 << (keyLength - 1 - index)) & number);
				*(arrBegin + i) <<= 1;
			}

			index = 0;
			for (; arrKey[index] != ((keyLength - 1) - (8 * i + 7)); index++);

			*(arrBegin + i) += (int)(bool)((1 << (keyLength - 1 - index)) & number);
		}
	}

	arrBegin -= offsetSize;
}

/* Получить целое число */
unsigned GetNumber(char* arrBegin, char* arrEnd)
{
	unsigned number = 0;

	for (; arrBegin < (arrEnd - 1); arrBegin++)
	{
		number += (unsigned)(unsigned char)*arrBegin;
		number <<= 8;
	}

	number += (unsigned)(unsigned char)*arrBegin;
	arrBegin -= 3;

	return number;
}
/* Поменять адреса местами */
void SwapAddress(arraypointer* a, arraypointer* b)
{
	arraypointer temp = *a;
	*a = *b;
	*b = temp;
}

/* Удалить содержимое файла */
void DeleteFileContents(string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::trunc);

	if (!fout.is_open())
	{
		exit(EXIT_FAILURE);
	}

	fout.close();
}
/* Запись блока в файл */
void WriteToFile(block& inputData)
{
	ofstream fout;
	fout.open("block.txt");

	if (!fout.is_open())
	{
		exit(EXIT_FAILURE);
	}

	fout << inputData.wholeBlock << " " << inputData.incompleteBlock;
	fout.close();
}
/* Чтение блока из файл */
void ReadToFile(block& inputData)
{
	ifstream fin;
	fin.open("block.txt");

	if (!fin.is_open())
	{
		exit(EXIT_FAILURE);
	}

	fin >> inputData.wholeBlock >> inputData.incompleteBlock;
	fin.close();
}

/* Записать в файл */
void WriteToFile(char arrInputData[], const unsigned inputBufferSize, string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::app);

	if (!fout.is_open())
	{
		exit(EXIT_FAILURE);
	}

	fout.write(arrInputData, inputBufferSize);
	fout.close();
}
/* Асинхронное чтение из файл */
void AsyncReadToFile(ifstream& fin, arraypointer arr[], int& offsetRead)
{
	do
	{
		if (!offsetRead)
		{
			fin.read(**arr, BufferSize);
			offsetRead = (int)fin.gcount();
		}

		this_thread::sleep_for(chrono::milliseconds(20));

	} while (!fin.eof());
}