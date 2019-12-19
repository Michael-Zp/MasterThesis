//--------------------------------------------------------------------------------------
// File: AMD_ISA.inl
//
// Defines AMD ISA-related internal helpers
//
// Author: Layla Mah
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#define AMD_ISA_AVAILABLE 1

namespace AMD 
{

	extern enum ISA_TARGET
	{
				FIRST_ISA_TARGET = 0,
		Kahn =	FIRST_ISA_TARGET,
		Loki,
		Fudo,
		Yamato,
		Pele,
		ShakaLaka,
		Scarface,
		Boom,
		Wekiva,
		Mario,
		Cypress,
		Cedar,
		Redwood,
		Juniper,
		Ibiza,
		Trinity,
		Tahiti,
		Bonaire,
		Iceland,
				NUM_ISA_TARGETS,
				DEFAULT_ISA_TARGET = Tahiti
	};

	struct ISA_TARGET_INFO
	{
		ISA_TARGET		m_Target;
		const wchar_t*	m_Name;
		const wchar_t*	m_Info;
	};

	static const ISA_TARGET_INFO AmdTargetInfo[] = {
		Kahn,				L"Kahn",		L"(R300 family)",
		Loki,				L"Loki",		L"(R300 family)",
		Fudo,				L"Fudo",		L"(R300 family)",
		Yamato,				L"Yamato",		L"(R300 family)",
		Pele,				L"Pele",		L"(R600 family)",
		ShakaLaka,			L"ShakaLaka",	L"(R600 family)",
		Scarface,			L"Scarface",	L"(R600 family)",
		Boom,				L"Boom",		L"(R600 family)",
		Wekiva,				L"Wekiva",		L"(R700 family)",
		Mario,				L"Mario",		L"(R700 family)",
		Cypress,			L"Cypress",		L"(R800 family)",
		Cedar,				L"Cedar",		L"(R800 family)",
		Redwood,			L"Redwood",		L"(R800 family)",
		Juniper,			L"Juniper",		L"(R800 family)",
		Ibiza,				L"Ibiza",		L"(R900 family)",
		Trinity,			L"Trinity",		L"(R900 family)",
		Tahiti,				L"Tahiti",		L"(R1000 family)",
		Bonaire,			L"Bonaire",		L"(R1100 family)",
		Iceland,			L"Iceland",		L"(R1200 family)",
		NUM_ISA_TARGETS,	NULL,			NULL
	};

}; // namespace AMD

//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
