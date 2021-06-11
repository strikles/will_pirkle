#include "ModulationMatrix.h"

CModulationMatrix::CModulationMatrix(void)
{
	// --- dynamic allocation of matrix core
	m_ppMatrixCore = new modMatrixRow*[MAX_SOURCES*MAX_DESTINATIONS];
	memset(m_ppMatrixCore, 0, MAX_SOURCES*MAX_DESTINATIONS*sizeof(modMatrixRow*));

	m_nSize = 0;
	clearMatrix(); // fill with NULL
	clearSources();
	clearDestinations();
}

CModulationMatrix::~CModulationMatrix(void)
{
	
}
