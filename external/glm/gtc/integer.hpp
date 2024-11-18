/// @ref gtc_integer
/// @file glm/gtc/integer.hpp
///
/// @see core (dependence)
/// @see gtc_integer (dependence)
///
/// @defgroup gtc_integer GLM_GTC_integer
/// @ingroup gtc
///
/// Include <glm/gtc/integer.hpp> to use the features of this extension.
///
/// @brief Allow to perform bit operations on integer values

#pragma once

// Dependencies
#include "external/glm/detail/setup.hpp"
#include "external/glm/detail/qualifier.hpp"
#include "external/glm/common.hpp"
#include "external/glm/integer.hpp"
#include "external/glm/exponential.hpp"
#include "external/glm/ext/scalar_common.hpp"
#include "external/glm/ext/vector_common.hpp"
#include <limits>

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_GTC_integer extension included")
#endif

namespace glm
{
	/// @addtogroup gtc_integer
	/// @{

	/// Returns the log2 of x for integer values. Usefull to compute mipmap count from the texture size.
	/// @see gtc_integer
	template<typename genIUType>
	GLM_FUNC_DECL genIUType log2(genIUType x);

	/// @}
} //namespace glm

#include "integer.inl"
