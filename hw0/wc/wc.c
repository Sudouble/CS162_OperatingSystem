#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

bool IsWord(char ch)
{
	return (ch>='0' && ch<='9')||(ch>='A'&&ch<='Z')||(ch>='a'&&ch<='z');
}

int countWordLine(string line)
{
	int nCount = 0;
	int lastState = 0; // 0:not word, 1:word
	for(int i=0; i<line.length();i++)
	{
		if (IsWord(line[i]) && lastState == 0)
		{
			nCount++;
			lastState = 1;
		}
		else
		{
			lastState = 0;
		}
	}
	return nCount;
}

int main(int argc, char *argv[]) {	    
	cout << "Hello World. " << argc << endl;

	string tmpin;
	if (argc == 1)
	{		
		cin >> tmpin;
	}
	else if (argc == 2)
	{
		string fileName = argv[1];
		ifstream in;
		in.open(fileName, ios_base::in);

		int nLineCount = 0;
		int nWordCount = 0;
		int nByteCount = 0;
		while (!in.eof())
		{
			char buffer[256];
			in.getline(buffer, 255);
			//cout << buffer << endl;
			if (strlen(buffer) != 0)			
				nLineCount++;
			nByteCount += strlen(buffer) + 1;
			nWordCount += countWordLine(buffer);
		}
		
		in.close();
		cout << nLineCount << " "
			<< nWordCount << " "
			<< nByteCount << " "
			<< endl;
	}
	return 0;
}
