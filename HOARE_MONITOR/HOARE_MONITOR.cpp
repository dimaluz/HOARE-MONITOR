// HOARE_MONITOR.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <cmath>
#include <windows.h>
#include <process.h>
#include <mutex>
using namespace std;

HANDLE CanWrite;
HANDLE CanRead;
HANDLE MUTEX;

LONG SHARED_RESOURCE=0;
LONG MAX_VAL=30;
LONG Stop=0;

LONG readers=0;
LONG writeLock=0;

LONG queueReaders=0;
LONG queueWriters=0;

void BeginWrite(int);
void EndWrite();
unsigned __stdcall Write(PVOID);

void BeginRead(int);
void EndRead();
unsigned __stdcall Read(PVOID);

LONG Show(LONG&);

int main(void){
	int NumReaders=5, NumWriters=2;
	int Qty=NumReaders+NumWriters;
	CanWrite=CreateEvent(NULL, FALSE, TRUE, NULL);
	CanRead=CreateEvent(NULL, TRUE, FALSE, NULL);
	MUTEX=CreateMutex(NULL, FALSE, NULL);
	for(int i=0; i<Qty; i++)
		if(i<NumReaders)
			_beginthreadex(NULL, 0, Read, (PVOID*)i, 0, NULL);
		else
			_beginthreadex(NULL, 0, Write, (PVOID*)i, 0, NULL);
	getchar();
	return 0;
}

unsigned __stdcall Write(PVOID Id){
	int id=(int)Id;
	while(!Stop){
		BeginWrite(id);
		if(SHARED_RESOURCE<MAX_VAL){
			Sleep(rand()%20);
			InterlockedExchangeAdd(&SHARED_RESOURCE, 1);
			cout<<"\t<WRITER #"<<id<<" WRITTEN VALUE: "<<Show(SHARED_RESOURCE)<<">"<<endl;
		}
		else{
			WaitForSingleObject(MUTEX, INFINITE);
			Stop=1;
			ReleaseMutex(MUTEX);
		}
		EndWrite();
	}
	return 0;
}
void BeginWrite(int id){
	InterlockedExchangeAdd(&queueWriters, 1);
	if(readers>0 || writeLock==1){
		ResetEvent(CanWrite);
	}
	WaitForSingleObject(CanWrite, INFINITE);
	InterlockedExchangeAdd(&writeLock, 1);
	InterlockedExchangeAdd(&queueWriters, -1);
}
void EndWrite(){
	InterlockedExchangeAdd(&writeLock, -1);
	if(queueReaders>0)
		SetEvent(CanRead);
	else
		SetEvent(CanWrite);
}

unsigned __stdcall Read(PVOID Id){
	int id=(int)Id;
	while(!Stop){
		BeginRead(id);
		Sleep(rand()%20);
		cout<<"<Reader #"<<id<<" Current Value: "<<Show(SHARED_RESOURCE)<<">"<<endl;
		EndRead();
	}
	return 0;
}
void BeginRead(int id){
	InterlockedExchangeAdd(&queueReaders, 1);
	if(writeLock==1 || queueWriters>0){
		ResetEvent(CanRead);
	}
	WaitForSingleObject(CanRead, INFINITE);
	InterlockedExchangeAdd(&queueReaders, -1);
	InterlockedExchangeAdd(&readers, 1);
	SetEvent(CanRead);
}
void EndRead(){
	InterlockedExchangeAdd(&readers, -1);
	if(readers==0){
		SetEvent(CanWrite);
	}
}

LONG Show(LONG& resource){
	return resource;
}