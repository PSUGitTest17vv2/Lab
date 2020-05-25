#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <string>

using namespace std;

const unsigned BufferSize = 32000U;
typedef char(*arraypointer)[BufferSize];

struct block
{
	long long wholeBlock; //целый блок
	long long incompleteBlock; //неполный блок
};

char* TextEncryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength);
char* TextDecryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength);

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
				const unsigned N = 10;
				int arrKey[N] = { 3, 9, 10, 5, 7, 1, 2, 8, 6, 4 };

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

							b.wholeBlock += arraySize / N;
							b.incompleteBlock += arraySize % N;

							while (arraySize % N != 0 && arraySize < BufferSize)
							{
								*(**(arrBuffer + 1) + arraySize) = ' ';
								arraySize++;
							}

							char* outputTE = TextEncryption(**(arrBuffer + 1), (**(arrBuffer + 1) + arraySize - 1), arrKey, N);
							WriteToFile(outputTE, arraySize, argv[i + 2]);
							delete[] outputTE;

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
				const unsigned N = 10;
				int arrKey[N] = { 3, 9, 10, 5, 7, 1, 2, 8, 6, 4 };

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
					long long fileSize = b.wholeBlock * N + b.incompleteBlock;

					while (true)
					{
						if (offset)
						{
							SwapAddress(arrBuffer, arrBuffer + 1);
							unsigned arraySize = offset;
							fileSize -= offset;
							offset = 0;

							while (arraySize % N != 0 && arraySize < BufferSize)
							{
								*(**(arrBuffer + 1) + arraySize) = ' ';
								arraySize++;
							}

							char* outputTD = TextDecryption(**(arrBuffer + 1), (**(arrBuffer + 1) + arraySize - 1), arrKey, N);

							if (fileSize < 0LL)
								arraySize += (int)fileSize;

							WriteToFile(outputTD, arraySize, argv[i + 2]);
							delete[] outputTD;

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
char* TextEncryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength)
{
	const unsigned offsetSize = arrEnd - arrBegin + 1;
	char* arrTextEncryption = new char[offsetSize];

	for (; arrBegin < arrEnd; arrBegin += keyLength)
	{
		for (size_t i = 0; i < keyLength; i++, arrTextEncryption++)
		{
			*arrTextEncryption = *(arrBegin + arrKey[i] - 1);
		}
	}

	arrBegin -= offsetSize;
	arrTextEncryption -= offsetSize;

	return arrTextEncryption;
}
/* Расшифровывание */
char* TextDecryption(char* arrBegin, char* arrEnd, int arrKey[], const unsigned keyLength)
{
	const unsigned offsetSize = arrEnd - arrBegin + 1;
	char* arrTextDecryption = new char[offsetSize];

	for (; arrBegin < arrEnd; arrBegin += keyLength)
	{
		for (size_t i = 0; i < keyLength; i++, arrTextDecryption++)
		{
			unsigned index = 0;
			for (; arrKey[index] != (i + 1); index++);

			*arrTextDecryption = *(arrBegin + index);
		}
	}

	arrBegin -= offsetSize;
	arrTextDecryption -= offsetSize;

	return arrTextDecryption;
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