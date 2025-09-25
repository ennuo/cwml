#pragma once

#include <fifo.h>
#include <refcount.h>
#include <vector.h>

class CResource;
class CSerialisedResource;

typedef CPriorityQueue<CP<CSerialisedResource> > CSRQueue;
typedef CVector<CP<CResource> > CStrongResourceArray;
typedef CRawVector<CResource*> CWeakResourceArray;
