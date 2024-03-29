/**
 * Taken from: https://github.com/18sg/uSHET
 * and expanded for Hemlock Revisited by Matthew Marshall.
 *
 * uSHET Library
 * =============
 *
 * Copyright (c) 2014 Thomas Nixon, Jonathan Heathcote
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * jsmn Library
 * ============
 *
 * Copyright (c) 2010 Serge A. Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * This header file contains a library of advanced C Pre-Processor (CPP) macros
 * which implement various useful functions, such as iteration, in the
 * pre-processor.
 *
 * Though the file name (quite validly) labels this as magic, there should be
 * enough documentation in the comments for a reader only casually familiar
 * with the CPP to be able to understand how everything works.
 *
 * The majority of the magic tricks used in this file are based on those
 * described by pfultz2 in his "Cloak" library:
 *
 *    https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
 *
 * Major differences are a greater level of detailed explanation in this
 * implementation and also a refusal to include any macros which require a O(N)
 * macro definitions to handle O(N) arguments (with the exception of DEFERn).
 */

#ifndef hemlock_preprocessor_h
#define hemlock_preprocessor_h

/**
 * Force the pre-processor to expand the macro a large number of times. Usage:
 *
 *   EVAL(expression)
 *
 * This is useful when you have a macro which evaluates to a valid macro
 * expression which is not subsequently expanded in the same pass. A contrived,
 * but easy to understand, example of such a macro follows. Note that though
 * this example is contrived, this behaviour is abused to implement bounded
 * recursion in macros such as FOR.
 *
 *   #define A(x) x+1
 *   #define EMPTY
 *   #define NOT_QUITE_RIGHT(x) A EMPTY (x)
 *   NOT_QUITE_RIGHT(999)
 *
 * Here's what happens inside the C preprocessor:
 *
 * 1. It sees a macro "NOT_QUITE_RIGHT" and performs a single macro expansion
 *    pass on its arguments. Since the argument is "999" and this isn't a macro,
 *    this is a boring step resulting in no change.
 * 2. The NOT_QUITE_RIGHT macro is substituted for its definition giving "A
 *    EMPTY() (x)".
 * 3. The expander moves from left-to-right trying to expand the macro:
 *    The first token, A, cannot be expanded since there are no brackets
 *    immediately following it. The second token EMPTY(), however, can be
 *    expanded (recursively in this manner) and is replaced with "".
 * 4. Expansion continues from the start of the substituted test (which in this
 *    case is just empty), and sees "(999)" but since no macro name is present,
 *    nothing is done. This results in a final expansion of "A (999)".
 *
 * Unfortunately, this doesn't quite meet expectations since you may expect that
 * "A (999)" would have been expanded into "999+1". Unfortunately this requires
 * a second expansion pass but luckily we can force the macro processor to make
 * more passes by abusing the first step of macro expansion: the preprocessor
 * expands arguments in their own pass. If we define a macro which does nothing
 * except produce its arguments e.g.:
 *
 *   #define PASS_THROUGH(...) __VA_ARGS__
 *
 * We can now do "PASS_THROUGH(NOT_QUITE_RIGHT(999))" causing "NOT_QUITE_RIGHT" to be
 * expanded to "A (999)", as described above, when the arguments are expanded.
 * Now when the body of PASS_THROUGH is expanded, "A (999)" gets expanded to
 * "999+1".
 *
 * The EVAL defined below is essentially equivalent to a large nesting of
 * "PASS_THROUGH(PASS_THROUGH(PASS_THROUGH(..." which results in the
 * preprocessor making a large number of expansion passes over the given
 * expression.
 */
#define EVAL2(...)     _EVAL4096(__VA_ARGS__)
#define _EVAL4096(...) _EVAL2048(_EVAL2048(__VA_ARGS__))
#define _EVAL2048(...) _EVAL1024(_EVAL1024(__VA_ARGS__))
#define EVAL(...)      _EVAL1024(__VA_ARGS__)
#define _EVAL1024(...) _EVAL512(_EVAL512(__VA_ARGS__))
#define _EVAL512(...)  _EVAL256(_EVAL256(__VA_ARGS__))
#define _EVAL256(...)  _EVAL128(_EVAL128(__VA_ARGS__))
#define _EVAL128(...)  _EVAL64(_EVAL64(__VA_ARGS__))
#define _EVAL64(...)   _EVAL32(_EVAL32(__VA_ARGS__))
#define _EVAL32(...)   _EVAL16(_EVAL16(__VA_ARGS__))
#define _EVAL16(...)   _EVAL8(_EVAL8(__VA_ARGS__))
#define _EVAL8(...)    _EVAL4(_EVAL4(__VA_ARGS__))
#define _EVAL4(...)    _EVAL2(_EVAL2(__VA_ARGS__))
#define _EVAL2(...)    _EVAL1(_EVAL1(__VA_ARGS__))
#define _EVAL1(...)    __VA_ARGS__

/**
 * Macros which expand to common values
 */
#define PASS(...) __VA_ARGS__
#define EMPTY()
#define COMMA()       ,
#define SEMICOLON()   ;
#define OPEN_BRACE()  {
#define CLOSE_BRACE() }
#define PLUS()        +
#define ZERO()        0
#define ONE()         1

/**
 * Causes a function-style macro to require an additional pass to be expanded.
 *
 * This is useful, for example, when trying to implement recursion since the
 * recursive step must not be expanded in a single pass as the pre-processor
 * will catch it and prevent it.
 *
 * Usage:
 *
 *   DEFER1(IN_NEXT_PASS)(args, to, the, macro)
 *
 * How it works:
 *
 * 1. When DEFER1 is expanded, first its arguments are expanded which are
 *    simply IN_NEXT_PASS. Since this is a function-style macro and it has no
 *    arguments, nothing will happen.
 * 2. The body of DEFER1 will now be expanded resulting in EMPTY() being
 *    deleted. This results in "IN_NEXT_PASS (args, to, the macro)". Note that
 *    since the macro expander has already passed IN_NEXT_PASS by the time it
 *    expands EMPTY() and so it won't spot that the brackets which remain can be
 *    applied to IN_NEXT_PASS.
 * 3. At this point the macro expansion completes. If one more pass is made,
 *    IN_NEXT_PASS(args, to, the, macro) will be expanded as desired.
 */
#define DEFER1(id) id EMPTY()

/**
 * As with DEFER1 except here n additional passes are required for DEFERn.
 *
 * The mechanism is analogous.
 *
 * Note that there doesn't appear to be a way of combining DEFERn macros in
 * order to achieve exponentially increasing defers e.g. as is done by EVAL.
 */
#define DEFER2(id) id EMPTY EMPTY()()
#define DEFER3(id) id EMPTY EMPTY EMPTY()()()
#define DEFER4(id) id EMPTY EMPTY EMPTY EMPTY()()()()
#define DEFER5(id) id EMPTY EMPTY EMPTY EMPTY EMPTY()()()()()
#define DEFER6(id) id EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY()()()()()()
#define DEFER7(id) id EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY()()()()()()()
#define DEFER8(id) id EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY EMPTY()()()()()()()()

/**
 * Indirection around the standard ## concatenation operator. This simply
 * ensures that the arguments are expanded (once) before concatenation.
 */
#define CAT(a, ...)     a##__VA_ARGS__
#define CAT3(a, b, ...) a##b##__VA_ARGS__

/**
 * Get the first argument and ignore the rest.
 */
#define FIRST(a, ...) a

/**
 * Get the second argument and ignore the rest.
 */
#define SECOND(a, b, ...) b

/**
 * Expects a single input (not containing commas). Returns 1 if the input is
 * PROBE() and otherwise returns 0.
 *
 * This can be useful as the basis of a NOT function.
 *
 * This macro abuses the fact that PROBE() contains a comma while other valid
 * inputs must not.
 */
#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE()       ~, 1

/**
 * Logical negation. 0 is defined as false and everything else as true.
 *
 * When 0, _NOT_0 will be found which evaluates to the PROBE. When 1 (or any other
 * value) is given, an appropriately named macro won't be found and the
 * concatenated string will be produced. IS_PROBE then simply checks to see if
 * the PROBE was returned, cleanly converting the argument into a 1 or 0.
 */
#define NOT(x, ...) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0      PROBE()

/**
 * Macro version of C's famous "cast to bool" operator (i.e. !!) which takes
 * anything and casts it to 0 if it is 0 and 1 otherwise.
 */
#define BOOL(x) NOT(NOT(x))

/**
 * Logical OR. Simply performs a lookup.
 */
#define OR(a, b) CAT3(_OR_, a, b)
#define _OR_00   0
#define _OR_01   1
#define _OR_10   1
#define _OR_11   1

/**
 * Logical AND. Simply performs a lookup.
 */
#define AND(a, b) CAT3(_AND_, a, b)
#define _AND_00   0
#define _AND_01   0
#define _AND_10   0
#define _AND_11   1

/**
 * Macro if statement. Usage:
 *
 *   IF(c)(expansion when true)
 *
 * Here's how:
 *
 * 1. The preprocessor expands the arguments to _IF casting the condition to '0'
 *    or '1'.
 * 2. The casted condition is concatencated with _IF_ giving _IF_0 or _IF_1.
 * 3. The _IF_0 and _IF_1 macros either returns the argument or doesn't (e.g.
 *    they implement the "choice selection" part of the macro).
 * 4. Note that the "true" clause is in the extra set of brackets; thus these
 *    become the arguments to _IF_0 or _IF_1 and thus a selection is made!
 */
#define IF(c)  _IF(BOOL(c))
#define _IF(c) CAT(_IF_, c)
#define _IF_0(...)
#define _IF_1(...) __VA_ARGS__

/**
 * Macro if/else statement. Usage:
 *
 *   IF_ELSE(c)( \
 *     expansion when true, \
 *     expansion when false \
 *   )
 *
 * The mechanism is analogous to IF.
 */
#define IF_ELSE(c)       _IF_ELSE(BOOL(c))
#define _IF_ELSE(c)      CAT(_IF_ELSE_, c)
#define _IF_ELSE_0(t, f) f
#define _IF_ELSE_1(t, f) t

/**
 * Macro which checks if it has any arguments. Returns '0' if there are no
 * arguments, '1' otherwise.
 *
 * Limitation: HAS_ARGS(,1,2,3) returns 0 -- this check essentially only checks
 * that the first argument exists.
 *
 * This macro works as follows:
 *
 * 1. _END_OF_ARGUMENTS_ is concatenated with the first argument.
 * 2. If the first argument is not present then only "_END_OF_ARGUMENTS_" will
 *    remain, otherwise "_END_OF_ARGUMENTS something_here" will remain. This
 *    remaining argument can start with parentheses.
 * 3. In the former case, the _END_OF_ARGUMENTS_(0) macro expands to a
 *    0 when it is expanded. In the latter, a non-zero result remains. If the
 *    first argument started with parentheses these will mostly not contain
 *    only a single 0, but e.g a C cast or some arithmetic operation that will
 *    cause the BOOL in _END_OF_ARGUMENTS_ to be one.
 * 4. BOOL is used to force non-zero results into 1 giving the clean 0 or 1
 *    output required.
 */
#define HAS_ARGS(...)           BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)(0))
#define _END_OF_ARGUMENTS_(...) BOOL(FIRST(__VA_ARGS__))

/**
 * Macro map/list comprehension. Usage:
 *
 *   MAP(op, sep, ...)
 *
 * Produces a 'sep()'-separated list of the result of op(arg) for each arg.
 *
 * Example Usage:
 *
 *   #define MAKE_HAPPY(x) happy_##x
 *   #define COMMA() ,
 *   MAP(MAKE_HAPPY, COMMA, 1,2,3)
 *
 * Which expands to:
 *
 *    happy_1 , happy_2 , happy_3
 *
 * How it works:
 *
 * 1. The MAP macro simply maps the inner MAP_INNER function in an EVAL which
 *    forces it to be expanded a large number of times, thus enabling many steps
 *    of iteration (see step 6).
 * 2. The MAP_INNER macro is substituted for its body.
 * 3. In the body, op(cur_val) is substituted giving the value for this
 *    iteration.
 * 4. The IF macro expands according to whether further iterations are required.
 *    This expansion either produces _IF_0 or _IF_1.
 * 5. Since the IF is followed by a set of brackets containing the "if true"
 *    clause, these become the argument to _IF_0 or _IF_1. At this point, the
 *    macro in the brackets will be expanded giving the separator followed by
 *    _MAP_INNER EMPTY()()(op, sep, __VA_ARGS__).
 * 5. If the IF was not taken, the above will simply be discarded and everything
 *    stops. If the IF is taken, The expression is then processed a second time
 *    yielding "_MAP_INNER()(op, sep, __VA_ARGS__)". Note that this call looks
 *    very similar to the  essentially the same as the original call except the
 *    first argument has been dropped.
 * 6. At this point expansion of MAP_INNER will terminate. However, since we can
 *    force more rounds of expansion using EVAL1. In the argument-expansion pass
 *    of the EVAL1, _MAP_INNER() is expanded to MAP_INNER which is then expanded
 *    using the arguments which follow it as in step 2-5. This is followed by a
 *    second expansion pass as the substitution of EVAL1() is expanded executing
 *    2-5 a second time. This results in up to two iterations occurring. Using
 *    many nested EVAL1 macros, i.e. the very-deeply-nested EVAL macro, will in
 *    this manner produce further iterations, hence the outer MAP macro doing
 *    this for us.
 *
 * Important tricks used:
 *
 * * If we directly produce "MAP_INNER" in an expansion of MAP_INNER, a special
 *   case in the preprocessor will prevent it being expanded in the future, even
 *   if we EVAL.  As a result, the MAP_INNER macro carefully only expands to
 *   something containing "_MAP_INNER()" which requires a further expansion step
 *   to invoke MAP_INNER and thus implementing the recursion.
 * * To prevent _MAP_INNER being expanded within the macro we must first defer its
 *   expansion during its initial pass as an argument to _IF_0 or _IF_1. We must
 *   then defer its expansion a second time as part of the body of the _IF_0. As
 *   a result hence the DEFER2.
 * * _MAP_INNER seemingly gets away with producing itself because it actually only
 *   produces MAP_INNER. It just happens that when _MAP_INNER() is expanded in
 *   this case it is followed by some arguments which get consumed by MAP_INNER
 *   and produce a _MAP_INNER.  As such, the macro expander never marks
 *   _MAP_INNER as expanding to itself and thus it will still be expanded in
 *   future productions of itself.
 */

#define MAP(op, sep, ...)                                                              \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(MAP_INNER(op, sep, __VA_ARGS__)))
#define MAP_INNER(op, sep, cur_val, ...)                                               \
  op(cur_val                                                                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_INNER)()(op, sep, ##__VA_ARGS__))
#define _MAP_INNER() MAP_INNER

/**
 * The same as MAP, except any first-level MAP macro may be safely nested inside this.
 */
#define MAP_2(op, sep, ...)                                                            \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(MAP_2_INNER(op, sep, __VA_ARGS__)))
#define MAP_2_INNER(op, sep, cur_val, ...)                                             \
  op(cur_val                                                                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_2_INNER)()(op, sep, ##__VA_ARGS__))
#define _MAP_2_INNER() MAP_2_INNER

/**
 * Like MAP but binds extra state to each call of op. The macro
 * being called as op(binding, arg).
 */

#define BIND_MAP(op, binding, sep, ...)                                                \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(BIND_MAP_INNER(op, binding, sep, __VA_ARGS__)))
#define BIND_MAP_INNER(op, binding, sep, cur_val, ...)                                 \
  op(binding, cur_val) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_INNER          \
  )()(op, binding, sep, ##__VA_ARGS__))
#define _BIND_MAP_INNER() BIND_MAP_INNER

/**
 * The same as BIND_MAP, except first-level MAP macro may be safely nested inside this.
 */
#define BIND_MAP_2(op, binding, sep, ...)                                              \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(BIND_MAP_2_INNER(op, binding, sep, __VA_ARGS__)))
#define BIND_MAP_2_INNER(op, binding, sep, cur_val, ...)                               \
  op(binding, cur_val) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_2_INNER        \
  )()(op, binding, sep, ##__VA_ARGS__))
#define _BIND_MAP_2_INNER() BIND_MAP_2_INNER

/**
 * Like MAP but binds extra state to each call of op. The macro
 * being called as op(binding1, binding2, arg).
 */

#define BIND2_MAP(op, binding1, binding2, sep, ...)                                    \
  IF(HAS_ARGS(__VA_ARGS__)) (                                                          \
    EVAL(BIND2_MAP_INNER(op, binding1, binding2, sep, __VA_ARGS__))                    \
  )
#define BIND2_MAP_INNER(op, binding1, binding2, sep, cur_val, ...)                     \
  op(binding1, binding2, cur_val) IF(HAS_ARGS(__VA_ARGS__))(                           \
    sep() DEFER2(_BIND2_MAP_INNER)()(op, binding1. binding2, sep, ##__VA_ARGS__)       \
  )
#define _BIND2_MAP_INNER() BIND2_MAP_INNER

/**
 * The same as BIND2_MAP, except first-level MAP macro may be safely nested inside this.
 */
#define BIND2_MAP_2(op, binding1, binding2, sep, ...)                                  \
  IF(HAS_ARGS(__VA_ARGS__)) (                                                          \
    EVAL2(BIND2_MAP_2_INNER(op, binding1, binding2, sep, __VA_ARGS__))                 \
  )
#define BIND2_MAP_2_INNER(op, binding1, binding2, sep, cur_val, ...)                   \
  op(binding1, binding2, cur_val) IF(HAS_ARGS(__VA_ARGS__))(                           \
    sep() DEFER2(_BIND2_MAP_2_INNER)()(op, binding1, binding2, sep, ##__VA_ARGS__)     \
  )
#define _BIND2_MAP_2_INNER() BIND2_MAP_2_INNER

/**
 * This is a variant of the MAP macro which also includes as an argument to the
 * operation a valid C variable name which is different for each iteration.
 *
 * Usage:
 *   MAP_WITH_ID(op, sep, ...)
 *
 * Where op is a macro op(val, id) which takes a list value and an ID. This ID
 * will simply be a unary number using the digit "I", that is, I, II, III, IIII,
 * and so on.
 *
 * Example:
 *
 *   #define MAKE_STATIC_VAR(type, name) static type name;
 *   MAP_WITH_ID(MAKE_STATIC_VAR, EMPTY, int, int, int, bool, char)
 *
 * Which expands to:
 *
 *   static int I; static int II; static int III; static bool IIII; static char IIIII;
 *
 * The mechanism is analogous to the MAP macro.
 */
#define MAP_WITH_ID(op, sep, ...)                                                      \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(MAP_WITH_ID_INNER(op, sep, I, ##__VA_ARGS__)))
#define MAP_WITH_ID_INNER(op, sep, id, cur_val, ...)                                   \
  op(cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_WITH_ID_INNER            \
  )()(op, sep, CAT(id, I), ##__VA_ARGS__))
#define _MAP_WITH_ID_INNER() MAP_WITH_ID_INNER

/**
 * The same as MAP_WITH_ID, except first-level MAP macro may be safely nested inside
 * this.
 */
#define MAP_WITH_ID_2(op, sep, ...)                                                    \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(MAP_WITH_ID_2_INNER(op, sep, I, ##__VA_ARGS__)))
#define MAP_WITH_ID_2_INNER(op, sep, id, cur_val, ...)                                 \
  op(cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_WITH_ID_2_INNER          \
  )()(op, sep, CAT(id, I), ##__VA_ARGS__))
#define _MAP_WITH_ID_2_INNER() MAP_WITH_ID_2_INNER

/**
 * Like MAP_WITH_ID but binds extra state to each call of op. The macro
 * being called as op(binding, val, id).
 */
#define BIND_MAP_WITH_ID(op, binding, sep, ...)                                        \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND_MAP_WITH_ID_INNER(op, binding, sep, I, ##__VA_ARGS__)))
#define BIND_MAP_WITH_ID_INNER(op, binding, sep, id, cur_val, ...)                     \
  op(binding, cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep(                              \
  ) DEFER2(_BIND_MAP_WITH_ID_INNER)()(op, binding, sep, CAT(id, I), ##__VA_ARGS__))
#define _BIND_MAP_WITH_ID_INNER() BIND_MAP_WITH_ID_INNER

/**
 * The same as BIND_MAP_WITH_ID, except first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND_MAP_WITH_ID_2(op, binding, sep, ...)                                      \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(BIND_MAP_WITH_ID_2_INNER(op, binding, sep, I, ##__VA_ARGS__)))
#define BIND_MAP_WITH_ID_2_INNER(op, binding, sep, id, cur_val, ...)                   \
  op(binding, cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep(                              \
  ) DEFER2(_BIND_MAP_WITH_ID_2_INNER)()(op, binding, sep, CAT(id, I), ##__VA_ARGS__))
#define _BIND_MAP_WITH_ID_2_INNER() BIND_MAP_WITH_ID_2_INNER

/**
 * Like MAP_WITH_ID but binds extra state to each call of op. The macro
 * being called as op(binding1, binding2, val, id).
 */
#define BIND2_MAP_WITH_ID(op, binding1, binding2, sep, ...)                            \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND2_MAP_WITH_ID_INNER(op, binding1, binding2, sep, I, ##__VA_ARGS__)))
#define BIND2_MAP_WITH_ID_INNER(op, binding1, binding2, sep, id, cur_val, ...)         \
  op(binding1, binding2, cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep(                   \
  ) DEFER2(_BIND2_MAP_WITH_ID_INNER)()(                                                \
    op, binding1, binding2, sep, CAT(id, I), ##__VA_ARGS__))
#define _BIND2_MAP_WITH_ID_INNER() BIND2_MAP_WITH_ID_INNER

/**
 * The same as BIND2_MAP_WITH_ID, except first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND2_MAP_WITH_ID_2(op, binding1, binding2, sep, ...)                          \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(BIND2_MAP_WITH_ID_2_INNER(op, binding1, binding2, sep, I, ##__VA_ARGS__)))
#define BIND2_MAP_WITH_ID_2_INNER(op, binding1, binding2, sep, id, cur_val, ...)       \
  op(binding1, binding2, cur_val, id) IF(HAS_ARGS(__VA_ARGS__))(sep(                   \
  ) DEFER2(_BIND2_MAP_WITH_ID_2_INNER)()(                                              \
    op, binding1, binding2, sep, CAT(id, I), ##__VA_ARGS__))
#define _BIND2_MAP_WITH_ID_2_INNER() BIND2_MAP_WITH_ID_2_INNER

/**
 * This is a variant of the MAP macro which also includes as an argument to the
 * operation a macro acc(val, curr) such that curr is accumulated to appropriately
 * by val.
 *
 * Usage:
 *   MAP_WITH_ACCUMULATE(op, sep, initial, acc, ...)
 *
 * Where op is a macro op(val, curr) which takes a list value and the so-far
 * accumulated value. This value is then later passed into acc, the macro
 * acc(val, curr) to generate the next accumulated value. The parameter
 * initial is the starting value of the accumulation.
 *
 * NOTE: The last entry is not accumulated but is passed to op. If desired
 * behaviour is to apply the accumulate first, then also apply acc within
 * op.
 *
 * Example:
 *
 *   #define STRIDE2(skip, curr_skip) curr_skip + skip
 *   #define ACCESS_WITH_STRIDE(skip, curr_skip) arr[curr_skip];
 *   MAP_WITH_ACCUMULATE(ACCESS_WITH_STRIDE, EMPTY, 0, STRIDE2, 2, 3, 6)
 *
 * Which expands to:
 *
 *   arr[0]; arr[0 + 2]; arr[0 + 2 + 3];
 *
 * The mechanism is analogous to the MAP macro.
 */
#define MAP_WITH_ACCUMULATE(op, sep, initial, acc, ...)                                \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(MAP_WITH_ACCUMULATE_INNER(op, sep, initial, acc, ##__VA_ARGS__)))
#define MAP_WITH_ACCUMULATE_INNER(op, sep, curr_acc, acc, map_entry, ...)              \
  op(map_entry,                                                                        \
     curr_acc) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_WITH_ACCUMULATE_INNER       \
  )()(op, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _MAP_WITH_ACCUMULATE_INNER() MAP_WITH_ACCUMULATE_INNER

/**
 * The same as MAP_WITH_ACCUMULATE, except any first-level MAP macro may be safely
 * nested inside this.
 */
#define MAP_WITH_ACCUMULATE_2(op, sep, initial, acc, ...)                              \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(MAP_WITH_ACCUMULATE_2_INNER(op, sep, initial, acc, ##__VA_ARGS__)))
#define MAP_WITH_ACCUMULATE_2_INNER(op, sep, curr_acc, acc, map_entry, ...)            \
  op(map_entry,                                                                        \
     curr_acc) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_WITH_ACCUMULATE_2_INNER     \
  )()(op, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _MAP_WITH_ACCUMULATE_2_INNER() MAP_WITH_ACCUMULATE_2_INNER

/**
 * Like MAP_WITH_ACCUMULATE but binds extra state to each call of op. The
 * macro being called as op(binding, val, curr).
 */
#define BIND_MAP_WITH_ACCUMULATE(op, binding, sep, initial, acc, ...)                  \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND_MAP_WITH_ACCUMULATE_INNER(op, binding, sep, initial, acc, ##__VA_ARGS__)))
#define BIND_MAP_WITH_ACCUMULATE_INNER(                                                \
    op, binding, sep, curr_acc, acc, map_entry, ...                                    \
)                                                                                      \
  op(binding, map_entry, curr_acc                                                      \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_WITH_ACCUMULATE_INNER             \
  )()(op, binding, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _BIND_MAP_WITH_ACCUMULATE_INNER() BIND_MAP_WITH_ACCUMULATE_INNER

/**
 * The same as BIND_MAP_WITH_ACCUMULATE, except any first-level MAP macro may be safely
 * nested inside this.
 */
#define BIND_MAP_WITH_ACCUMULATE_2(op, binding, sep, initial, acc, ...)                \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(                                                                              \
      BIND_MAP_WITH_ACCUMULATE_2_INNER(op, binding, sep, initial, acc, ##__VA_ARGS__)  \
  ))
#define BIND_MAP_WITH_ACCUMULATE_2_INNER(                                              \
    op, binding, sep, curr_acc, acc, map_entry, ...                                    \
)                                                                                      \
  op(binding, map_entry, curr_acc                                                      \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_WITH_ACCUMULATE_2_INNER           \
  )()(op, binding, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _BIND_MAP_WITH_ACCUMULATE_2_INNER() BIND_MAP_WITH_ACCUMULATE_2_INNER

/**
 * Like MAP_WITH_ACCUMULATE but binds extra state to each call of op. The
 * macro being called as op(binding1, binding2, val, curr).
 */
#define BIND2_MAP_WITH_ACCUMULATE(op, binding1, binding2, sep, initial, acc, ...)      \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND2_MAP_WITH_ACCUMULATE_INNER(                                               \
    op, binding1, binding2, sep, initial, acc, ##__VA_ARGS__)))
#define BIND2_MAP_WITH_ACCUMULATE_INNER(                                               \
    op, binding1, binding2, sep, curr_acc, acc, map_entry, ...                         \
)                                                                                      \
  op(binding1, binding2, map_entry, curr_acc                                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND2_MAP_WITH_ACCUMULATE_INNER            \
  )()(op, binding1, binding2, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _BIND2_MAP_WITH_ACCUMULATE_INNER() BIND2_MAP_WITH_ACCUMULATE_INNER

/**
 * The same as BIND2_MAP_WITH_ACCUMULATE, except any first-level MAP macro may be safely
 * nested inside this.
 */
#define BIND2_MAP_WITH_ACCUMULATE_2(op, binding1, binding2, sep, initial, acc, ...)    \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(                                                                              \
      BIND2_MAP_WITH_ACCUMULATE_2_INNER(                                               \
        op, binding1, binding2, sep, initial, acc, ##__VA_ARGS__)                      \
  ))
#define BIND2_MAP_WITH_ACCUMULATE_2_INNER(                                             \
    op, binding1, binding2, sep, curr_acc, acc, map_entry, ...                         \
)                                                                                      \
  op(binding1, binding2, map_entry, curr_acc                                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND2_MAP_WITH_ACCUMULATE_2_INNER          \
  )()(op, binding1, binding2, sep, acc(map_entry, curr_acc), acc, ##__VA_ARGS__))
#define _BIND2_MAP_WITH_ACCUMULATE_2_INNER() BIND2_MAP_WITH_ACCUMULATE_2_INNER

/**
 * This is a variant of the MAP macro which iterates over pairs rather than
 * singletons.
 *
 * Usage:
 *   MAP_PAIRS(op, sep, ...)
 *
 * Where op is a macro op(val_1, val_2) which takes two list values.
 *
 * Example:
 *
 *   #define MAKE_STATIC_VAR(type, name) static type name;
 *   MAP_PAIRS(MAKE_STATIC_VAR, EMPTY, char, my_char, int, my_int)
 *
 * Which expands to:
 *
 *   static char my_char; static int my_int;
 *
 * The mechanism is analogous to the MAP macro.
 */
#define MAP_PAIRS(op, sep, ...)                                                        \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(MAP_PAIRS_INNER(op, sep, __VA_ARGS__)))
#define MAP_PAIRS_INNER(op, sep, cur_val_1, cur_val_2, ...)                            \
  op(cur_val_1, cur_val_2                                                              \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_PAIRS_INNER)()(op, sep, __VA_ARGS__))
#define _MAP_PAIRS_INNER() MAP_PAIRS_INNER

/**
 * The same as MAP_PAIRS, except any first-level MAP macro may be safely nested inside
 * this.
 */
#define MAP_PAIRS_2(op, sep, ...)                                                      \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(MAP_PAIRS_2_INNER(op, sep, __VA_ARGS__)))
#define MAP_PAIRS_2_INNER(op, sep, cur_val_1, cur_val_2, ...)                          \
  op(cur_val_1, cur_val_2) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_PAIRS_2_INNER   \
  )()(op, sep, __VA_ARGS__))
#define _MAP_PAIRS_2_INNER() MAP_PAIRS_2_INNER

/**
 * Like MAP_PAIRS but binds extra state to each call of op. The
 * macro being called as op(binding, val_1, val_2).
 */
#define BIND_MAP_PAIRS(op, binding, sep, ...)                                          \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(BIND_MAP_PAIRS_INNER(op, sep, __VA_ARGS__)))
#define BIND_MAP_PAIRS_INNER(op, binding, sep, cur_val_1, cur_val_2, ...)              \
  op(binding, cur_val_1, cur_val_2) IF(HAS_ARGS(__VA_ARGS__))(sep(                     \
  ) DEFER2(_BIND_MAP_PAIRS_INNER)()(op, binding, sep, __VA_ARGS__))
#define _BIND_MAP_PAIRS_INNER() BIND_MAP_PAIRS_INNER

/**
 * The same as BIND_MAP_PAIRS, except any first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND_MAP_PAIRS_2(op, binding, sep, ...)                                        \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(BIND_MAP_PAIRS_2_INNER(op, sep, __VA_ARGS__)))
#define BIND_MAP_PAIRS_2_INNER(op, binding, sep, cur_val_1, cur_val_2, ...)            \
  op(binding, cur_val_1, cur_val_2) IF(HAS_ARGS(__VA_ARGS__))(sep(                     \
  ) DEFER2(_BIND_MAP_PAIRS_2_INNER)()(op, binding, sep, __VA_ARGS__))
#define _BIND_MAP_PAIRS_2_INNER() BIND_MAP_PAIRS_2_INNER

/**
 * Like MAP_PAIRS but binds extra state to each call of op. The
 * macro being called as op(binding1, binding2, val_1, val_2).
 */
#define BIND2_MAP_PAIRS(op, binding1, binding2, sep, ...)                              \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(BIND2_MAP_PAIRS_INNER(op, sep, __VA_ARGS__)))
#define BIND2_MAP_PAIRS_INNER(op, binding1, binding2, sep, cur_val_1, cur_val_2, ...)  \
  op(binding1, binding2, cur_val_1, cur_val_2) IF(HAS_ARGS(__VA_ARGS__))(sep(          \
  ) DEFER2(_BIND2_MAP_PAIRS_INNER)()(op, binding1, binding2, sep, __VA_ARGS__))
#define _BIND2_MAP_PAIRS_INNER() BIND2_MAP_PAIRS_INNER

/**
 * The same as BIND2_MAP_PAIRS, except any first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND2_MAP_PAIRS_2(op, binding1, binding2, sep, ...)                            \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(BIND2_MAP_PAIRS_2_INNER(op, sep, __VA_ARGS__)))
#define BIND2_MAP_PAIRS_2_INNER(                                                       \
  op, binding1, binding2, sep, cur_val_1, cur_val_2, ...                               \
)                                                                                      \
  op(binding1, binding2, cur_val_1, cur_val_2) IF(HAS_ARGS(__VA_ARGS__))(sep(          \
  ) DEFER2(_BIND2_MAP_PAIRS_2_INNER)()(op, binding1, binding2, sep, __VA_ARGS__))
#define _BIND2_MAP_PAIRS_2_INNER() BIND2_MAP_PAIRS_2_INNER

/**
 * This is a variant of the MAP macro which iterates over a two-element sliding
 * window.
 *
 * Usage:
 *   MAP_SLIDE(op, last_op, sep, ...)
 *
 * Where op is a macro op(val_1, val_2) which takes the two list values
 * currently in the window. last_op is a macro taking a single value which is
 * called for the last argument.
 *
 * Example:
 *
 *   #define SIMON_SAYS_OP(simon, next) IF(NOT(simon()))(next)
 *   #define SIMON_SAYS_LAST_OP(val) last_but_not_least_##val
 *   #define SIMON_SAYS() 0
 *
 *   MAP_SLIDE(SIMON_SAYS_OP, SIMON_SAYS_LAST_OP, EMPTY, wiggle, SIMON_SAYS, dance,
 * move, SIMON_SAYS, boogie, stop)
 *
 * Which expands to:
 *
 *   dance boogie last_but_not_least_stop
 *
 * The mechanism is analogous to the MAP macro.
 */
#define MAP_SLIDE(op, last_op, sep, ...)                                               \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL(MAP_SLIDE_INNER(op, last_op, sep, __VA_ARGS__)))
#define MAP_SLIDE_INNER(op, last_op, sep, cur_val, ...)                                \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(cur_val, FIRST(__VA_ARGS__))) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(cur_val)    \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_SLIDE_INNER                            \
  )()(op, last_op, sep, __VA_ARGS__))
#define _MAP_SLIDE_INNER() MAP_SLIDE_INNER

/**
 * The same as MAP_SLIDE, except any first-level MAP macro may be safely nested inside
 * this.
 */
#define MAP_SLIDE_2(op, last_op, sep, ...)                                             \
  IF(HAS_ARGS(__VA_ARGS__)) (EVAL2(MAP_SLIDE_2_INNER(op, last_op, sep, __VA_ARGS__)))
#define MAP_SLIDE_2_INNER(op, last_op, sep, cur_val, ...)                              \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(cur_val, FIRST(__VA_ARGS__))) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(cur_val)    \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_MAP_SLIDE_2_INNER                          \
  )()(op, last_op, sep, __VA_ARGS__))
#define _MAP_SLIDE_2_INNER() MAP_SLIDE_2_INNER

/**
 * Like MAP_SLIDE but binds extra state to each call of op and
 * last_op. The macros being called as op(binding, val_1, val_2)
 * and last_op(binding, final_val).
 */
#define BIND_MAP_SLIDE(op, binding, last_op, sep, ...)                                 \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND_MAP_SLIDE_INNER(op, binding, last_op, sep, __VA_ARGS__)))
#define BIND_MAP_SLIDE_INNER(op, binding, last_op, sep, cur_val, ...)                  \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(binding, cur_val, FIRST(__VA_ARGS__))                                            \
  ) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(binding, cur_val)                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_SLIDE_INNER                       \
  )()(op, binding, last_op, sep, __VA_ARGS__))
#define _BIND_MAP_SLIDE_INNER() BIND_MAP_SLIDE_INNER

/**
 * The same as BIND_MAP_SLIDE, except any first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND_MAP_SLIDE_2(op, binding, last_op, sep, ...)                               \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(BIND_MAP_SLIDE_2_INNER(op, binding, last_op, sep, __VA_ARGS__)))
#define BIND_MAP_SLIDE_2_INNER(op, binding, last_op, sep, cur_val, ...)                \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(binding, cur_val, FIRST(__VA_ARGS__))                                            \
  ) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(binding, cur_val)                           \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND_MAP_SLIDE_2_INNER                     \
  )()(op, binding, last_op, sep, __VA_ARGS__))
#define _BIND_MAP_SLIDE_2_INNER() BIND_MAP_SLIDE_2_INNER

/**
 * Like MAP_SLIDE but binds extra state to each call of op and
 * last_op. The macros being called as op(binding1, binding2, val_1, val_2)
 * and last_op(binding1, binding2, final_val).
 */
#define BIND2_MAP_SLIDE(op, binding1, binding2, last_op, sep, ...)                     \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL(BIND2_MAP_SLIDE_INNER(op, binding1, binding2, last_op, sep, __VA_ARGS__)))
#define BIND2_MAP_SLIDE_INNER(op, binding1, binding2, last_op, sep, cur_val, ...)      \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(binding1, binding2, cur_val, FIRST(__VA_ARGS__))                                 \
  ) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(binding1, binding2, cur_val)                \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND2_MAP_SLIDE_INNER                      \
  )()(op, binding1, binding2, last_op, sep, __VA_ARGS__))
#define _BIND2_MAP_SLIDE_INNER() BIND2_MAP_SLIDE_INNER

/**
 * The same as BIND2_MAP_SLIDE, except any first-level MAP macro may be safely nested
 * inside this.
 */
#define BIND2_MAP_SLIDE_2(op, binding1, binding2, last_op, sep, ...)                   \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (EVAL2(BIND2_MAP_SLIDE_2_INNER(op, binding1, binding2, last_op, sep, __VA_ARGS__)))
#define BIND2_MAP_SLIDE_2_INNER(op, binding1, binding2, last_op, sep, cur_val, ...)    \
  IF(HAS_ARGS(__VA_ARGS__))                                                            \
  (op(binding1, binding2, cur_val, FIRST(__VA_ARGS__))                                 \
  ) IF(NOT(HAS_ARGS(__VA_ARGS__)))(last_op(binding1, binding2, cur_val)                \
  ) IF(HAS_ARGS(__VA_ARGS__))(sep() DEFER2(_BIND2_MAP_SLIDE_2_INNER                    \
  )()(op, binding1, binding2, last_op, sep, __VA_ARGS__))
#define _BIND2_MAP_SLIDE_2_INNER() BIND2_MAP_SLIDE_2_INNER

/**
 * Strip any excess commas from a set of arguments.
 */

#define REMOVE_TRAILING_COMMAS(...) MAP(PASS, COMMA, __VA_ARGS__)

/**
 * Not a preprocessor but a constexpr to convert ID "I", "II" etc to integer.
 */
constexpr uint32_t ID_TO_INT(std::string_view ID) {
    return static_cast<uint32_t>(ID.size() - 1);
}

#endif  // hemlock_preprocessor_h
