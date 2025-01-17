/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * String built-in description
 */

#include "ecma-builtin-helpers-macro-defines.inc.h"

#if JERRY_BUILTIN_STRING

/* Number properties:
 *  (property name, number value, writable, enumerable, configurable) */

NUMBER_VALUE(LIT_MAGIC_STRING_LENGTH, 1, ECMA_PROPERTY_FLAG_DEFAULT_LENGTH)

/* Object properties:
 *  (property name, object pointer getter) */

/* ECMA-262 v5, 15.7.3.1 */
OBJECT_VALUE(LIT_MAGIC_STRING_PROTOTYPE, ECMA_BUILTIN_ID_STRING_PROTOTYPE, ECMA_PROPERTY_FIXED)

#if JERRY_ESNEXT
STRING_VALUE(LIT_MAGIC_STRING_NAME, LIT_MAGIC_STRING_STRING_UL, ECMA_PROPERTY_FLAG_CONFIGURABLE)
#endif /* JERRY_ESNEXT */

/* Routine properties:
 *  (property name, C routine name, arguments number or NON_FIXED, value of the routine's length property) */
ROUTINE(LIT_MAGIC_STRING_FROM_CHAR_CODE_UL, ECMA_BUILTIN_STRING_OBJECT_FROM_CHAR_CODE, NON_FIXED, 1)

#if JERRY_ESNEXT
ROUTINE(LIT_MAGIC_STRING_FROM_CODE_POINT_UL, ECMA_BUILTIN_STRING_OBJECT_FROM_CODE_POINT, NON_FIXED, 1)
ROUTINE(LIT_MAGIC_STRING_RAW, ECMA_BUILTIN_STRING_OBJECT_RAW, NON_FIXED, 1)
#endif /* JERRY_ESNEXT */

#endif /* JERRY_BUILTIN_STRING */

#include "ecma-builtin-helpers-macro-undefs.inc.h"
