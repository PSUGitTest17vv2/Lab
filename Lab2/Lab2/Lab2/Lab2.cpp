#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

using namespace std;

struct block
{
	int wholeBlock; //целый блок
	int incompleteBlock; //неполный блок
};

int GetTheNumber(char inputData[], const int SIZE);
void TextEncryption(char inputBuffer[], const int SIZE, const int arrKey[], int number);
void TextDecryption(char inputBuffer[], const int sizeBuffer, const int arrKey[], const int sizeKey, int number);

void WriteToFile(const char inputData[], const int SIZE, string directory);
void DeleteFileContents(string directory);

void WriteToFile(block& inputData);
void ReadToFile(block& inputData);

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
				const int N = 32;
				const int arrKey[N] = { 6, 29, 17, 3, 26, 13, 7, 0, 8, 15, 12, 19, 21, 5, 28, 16, 23, 24, 1, 2, 25, 30, 31, 10, 9, 14, 27, 18, 22, 4, 11, 20 };
				char arrBuffer[N / 8];

				/* Шифрование */
				block b{};
				ifstream finE;
				finE.open(argv[i + 1], ios_base::binary);

				if (!finE.is_open())
				{
					exit(EXIT_FAILURE);
				}

				DeleteFileContents(argv[i + 2]);

				do
				{
					finE.read(arrBuffer, (N / 8));

					if ((int)finE.gcount())
					{
						for (int i = (int)finE.gcount(); i < (N / 8); i++)
						{
							arrBuffer[i] = NULL;
						}

						TextEncryption(arrBuffer, (N / 8), arrKey, GetTheNumber(arrBuffer, (N / 8)));
						WriteToFile(arrBuffer, (N / 8), argv[i + 2]);

						((int)finE.gcount() < (N / 8)) ? b.incompleteBlock = (int)finE.gcount() : b.wholeBlock++;
					}

				} while (!finE.eof());

				finE.close();
				WriteToFile(b);
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
				const int N = 32;
				const int arrKey[N] = { 6, 29, 17, 3, 26, 13, 7, 0, 8, 15, 12, 19, 21, 5, 28, 16, 23, 24, 1, 2, 25, 30, 31, 10, 9, 14, 27, 18, 22, 4, 11, 20 };
				char arrBuffer[N / 8];

				/* Расшифровывание */
				ifstream finD;
				finD.open(argv[i + 1], ios_base::binary);

				if (!finD.is_open())
				{
					exit(EXIT_FAILURE);
				}

				DeleteFileContents(argv[i + 2]);
				block b{};
				ReadToFile(b);

				do
				{
					finD.read(arrBuffer, (N / 8));

					if (b.wholeBlock--)
					{
						TextDecryption(arrBuffer, (N / 8), arrKey, N, GetTheNumber(arrBuffer, (N / 8)));
						WriteToFile(arrBuffer, (N / 8), argv[i + 2]);

						if (!b.wholeBlock && b.incompleteBlock)
						{
							finD.read(arrBuffer, (N / 8));
							TextDecryption(arrBuffer, (N / 8), arrKey, N, GetTheNumber(arrBuffer, (N / 8)));

							char* arrDecryption = new char[b.incompleteBlock];

							for (int i = 0; i < b.incompleteBlock; i++)
							{
								*(arrDecryption + i) = arrBuffer[i];
							}

							WriteToFile(arrDecryption, b.incompleteBlock, argv[i + 2]);
							break;
						}
					}

				} while (!finD.eof());

				finD.close();
			}
			else
			{
				cout << "\n-d [диск1:][путь1]имя_файла1 [диск2:][путь2]имя_файла2\n";
			}
		}
	}

	return 0;
}

/* Получить целое число */
int GetTheNumber(char inputData[], const int SIZE)
{
	int number = 0;

	for (int i = 0; i < (SIZE - 1); i++)
	{
		number += (int)(unsigned char)inputData[i];
		number <<= 8;
	}

	number += (int)(unsigned char)inputData[SIZE - 1];

	return number;
}
/* Шифрование + запись в буфер */
void TextEncryption(char inputBuffer[], const int SIZE, const int arrKey[], int number)
{
	for (int i = 0; i < SIZE; i++)
	{
		inputBuffer[i] = 0;

		for (int j = 0; j < 7; j++)
		{
			inputBuffer[i] += (int)(bool)((1 << arrKey[8 * i + j]) & number);
			inputBuffer[i] <<= 1;
		}

		inputBuffer[i] += (int)(bool)((1 << arrKey[8 * i + 7]) & number);
	}
}
/* Расшифрование + запись в буфер */
void TextDecryption(char inputBuffer[], const int sizeBuffer, const int arrKey[], const int sizeKey, int number)
{
	for (int index, i = 0; i < sizeBuffer; i++)
	{
		inputBuffer[i] = 0;

		for (int j = 0; j < 7; j++)
		{
			index = 0;

			for (; arrKey[index] != ((sizeKey - 1) - (8 * i + j)); index++);

			inputBuffer[i] += (int)(bool)((1 << (sizeKey - 1 - index)) & number);
			inputBuffer[i] <<= 1;
		}

		index = 0;
		for (; arrKey[index] != ((sizeKey - 1) - (8 * i + 7)); index++);
		inputBuffer[i] += (int)(bool)((1 << (sizeKey - 1 - index)) & number);
	}
}

/* Запись в файл */
void WriteToFile(const char inputData[], const int SIZE, string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::app);

	if (!fout.is_open())
	{
		exit(EXIT_FAILURE);
	}

	fout.write(inputData, SIZE);
	fout.close();
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