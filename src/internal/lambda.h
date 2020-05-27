#ifndef LAMBDA_HELPERS_H_
#define LAMBDA_HELPERS_H_

/** Lambda Helper: Decay to Function Pointer
 *
 * When you need to convert a lambda with captures to a C pointer,
 * such as when a callback interface doesn't provide a variable for private data
 * which can store a `this` pointer, use this helper.
 *
 * @code
 * int a = 100;
 * auto b = [&](void*) {return ++a;};
 *
 * // Converting lambda with captures to a C pointer
 * void (*f1)(void*) = Lambda::ptr(b);
 * f1(nullptr);
 * printf("%d\n", a);  // 101
 *
 * // In case return value should be used
 * int (*f3)(void*) = Lambda::ptr<int>(b);
 * printf("%d\n", f3(nullptr)); // 103
 *
 * // In case data is used
 * auto b2 = [&](void* data) {return *(int*)(data) + a;};
 * int (*f4)(void*) = Lambda::ptr<int>(b2);
 * int data = 5;
 * printf("%d\n", f4(&data)); // 108
 * @endcode
 *
 * Source: https://stackoverflow.com/questions/7852101/c-lambda-with-captures-as-a-function-pointer
 */
struct Lambda
{
	template<typename Tret, typename T>
	static Tret lambda_ptr_exec(char data)
	{
		return (Tret)(*(T*)fn<T>())(data);
	}

	template<typename Tret = void, typename Tfp = Tret (*)(char), typename T>
	static Tfp ptr(T& t)
	{
		fn<T>(&t);
		return (Tfp)lambda_ptr_exec<Tret, T>;
	}

	template<typename T>
	static void* fn(void* new_fn = nullptr)
	{
		static void* fn;
		if(new_fn != nullptr)
		{
			fn = new_fn;
		}
		return fn;
	}
};

#endif // LAMBDA_HELPERS_H_
