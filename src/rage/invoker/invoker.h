#pragma once
#include <stdint.h>
#include <string.h>

// GTA V native invoker for the PS4 CUSA00411 v1.57 eboot.
//
// The generated natives.h calls _i<R>(0xRVA, args...), where the literal is the
// native impl's RVA (offset from the eboot's ELF base 0), NOT a hash. Unlike a
// PC invoker there is no hash->function crossmap: we add the runtime image base
// (resolved once at load) to the RVA and call the impl directly.
//
// The call context layout below IS the RAGE scrNativeCallContext ABI recovered
// from the binary: return pointer @0, argument count @8, argument array @16,
// each argument and the return value one 8-byte slot. Do not reorder these
// fields.
//
// Vector3 OUT-params (natives spelled `math::vector3<float>*` in natives.h) are
// passed as RAW POINTERS: the native writes straight through the caller's
// pointer. We deliberately do NOT use a ScriptHookV-style vector redirect (stash
// pointer, push &result_vectors[n], copy back after the call). The "Basic" PS4
// invoker carries a setVectors()/argVectors mechanism but never populates it
// (vectorCount stays 0) -- it is vestigial, and its Vector3 out-params work
// purely through raw pointers on-console. A blind redirect would also be unsafe:
// the invoker cannot tell an out-param from an in-param (e.g. create_itemset
// takes a vector3* INPUT), so redirecting would corrupt inputs. The
// m_script_vectors/m_result_vectors fields exist only for ABI-layout parity.

namespace rage::invoker {

	struct vec3f { float x, y, z; };
	struct vec4f { float x, y, z, w; };

	class native_context {
	protected:
		void* m_return_data;          // @0
		uint32_t m_argument_count;    // @8
		void* m_argument_data;        // @16
		uint32_t m_data_count;
		vec3f* m_script_vectors[4];
		vec4f m_result_vectors[4];
	public:
		template<typename T>
		T get_argument(int index) {
			return *(T*)&((uint64_t*)m_argument_data)[index];
		}
		template<typename T>
		void set_argument(int index, T value) {
			*(T*)&((uint64_t*)m_argument_data)[index] = value;
		}
		template<typename T>
		T get_return_result() {
			return *(T*)m_return_data;
		}
		template<typename T>
		void set_return(T value) {
			*(T*)m_return_data = value;
		}
		void* get_return_data() { return m_return_data; }
		uint32_t get_argument_count() { return m_argument_count; }
	};

	// Args and the return value share one 8-byte-slot buffer: arguments are read
	// by the native before it writes the result, exactly as ScriptHookV's
	// nativeInit/nativePush/nativeCall does.
	class native_setup : public native_context {
	private:
		uint8_t m_temp_buffer[256];
	public:
		native_setup() {
			m_argument_data = m_temp_buffer;
			m_return_data = m_temp_buffer;
			m_argument_count = 0;
			m_data_count = 0;
			memset(m_temp_buffer, 0, sizeof(m_temp_buffer));
		}
		template<typename T>
		void push(T value) {
			memset(&m_temp_buffer[8 * m_argument_count], 0, 8);
			*(T*)&m_temp_buffer[8 * m_argument_count] = value;
			m_argument_count++;
		}
		template<typename T>
		T get_return() { return *(T*)m_temp_buffer; }
	};

	typedef void(*native_handler)(native_context*);

	// Runtime base of the GTA eboot's first segment; RVA + this = absolute impl.
	// 0 until resolve_base() succeeds; invoke() refuses to call while it is 0.
	extern uintptr_t g_eboot_base;

	bool resolve_base();

	struct pass { template<typename... T> pass(T...) {} };

	template<typename R, typename... Args>
	R invoke(uint64_t rva, Args&&... args) {
		native_setup ctx;
		pass{ ([&]() { ctx.push(args); }(), 1)... };
		if (!g_eboot_base)
			return R();
		native_handler handler = (native_handler)(g_eboot_base + rva);
		handler(&ctx);
		return ctx.get_return<R>();
	}
}
