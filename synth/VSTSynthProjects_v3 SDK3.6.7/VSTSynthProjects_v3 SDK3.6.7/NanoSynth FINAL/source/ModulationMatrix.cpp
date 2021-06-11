#include "ModulationMatrix.h"

CModulationMatrix::CModulationMatrix(void)
{
	// --- setup the core
	m_ppMatrixCore = NULL;
	createModMatrixCore();

	m_nSize = 0;
	clearMatrix(); // fill with NULL
	clearSources();
	clearDestinations();
}

CModulationMatrix::~CModulationMatrix(void)
{
	
}
