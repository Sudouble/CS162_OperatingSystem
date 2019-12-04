#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

bool IsWord(char ch)
{
	//return (ch>='0' && ch<='9')||(ch>='A'&&ch<='Z')||(ch>='a'&&ch<='z');
	return !(ch == ' ' || ch == '\t');
}

int countWordLine(string line)
{
	int nCount = 0;
	int lastState = 0; // 0:not word, 1:word
	for(size_t i=0; i<line.length();i++)
	{
		if (IsWord(line[i]) && lastState == 0)
		{
			nCount++;
			lastState = 1;				
		}		
		else if (!IsWord(line[i]) && lastState == 1)
		{
			lastState = 0;
		}
	}
	return nCount;
}

int main(int argc, char *argv[]){
	//cout << "Hello World. " << argc << endl;

	int nLineCount = 0;
	int nWordCount = 0;
	int nByteCount = 0;
	
	string tmpin;
	if (argc == 1)
	{		
		while (cin >> tmpin)
		{		
			nLineCount++;
			nByteCount += tmpin.length() + 1;
			nWordCount += countWordLine(tmpin);
		}
	}
	else if (argc == 2)
	{
		string fileName = argv[1];
		ifstream in;
		in.open(fileName, ios_base::in);		
		while (!in.eof())
		{
			char buffer[256];
			in.getline(buffer, 255);
			//cout << buffer << endl;
			nLineCount++;
			nByteCount += strlen(buffer) + 1;
			nWordCount += countWordLine(buffer);
		}
		in.close();		
		
		--nLineCount;
		--nByteCount;
	}		
			
	cout << nLineCount << " "
		<< nWordCount << " "
		<< nByteCount << " "
		<< endl;
	return 0;
}
