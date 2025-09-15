#include <System.h>
#include <DebugLog.h>
#include <Clock.h>
#include <vector.h>
#include <thread.h>
#include <JobManager.h>

bool gWantQuitRequested;
bool gWantQuit;
u64 gSetWantQuitTime;
u64 gCheckWantQuitTime;
u64 gCheckWantQuitOrWantQuitRequestedTime;
u64 gUpdateWantQuitTime;

void AddInitSteps(CInitStep* newsteps)
{
	CInitStep* step = gInitSteps;
	CInitStep* last = NULL;
	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
	}
	last->ChainTo = newsteps;
}

CInitStep* FindInitStep(CInitStep* step, const char* name)
{
	CInitStep* last = NULL;
	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		if (strcmp(name, step->DebugText) == 0)
			return step;
		
		step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
	}

	return NULL;
}


CInitStep* FindInitStep(const char* name)
{
	CInitStep* step = gInitSteps;
	CInitStep* last = NULL;
	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		if (strcmp(name, step->DebugText) == 0)
			return step;
		
		step = ((last = step)->ChainTo == NULL) ? ++step : last->ChainTo;
	}

	return NULL;
}

CInitStep* GetNextInitStep(CInitStep* step)
{
	step = step->ChainTo == NULL ? step + 1 : step->ChainTo;	
	if (step->InitFunc == NULL && step->CloseFunc == NULL && step->PostResourceInitFunc == NULL)
		return NULL;
	return step;
}

void UpdateWantQuit()
{
	if (AmInMainThread())
		gUpdateWantQuitTime = GetClock();

	if (!gWantQuitRequested && gWantQuit)
	{
		gWantQuit = false;
		gSetWantQuitTime = 0;
	}

	if (gWantQuitRequested && !gWantQuit)
	{
		MMLogCh(DC_INIT, "UpdateWantQuit() at %.2f %.2f\n", ToSeconds(gSetWantQuitTime), GetClockSeconds());
		gWantQuit = true;
	}
}

bool WantQuit()
{
	if (AmInMainThread())
		gCheckWantQuitTime = GetClock();
	return gWantQuit;
}

bool WantQuitOrWantQuitRequested()
{
	if (AmInMainThread())
		gCheckWantQuitOrWantQuitRequestedTime = GetClock();
	return gWantQuit || gWantQuitRequested;
}

void SetWantQuit(bool wantQuit)
{
	if (wantQuit && !gWantQuitRequested)
	{
		gSetWantQuitTime = GetClock();
		MMLogCh(DC_INIT, "SetWantQuit() at %.2f %.2f\n", ToSeconds(gSetWantQuitTime), GetClockSeconds());
	}

	gWantQuitRequested = wantQuit;
}

u64 GetQuitTime()
{
	return gSetWantQuitTime;
}

bool InitCWLib(const SCWLibOptions& options, InitStepCallback_Init* start_func)
{
	CInitStep* step = gInitSteps;
	
	bool start = start_func == NULL;

	u64 init_start_time = GetClock();

	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		if (!start && step->InitFunc == start_func)
			start = true;
		
		if (start)
		{
			if (step->InitFunc != NULL)
			{
				MMLogCh(DC_INIT, "Initing %40s... ", step->DebugText);
				u64 start_time = GetClock();
				if (step->Check_This_Bool_Before_Init == NULL || *step->Check_This_Bool_Before_Init)
				{
					step->Inited = step->InitFunc();
					MMLogCh(DC_INIT, "[%s]", step->Inited ? "OK" : "FAIL");
					if (!step->Inited)
					{
						MMLogCh(DC_INIT, "\nAborting init.\n");
						UpdateWantQuit();
						if (step->CloseFunc != NULL)
							step->CloseFunc();

						return false;
					}

				}
				else MMLogCh(DC_INIT, "[Skip]");

				u64 end_time = GetClock();
				MMLogCh(DC_INIT, " %5.3fs\n", ToSeconds(end_time - start_time));
			}
			else step->Inited = true;
		}
		
		step = step->ChainTo == NULL ? step + 1 : step->ChainTo;
	}

	u64 init_end_time = GetClock();
	MMLogCh(DC_INIT, "INIT FINISHED. %f seconds.\n", ToSeconds(init_end_time - init_start_time));

	return true;
}

void CloseCWLib(InitStepCallback_Close* stop_func)
{
	UpdateWantQuit();
	MMLogCh(DC_INIT, "closing...\n");

	CRawVector<CInitStep*> steps;

	CInitStep* step = gInitSteps;
	bool start = stop_func == NULL;

	while (step->InitFunc != NULL || step->CloseFunc != NULL || step->PostResourceInitFunc != NULL)
	{
		if (!start && step->CloseFunc == stop_func)
			start = true;
		
		if (start && step->Inited)
			steps.push_back(step);
		
		step = step->ChainTo == NULL ? step + 1 : step->ChainTo;
	}

	for (CInitStep** it = steps.end() - 1; it >= steps.begin(); --it)
	{
		CInitStep* step = *it;
		MMLogCh(DC_INIT, "Closing %40s... ", step->DebugText);
		u64 start_time = GetClock();
		
		if (step->CloseFunc != NULL)
			step->CloseFunc();
		
		step->Inited = false;

		u64 end_time = GetClock();
		MMLogCh(DC_INIT, " %5.3f s @ %5.3f\n", ToSeconds(end_time - start_time), GetClockSeconds());
	}
}

bool InitJobManager()
{
	gJobManager = new CJobManager(4);
	gHTTPJobManager = new CJobManager(2);
	return true;
}

void CloseJobManager()
{
	delete gJobManager;
	gJobManager = NULL;

	delete gHTTPJobManager;
	gHTTPJobManager = NULL;
}