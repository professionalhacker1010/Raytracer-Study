#pragma once
#include <windows.h>
#include "Constants.h"

// Nils's jobmanager
class Job
{
public:
	virtual void Main() = 0;
protected:
	friend class JobThread;
	void RunCodeWrapper();
};
class JobThread
{
public:
	void CreateAndStartThread(uint threadId);
	void Go();
	void BackgroundTask();
	HANDLE m_GoSignal, m_ThreadHandle;
	int m_ThreadID;
};
class JobManager	// singleton class!
{
protected:
	JobManager(uint numThreads);
public:
	~JobManager();
	static void CreateJobManager(uint numThreads);
	static JobManager* GetJobManager();
	static void GetProcessorCount(uint& cores, uint& logical);
	void AddJob2(Job* a_Job);
	unsigned int GetNumThreads() { return m_NumThreads; }
	void RunJobs();
	void ThreadDone(uint n);
	int MaxConcurrent() { return m_NumThreads; }
protected:
	friend class JobThread;
	Job* GetNextJob();
	static JobManager* m_JobManager;
	Job* m_JobList[256];
	CRITICAL_SECTION m_CS;
	HANDLE m_ThreadDone[64];
	unsigned int m_NumThreads, m_JobCount;
	JobThread* m_JobThreadList;
};


