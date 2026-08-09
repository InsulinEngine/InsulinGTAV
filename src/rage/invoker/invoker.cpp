#include "invoker.h"
#include <orbis/libkernel.h>

namespace rage::invoker {

	uintptr_t g_eboot_base = 0;

	// The GTA eboot is the process main module; under GoldHEN its module handle
	// is 0. segmentInfo[0] is the executable segment, whose address is the base
	// the RVAs in natives.h are relative to.
	bool resolve_base() {
		OrbisKernelModuleInfo info;
		memset(&info, 0, sizeof(info));
		info.size = sizeof(info);

		if (sceKernelGetModuleInfo(0, &info) != 0)
			return false;
		if (info.segmentCount == 0)
			return false;

		g_eboot_base = (uintptr_t)info.segmentInfo[0].address;
		return g_eboot_base != 0;
	}
}
