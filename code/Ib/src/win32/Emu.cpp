#ifdef WIN32

#include <Ib/Emu.h>
using namespace Ib;

bool Ib::IsEmulator() { return false; }
bool Ib::IsUsingRecompiler() { return false; }

#endif // WIN32