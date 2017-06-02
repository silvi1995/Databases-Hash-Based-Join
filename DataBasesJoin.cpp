#include<bits/stdc++.h>

using namespace std;

int hashFunction(int k, int val, int numberOfAvailablePages)
{
	numberOfAvailablePages -= 1;
	return ((int)floor(val*k*M_PI))%numberOfAvailablePages;
}


void InMemoryJoin(vector<int> data1,vector<int> data2,FILE *fout,FILE *fjoin)
{
	/*sort(data1.begin(),data1.end());
	sort(data2.begin(),data2.end());
	int i,j,size1,size2,count;
	size1 = data1.size();
	size2 = data2.size();
 	i = j = count = 0;
	fprintf(fout,"\nMatching pairs are : \n");
	while(i<size1 && j<size2)
	{
		if(data1[i] == data2[j])
		{
			fprintf(fout,"\n...... %d\n",data1[i]);
			fprintf(fjoin,"%d\n",data1[i]);
			i++;
			j++;
			count++;
		}
		else if(data1[i] > data2[j])
		{
			j++;
		}
		else if(data1[i] < data2[j])
		{
			i++;
		}
	}
	if(count == 0)
        fprintf(fout,"\nNo matching tuple . No further processing required\n");
	fprintf(fout,"\n\n");*/

	int size1 = data1.size();
	int size2 = data2.size();
	int count = 0;
	for(int i=0;i<size1;i++)
    {
        for(int j=0;j<size2;j++)
        {
            if(data1[i] == data2[j])
            {
                count++;
                fprintf(fout,"\n...... %d\n",data1[i]);
                fprintf(fjoin,"%d\n",data1[i]);
            }
        }
    }
    if(count == 0)
        fprintf(fout,"\nNo matching tuple . No further processing required\n");
	fprintf(fout,"\n\n");
}


vector<int> FileRead(FILE *fp)
{
    vector<int> data;
    if(fp == NULL)
    {
        return data;
    }
    else
    {
        char str[100];
        int dataVal;
        while(fgets(str,100,fp))
        {
            stringstream ss(str);
            ss >> dataVal;
            data.push_back(dataVal);
        }
        return data;
    }
}


bool IsInMemoryJoinPossible(int page1,int page2,int numberOfAvailablePages)
{
	if(page1 == 0 || page2 == 0)
        return false;
	if((page1 + page2) < numberOfAvailablePages)
		return true;
	else
		return false;
}

void CreateDumpFiles(vector<int> data1,vector<int> data2,int numberOfAvailablePages,int round,int maximumNumberOfHashingRounds,
                     FILE *fout,FILE *fjoin,int recordPerPage1,int recordPerPage2,int *numIO,string fName1,string fName2)
{
    int numRecord1 = data1.size();
    int numRecord2 = data2.size();

    if(numRecord1 == 0 || numRecord2 == 0)
        return;

    int page1 = ceil((double)numRecord1/recordPerPage1);
    int page2 = ceil((double)numRecord2/recordPerPage2);

    *numIO += page1 + page2;

    fprintf(fout,"\nSize of %s : %d pages\n",fName1.c_str(),page1);
    fprintf(fout,"\nSize of %s : %d pages\n",fName2.c_str(),page2);
    fprintf(fout,"\nTotal number of available pages : %d pages\n",numberOfAvailablePages);
    fprintf(fout,"\nNumber of buckets in hash table : %d\n\n\n",numberOfAvailablePages-1);

    if(IsInMemoryJoinPossible(page1,page2,numberOfAvailablePages))
    {
        fprintf(fout,"\nPerforming in memory join\n");
        InMemoryJoin(data1,data2,fout,fjoin);
        return;
    }

    if(round > maximumNumberOfHashingRounds)
    {
        fprintf(fout,"\nHash join not possible with given number of hash rounds\n");
        return ;
    }

    fprintf(fout,"\nHashing Round : %d\n",round);

    FILE *DumpFiles1[numberOfAvailablePages-1],*DumpFiles2[numberOfAvailablePages-1];

    vector<string> fileName1,fileName2;
    char str[20];string fileNametxt;
    int sizeHash1[numberOfAvailablePages-1],sizeHash2[numberOfAvailablePages-1];

    for(int i=0; i<numberOfAvailablePages-1;i++)
    {
        sprintf(str,"%d",i);
        string s(str);

        fileName1.push_back(fName1 + ".bucket" + s);
        fileNametxt = fileName1[i] + ".txt";
        DumpFiles1[i] = fopen(fileNametxt.c_str(),"w");

        fileName2.push_back(fName2 + ".bucket" + s);
        fileNametxt = fileName2[i] + ".txt";
        DumpFiles2[i] = fopen(fileNametxt.c_str(),"w");

        sizeHash1[i] = sizeHash2[i] = 0;
    }

    int i = 0;int hashVal;int sizeTemp;
    vector<int> hashTable1[numberOfAvailablePages-1],hashTable2[numberOfAvailablePages-1];
    fprintf(fout,"\nReading %s :\n",fName1.c_str());

    while(i < numRecord1)
    {
        hashVal = hashFunction(round,data1[i],numberOfAvailablePages);
        fprintf(DumpFiles1[hashVal],"%d\n",data1[i]);
        hashTable1[hashVal].push_back(data1[i]);
        fprintf(fout,"\nTuple %d : %d Mapped to bucket %d\n",i,data1[i],hashVal);
        if(hashTable1[hashVal].size() == recordPerPage1)
        {
            fprintf(fout,"\nPage for %s.bucket%d full.Flushed to secondary storage.\n",fName1.c_str(),hashVal);
            hashTable1[hashVal].clear();
        }
        i ++;
        sizeHash1[hashVal] ++;
    }

    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        if(hashTable1[i].size() != recordPerPage1)
            fprintf(fout,"\nPage for %s.bucket%d flushed to secondary storage.\n",fName1.c_str(),i);
    }

    fprintf(fout,"\nDone with %s\n",fName1.c_str());
    fprintf(fout,"\nCreated following files :\n");
    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        sizeTemp = ceil((double)sizeHash1[i]/recordPerPage1);
        fprintf(fout,"\n%s.bucket%d : %d pages\n",fName1.c_str(),i,sizeTemp);
    }
    fprintf(fout,"\n\n");

    i = 0;
    fprintf(fout,"\nReading %s :\n",fName2.c_str());

    while(i < numRecord2)
    {
        hashVal = hashFunction(round,data2[i],numberOfAvailablePages);
        fprintf(DumpFiles2[hashVal],"%d\n",data2[i]);
        hashTable2[hashVal].push_back(data2[i]);
        fprintf(fout,"\nTuple %d : %d Mapped to bucket %d\n",i,data2[i],hashVal);
        if(hashTable2[hashVal].size() == recordPerPage2)
        {
            fprintf(fout,"\nPage for %s.bucket%d full.Flushed to secondary storage.\n",fName2.c_str(),hashVal);
            hashTable1[hashVal].clear();
        }
        i ++;
        sizeHash2[hashVal] ++;
    }

    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        if(hashTable2[i].size() != recordPerPage2)
            fprintf(fout,"\nPage for %s.bucket%d flushed to secondary storage.\n",fName2.c_str(),i);
    }

    fprintf(fout,"\nDone with %s\n",fName2.c_str());
    fprintf(fout,"\nCreated following files :\n");

    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        sizeTemp = ceil((double)sizeHash2[i]/recordPerPage2);
        fprintf(fout,"\n%s.bucket%d : %d pages\n",fName1.c_str(),i,sizeTemp);
    }
    fprintf(fout,"\n\n");

    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        *numIO += ceil((double)sizeHash1[i]/recordPerPage1);
        *numIO += ceil((double)sizeHash2[i]/recordPerPage2);

        fclose(DumpFiles1[i]);
        fclose(DumpFiles2[i]);

        fileNametxt = fileName1[i] + ".txt";
        DumpFiles1[i] = fopen(fileNametxt.c_str() ,"r") ;

        fileNametxt = fileName2[i] + ".txt";
        DumpFiles2[i] = fopen(fileNametxt.c_str() ,"r") ;
    }

    data1.clear();
    data2.clear();

    for(int i=0;i<numberOfAvailablePages-1;i++)
    {
        data1 = FileRead(DumpFiles1[i]);
        data2 = FileRead(DumpFiles2[i]);

        CreateDumpFiles(data1,data2,numberOfAvailablePages,round+1,maximumNumberOfHashingRounds,fout,fjoin,
                        recordPerPage1,recordPerPage2,numIO,fileName1[i],fileName2[i]);
    }

}


int main()
{
    FILE *fout = fopen("logFile.txt","w");
    FILE *fjoin = fopen("JoinOutput.txt","w");
    string relationFile1 , relationFile2;
    int recordSize1 , recordSize2 , pageSize , numberOfAvailablePages , maximumNumberOfHashingRounds ;
    int recordPerPage1 , recordPerPage2 , numIO;

    cout << "\nEnter relationFile1 ( example : data.txt)\n";
    cin >> relationFile1;
    cout << "\nEnter relationFile2 ( example : data.txt)\n";
    cin >> relationFile2;
    cout << "\nEnter recordSize for relationFile1\n";
    cin >> recordSize1;
    cout << "\nEnter recordSize for relationFile1\n";
    cin >> recordSize2;
    cout << "\nEnter pageSize\n";
    cin >> pageSize;
    cout << "\nEnter numberOfAvailablePages\n";
    cin >> numberOfAvailablePages;
    cout << "\nEnter maximumNumberOfHashingRounds\n";
    cin >> maximumNumberOfHashingRounds;

    recordPerPage1 = pageSize/recordSize1;
    recordPerPage2 = pageSize/recordSize2;

    FILE *fp1 = fopen(relationFile1.c_str(),"r");
    FILE *fp2 = fopen(relationFile2.c_str(),"r");

    vector<int> data1(FileRead(fp1));
    vector<int> data2(FileRead(fp2));

    numIO = 0;
    CreateDumpFiles(data1,data2,numberOfAvailablePages,1,maximumNumberOfHashingRounds,fout,fjoin,
                    recordPerPage1,recordPerPage2,&numIO,"Relation1","Relation2");

    fprintf(fout,"\nTOTAL NUMBER OF I/O : %d\n",numIO);
}
