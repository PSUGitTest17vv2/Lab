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

string TextEncryption(char inputData[], int arrKey[], const int keyLength);
string TextDecryption(char inputData[], int arrKey[], const int keyLength);

void WriteToFile(const string& inputData, string directory);
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
				const int N = 10;
				int arrKey[N] = { 3, 9, 10, 5, 7, 1, 2, 8, 6, 4 };
				char arrBuffer[N];

				/* Шифрование */
				ifstream finE;
				finE.open(argv[i + 1], ios_base::binary);

				if (!finE.is_open())
				{
					exit(EXIT_FAILURE);
				}

				DeleteFileContents(argv[i + 2]);
				block b{};

				do
				{
					finE.read(arrBuffer, N);					

					if ((int)finE.gcount())
					{
						for (int i = (int)finE.gcount(); i < N; i++)
						{
							arrBuffer[i] = ' ';
						}

						WriteToFile(TextEncryption(arrBuffer, arrKey, N), argv[i + 2]);

						((int)finE.gcount() < N) ? b.incompleteBlock = (int)finE.gcount() : b.wholeBlock++;
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
				const int N = 10;
				int arrKey[N] = { 3, 9, 10, 5, 7, 1, 2, 8, 6, 4 };
				char arrBuffer[N];

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
					finD.read(arrBuffer, N);

					if (b.wholeBlock)
					{
						WriteToFile(TextDecryption(arrBuffer, arrKey, N), argv[i + 2]);
						b.wholeBlock--;
					}
					else if (b.incompleteBlock)
					{
						string strTime = TextDecryption(arrBuffer, arrKey, N);
						string strDecryption;

						for (int i = 0; i < b.incompleteBlock; i++)
						{
							strDecryption += strTime[i];
						}

						WriteToFile(strDecryption, argv[i + 2]);
						break;
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

/* Шифрование */
string TextEncryption(char inputData[], int arrKey[], const int keyLength)
{
	string textEncryption;

	for (int i = 0; i < keyLength; i++)
	{
		textEncryption += inputData[arrKey[i] - 1];
	}

	return textEncryption;
}
/* Расшифровывание */
string TextDecryption(char inputData[], int arrKey[], const int keyLength)
{
	string textDecryption;

	for (int i = 0; i < keyLength; i++)
	{
		int index = 0;

		for (; arrKey[index] != (i + 1); index++);

		textDecryption += inputData[index];
	}

	return textDecryption;
}

/* Записать в файл */
void WriteToFile(const string& inputData, string directory)
{
	ofstream fout;
	fout.open(directory, ios_base::binary | ios_base::app);

	if (!fout.is_open())
	{
		exit(EXIT_FAILURE);
	}

	//fout << inputData;
	fout.write(inputData.c_str(), inputData.size());
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