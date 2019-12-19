//--------------------------------------------------------------------------------------
// File: StringTokenizer.h
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

int StringTokenizer(const std::string& input, const std::string& delimiter, 
				std::vector<std::string>& results, bool includeEmpties);


#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
