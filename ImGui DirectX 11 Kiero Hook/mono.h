#pragma once

#include "includes.h"

#ifndef NO_SECURITY
#include <VirtualizerSDK.h>
#endif 

#include "resolver.h"

#define mono_method( fn, args, ... ) using t_##fn = args; static t_##fn fn{ 0 }; if(!fn) fn = reinterpret_cast< t_##fn >( mono::get_method( ##__VA_ARGS__ ) )

struct driver_t
{

	template <typename type>
	bool write(uintptr_t address, type value)
	{
		
		if (!address)
			return false;

		*reinterpret_cast<type*>(address) = value;

		return true;

	}

	bool write(uintptr_t address, uintptr_t value, size_t size)
	{

		if (!address || !value || !size)
			return false;

		memcpy((void*)address, (void*)value, size);

		return true;

	}

	template <typename type>
	type read(uintptr_t address)
	{
		
		if (!address)
			return {};

		return *reinterpret_cast<type*>(address);

	}

	bool read(uintptr_t address, uintptr_t buffer_address, size_t size)
	{

		if (!address || !buffer_address || !size)
			return false;

		memcpy((void*)buffer_address, (void*)address, size);

	}

	bool read(uintptr_t address, void* buffer_address, size_t size)
	{

		if (!address || !buffer_address || !size)
			return false;

		memcpy(buffer_address, (void*)address, size);

	}

	template <typename type>
	type read_chain(uintptr_t address, std::vector<uintptr_t> chain)
	{

		uintptr_t current = address;

		for (int i = 0; i < chain.size() - 1; i++)
			current = read<uintptr_t>(current + chain[i]);

		return read<type>(current + chain[chain.size() - 1]);

	}

	template <typename type>
	std::vector<type> read_vector(uintptr_t address, size_t size, size_t custom_type_size = NULL)
	{

		std::vector<type> temp{};

		if (!size)
			return temp;

		temp.resize(custom_type_size ? custom_type_size : sizeof(type) * size);

		if (read(address, reinterpret_cast<uintptr_t>(&temp[0]), custom_type_size ? custom_type_size : sizeof(type) * size))
			return temp;

		return temp;

	}

	std::string read_unicode_str(uintptr_t address, size_t size)
	{
		address = address + 0x14;
		char16_t wcharTemp[64] = { '\0' };
		read((ULONG64)address, (ULONG64)wcharTemp, size * 2);
		std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(wcharTemp);
		return u8_conv;
	}

	std::wstring read_wstr(uintptr_t address)
	{

		wchar_t buffer[128 * sizeof(wchar_t)];

		read(address, &buffer, 128 * sizeof(wchar_t));

		auto str = std::wstring(buffer);
		if (str.size() > 64 || str.size() <= 2)
			return L"None";

		return str;

	}

	std::string read_str(uintptr_t address, int size = 64)
	{
		std::unique_ptr<char[]> buffer(new char[size]);
		read(address, buffer.get(), size);

		if (!buffer.get())
			return "invalid";

		return std::string(buffer.get());
	}

}; inline driver_t driver{};

#define OFFSET(func, type, offset) type func { return driver.read<type>( reinterpret_cast<uintptr_t>( this ) + offset ); } 

namespace static_modules
{

	inline uintptr_t game_assembly = NULL;
	inline uintptr_t mono = NULL;

}

inline std::unordered_map<uintptr_t, uintptr_t> functions;

inline unsigned short utf8_to_utf16(const char* val) {
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string dest = convert.from_bytes(val);
	return *reinterpret_cast<unsigned short*>(&dest[0]);
}

inline std::string read_widechar(const std::uintptr_t address, const std::size_t size) {
	const auto buffer = std::make_unique<char[]>(size);
	driver.read(address, buffer.get(), size);
	return std::string(buffer.get());
}

inline std::wstring read_unicode(const std::uintptr_t address, const std::size_t size) {
	const auto buffer = std::make_unique<wchar_t[]>(size * 2);
	driver.read(address, buffer.get(), size);
	return std::wstring(buffer.get());
}

struct glist_t {
	OFFSET(data(), uintptr_t, 0x0)
	OFFSET(next(), uintptr_t, 0x8)
};

struct mono_root_domain_t {
	OFFSET(domain_assemblies(), glist_t*, 0xA0)
	OFFSET(domain_id(), int, 0x94)
	OFFSET(jitted_function_table(), uintptr_t, 0x120)
};

namespace mono
{

	inline mono_root_domain_t* get_root_domain() {
		return reinterpret_cast<mono_root_domain_t*>(driver.read<uintptr_t>(static_modules::mono + 0x72F020));
	}

}

struct mono_table_info_t {
	int get_rows() {
		return driver.read<int>(reinterpret_cast<uintptr_t>(this) + 0x8) & 0xFFFFFFu;
	}
};

struct mono_method_t {
	std::string name() {
		if (!this)
			return "";

		auto ptr = driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x18);

		if (!ptr)
			return "";

		auto name = read_widechar(ptr , 128);
		if (static_cast<std::uint8_t>(name[0]) == 0xEE) { // https://github.com/cheat-engine/cheat-engine/blob/8bdb7f28a9d40ccaa6f4116b13c570907ce9ee2e/Cheat%20Engine/MonoDataCollector/MonoDataCollector/PipeServer.cpp#L896
			char name_buff[32];
			sprintf_s(name_buff, 32, "\\u%04X", utf8_to_utf16(const_cast<char*>(name.c_str())));
			name = name_buff;
		}

		return name;
	}
};

struct mono_class_field_t {

		int offset()
		{

			if (!this)
				return 0;

			return driver.read<int>(reinterpret_cast<uintptr_t>(this) + 0x18);

		}

		std::string name() {
		auto name = read_widechar(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x8), 128);
		if (static_cast<std::uint8_t>(name[0]) == 0xEE) {
			char name_buff[32];
			sprintf_s(name_buff, 32, "\\u%04X", utf8_to_utf16(const_cast<char*>(name.c_str())));
			name = name_buff;
		}

		return name;
	}
};

struct mono_class_runtime_info_t {
	OFFSET(max_domain(), int, 0x0)
};

struct mono_vtable_t {
	OFFSET(flags(), BYTE, 0x30)

	uintptr_t get_static_field_data() 
	{
		if (!this)
			return NULL;
		if ((this->flags() & 4) != 0)
			return driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 8 * driver.read<int>(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x0) + 0x5C) + 0x48);

		return 0;

	}
};

struct cached_field_t
{
	std::string field_name;
	mono_class_field_t* field;
	uintptr_t klass;
};

inline std::vector<cached_field_t> field_cache{};

struct mono_class_t 
{

	OFFSET(num_fields(), int, 0x100)
	OFFSET(runtime_info(), mono_class_runtime_info_t*, 0xD0)

	std::string name()
	{

		auto name = read_widechar(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x48), 128);

		if (static_cast<std::uint8_t>(name[0]) == 0xEE)
		{

			char name_buff[32];
			sprintf_s(name_buff, 32, "\\u%04X", utf8_to_utf16(const_cast<char*>(name.c_str())));
			name = name_buff;

		}

		return name;

	}

	std::string namespace_name() 
	{

		auto name = read_widechar(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x50), 128);

		if (static_cast<std::uint8_t>(name[0]) == 0xEE) 
		{

			char name_buff[32];
			sprintf_s(name_buff, 32, ("\\u%04X"), utf8_to_utf16(const_cast<char*>(name.c_str())));
			name = name_buff;

		}

		return name;

	}

	int get_num_methods() 
	{

		auto count = driver.read<unsigned int>((uintptr_t)this + 0x1B) - 1;

		switch (count)
		{

		case 0u:
		case 1u:
			return driver.read<int>((uintptr_t)this + 0xFC);
		case 2u:
		{

			auto klass = driver.read<uintptr_t>(driver.read<uintptr_t>((uintptr_t)this + 0xF0));
			count = driver.read<unsigned int>(klass + 0x1B) - 1;
			if (count <= 0xAB)
				return NULL;

		}
		case 3u:
		case 5u:
			return 0;

		case 4u:
			return driver.read<int>((uintptr_t)this + 0xF0);

		default: break;

		}

		return 0;

	}

	mono_method_t* get_method(const int i) 
	{

		if (!this)
			return nullptr;

		auto ptr = driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0xA0);

		if (!ptr)
			return nullptr;

		auto ret = reinterpret_cast<mono_method_t*>(driver.read<uintptr_t>(ptr + 0x8 * i));

		return ret;

	}

	mono_class_field_t* get_field(const int i) {
		return reinterpret_cast<mono_class_field_t*>(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x98) + 0x20 * i);
	}

	mono_vtable_t* get_vtable(mono_root_domain_t* domain) {
		if (!this)
			return nullptr;
		const auto runtime_info = this->runtime_info();
		if (!runtime_info)
			return nullptr;

		const auto domain_id = domain->domain_id();
		const auto max_domain = runtime_info->max_domain();

		if (max_domain < domain_id)
			return nullptr;

		return reinterpret_cast<mono_vtable_t*>(driver.read<uintptr_t>(reinterpret_cast<uintptr_t>(runtime_info) + 8 * domain_id + 8));
	}

	mono_method_t* find_method(const char* method_name) {
		std::cout << "[~] entered find_method\n";
		if (!this)
			return nullptr;
		auto mono_ptr = uintptr_t();
		auto method_num = this->get_num_methods();
		std::cout << "[+] method_count: " << method_num << "\n";
		for (auto i = 0; i < method_num; i++) {
			const auto method = this->get_method(i);
			std::cout << "[" << i << "] method : " << method << "\n";
			if (!method)
				continue;

			auto name = method->name().c_str();

			if (!strcmp(name, method_name))
			{
				std::cout << "[!] found " << name << "\n";
				mono_ptr = reinterpret_cast<uintptr_t>(method);
			}
			else
				std::cout << "[~] mismatch " << name << " != " << method_name << "\n";

		}
		return reinterpret_cast<mono_method_t*>(functions[mono_ptr]);
	}

	mono_class_field_t* find_field(const char* field_name) {

		if (!this)
			return nullptr;

		for (auto i = 0; i < this->num_fields(); i++) {
			const auto field = this->get_field(i);
			if (!field)
				continue;

			if (!strcmp(field->name().c_str(), field_name))
				return field;
		}

		return nullptr;
	}

	int find_field_offset(std::string field_name)
	{

		for (auto i = 0; i < this->num_fields(); i++) 
		{

			const auto field = this->get_field(i);

			if (!field)
				continue;

			if (!strcmp(field->name().c_str(), field_name.c_str()))
			{

				auto offset = field->offset();

				return offset;

			}

		}

		return -1;

	}

	uintptr_t get_static_instance()
	{

		auto instance = get_vtable(mono::get_root_domain())->get_static_field_data();
		return instance;

	}

};

struct mono_hash_table_t {
	OFFSET(size(), int, 0x18)
	OFFSET(data(), uintptr_t, 0x20)
	OFFSET(next_value(), void*, 0x108)
	OFFSET(key_extract(), unsigned int, 0x58)

	template<typename T>
	T* lookup(void* key) {
		auto v4 = static_cast<mono_hash_table_t*>(driver.read<void*>(data() + 0x8 * (reinterpret_cast<unsigned int>(key) % this->size())));
		if (!v4)
			return nullptr;

		while (reinterpret_cast<void*>(v4->key_extract()) != key) {
			v4 = static_cast<mono_hash_table_t*>(v4->next_value());
			if (!v4)
				return nullptr;
		}

		return reinterpret_cast<T*>(v4);
	}
};

struct mono_image_t {
	OFFSET(flags(), int, 0x1C)

	mono_table_info_t* get_table_info(const int table_id) {
		if (table_id > 55)
			return nullptr;

		return reinterpret_cast<mono_table_info_t*>(reinterpret_cast<uintptr_t>(this) + 0x10 * (static_cast<int>(table_id) + 0xF));
	}

	mono_class_t* get(const int type_id) {
		if ((this->flags() & 0x20) != 0)
			return nullptr;

		if ((type_id & 0xFF000000) != 0x2000000)
			return nullptr;

		return reinterpret_cast<mono_hash_table_t*>(this + 0x4D0)->lookup<mono_class_t>(reinterpret_cast<void*>(type_id));
	}
};

struct mono_assembly_t {
	OFFSET(mono_image(), mono_image_t*, 0x60)
};

struct MonoThread
{

};

struct MonoDomain
{

};

struct MonoAssembly
{

};

struct MonoImage 
{

};

struct MonoClass
{

};

struct MonoMethod
{

};

struct MonoObject
{

};

struct MonoClassField
{

};

struct MonoVTable
{

};

struct MonoType
{

};

struct MonoReflectionType
{

};

typedef MonoThread* (*t_mono_thread_attach)(MonoDomain* domain);
typedef MonoDomain* (*t_mono_get_root_domain)();
typedef MonoAssembly* (*t_mono_domain_assembly_open)(MonoDomain* doamin, const char* name);
typedef MonoImage* (*t_mono_assembly_get_image)(MonoAssembly* assembly);
typedef MonoClass* (*t_mono_class_from_name)(MonoImage* image, const char* name_space, const char* name);
typedef MonoMethod* (*t_mono_class_get_method_from_name)(MonoClass* klass, const char* name, int param_count);
typedef void* (*t_mono_compile_method)(MonoMethod* method);
typedef MonoObject* (*t_mono_runtime_invoke)(MonoMethod* method, void* obj, void** params, MonoObject** exc);

typedef MonoClassField* (*t_mono_class_get_field_from_name)(MonoClass* klass, const char* name);
typedef void* (*t_mono_field_get_value)(void* obj, MonoClassField* field, void* value);
typedef void (*t_mono_field_set_value)(MonoObject* obj, MonoClassField* field, void* value);
typedef MonoClass* (*t_mono_method_get_class)(MonoMethod* method);
typedef MonoVTable* (*t_mono_class_vtable)(MonoDomain* domain, MonoClass* klass);
typedef void* (*t_mono_vtable_get_static_field_data)(MonoVTable* vt);
typedef uint32_t(*t_mono_field_get_offset)(MonoClassField* field);

typedef void* (*t_mono_method_get_unmanaged_thunk)(MonoMethod* method);
typedef void* (*t_mono_object_unbox)(MonoObject* obj);

typedef MonoType* (*t_mono_class_get_type)(MonoClass* klass);
typedef MonoReflectionType* (*t_mono_type_get_object)(MonoDomain*, void*);

namespace mono {

	template <class t>
	t resolve_export(const char* name) { return reinterpret_cast<t>(GetProcAddress((HMODULE)static_modules::mono, name)); }


	//t_mono_thread_attach mono_thread_attach{};
	//t_mono_get_root_domain mono_get_root_domain{};
	//t_mono_domain_assembly_open mono_domain_assembly_open{};
	//t_mono_assembly_get_image mono_assembly_get_image{};
	//t_mono_class_from_name mono_class_from_name{};
	//t_mono_class_get_method_from_name mono_class_get_method_from_name{};
	//t_mono_compile_method mono_compile_method{};
	//t_mono_runtime_invoke mono_runtime_invoke{};
	//t_mono_class_get_field_from_name mono_class_get_field_from_name{};
	//t_mono_field_get_value mono_field_get_value{};
	//t_mono_field_set_value mono_field_set_value{};
	//t_mono_method_get_class mono_method_get_class{};
	//t_mono_class_vtable mono_class_vtable{};
	//t_mono_vtable_get_static_field_data mono_vtable_get_static_field_data{};
	//t_mono_field_get_offset mono_field_get_offset{};
	//t_mono_method_get_unmanaged_thunk mono_method_get_unmanaged_thunk{};
	//t_mono_object_unbox mono_object_unbox{};
	//t_mono_class_get_type mono_class_get_type{};
	t_mono_type_get_object mono_type_get_object{};

	inline void init()
	{

		//const auto jitted_table = get_root_domain()->jitted_function_table();

		//for (auto i = 0; i < driver.read<int>(jitted_table + 0x8); i++) {
		//	const auto entry = driver.read<uintptr_t>(jitted_table + 0x10 + i * 0x8);
		//	if (!entry)
		//		continue;
		//	std::cout << "[" << entry << "] entry : " << entry << "\n";
		//	for (auto j = 0; j < driver.read<int>(entry + 0x4); j++) {
		//		const auto function = driver.read<uintptr_t>(entry + 0x18 + j * 0x8);
		//		if (!function)
		//			continue;
		//		std::cout << "[" << j << "] function : " << function << "\n";
		//		const auto mono_ptr = driver.read<uintptr_t>(function + 0x0);
		//		const auto jitted_ptr = driver.read<uintptr_t>(function + 0x10);
		//		functions[mono_ptr] = jitted_ptr;
		//	}
		//}

		auto hMono = GetModuleHandleA("mono-2.0-bdwgc.dll");
		if (hMono == NULL)
			return;

		//mono_domain_assembly_open = reinterpret_cast<t_mono_domain_assembly_open>(GetProcAddress(hMono, "mono_domain_assembly_open"));
		//mono_assembly_get_image = reinterpret_cast<t_mono_assembly_get_image>(GetProcAddress(hMono, "mono_assembly_get_image"));
		//mono_class_from_name = reinterpret_cast<t_mono_class_from_name>(GetProcAddress(hMono, "mono_class_from_name"));
		//mono_class_get_method_from_name = reinterpret_cast<t_mono_class_get_method_from_name>(GetProcAddress(hMono, "mono_class_get_method_from_name"));
		//mono_compile_method = reinterpret_cast<t_mono_compile_method>(GetProcAddress(hMono, "mono_compile_method"));
		//mono_runtime_invoke = reinterpret_cast<t_mono_runtime_invoke>(GetProcAddress(hMono, "mono_runtime_invoke"));

		//mono_class_get_field_from_name = reinterpret_cast<t_mono_class_get_field_from_name>(GetProcAddress(hMono, "mono_class_get_field_from_name"));
		//mono_field_get_value = reinterpret_cast<t_mono_field_get_value>(GetProcAddress(hMono, "mono_field_get_value"));
		//mono_field_set_value = reinterpret_cast<t_mono_field_set_value>(GetProcAddress(hMono, "mono_field_set_value"));
		//mono_method_get_class = reinterpret_cast<t_mono_method_get_class>(GetProcAddress(hMono, "mono_method_get_class"));
		//mono_class_vtable = reinterpret_cast<t_mono_class_vtable>(GetProcAddress(hMono, "mono_class_vtable"));
		//mono_vtable_get_static_field_data = reinterpret_cast<t_mono_vtable_get_static_field_data>(GetProcAddress(hMono, "mono_vtable_get_static_field_data"));
		//mono_field_get_offset = reinterpret_cast<t_mono_field_get_offset>(GetProcAddress(hMono, "mono_field_get_offset"));

		//mono_thread_attach = reinterpret_cast<t_mono_thread_attach>(GetProcAddress(hMono, "mono_thread_attach"));
		//mono_get_root_domain = reinterpret_cast<t_mono_get_root_domain>(GetProcAddress(hMono, "mono_get_root_domain"));

		//mono_method_get_unmanaged_thunk = reinterpret_cast<t_mono_method_get_unmanaged_thunk>(GetProcAddress(hMono, "mono_method_get_unmanaged_thunk"));
		//mono_object_unbox = reinterpret_cast<t_mono_object_unbox>(GetProcAddress(hMono, "mono_object_unbox"));

		//mono_class_get_type = reinterpret_cast<t_mono_class_get_type>(GetProcAddress(hMono, "mono_class_get_type"));
		mono_type_get_object = reinterpret_cast<t_mono_type_get_object>(GetProcAddress(hMono, "mono_type_get_object"));

		//mono_thread_attach(mono_get_root_domain());

	}

	inline mono_assembly_t* domain_assembly_open(mono_root_domain_t* domain, const char* name) {
		auto domain_assemblies = domain->domain_assemblies();

		if (!domain_assemblies)
			return nullptr;

		auto data = uintptr_t();
		while (true) {
			data = domain_assemblies->data();
			if (!data)
				continue;

			const auto data_name = read_widechar(driver.read<uintptr_t>(data + 0x10), 128);

			if (!strcmp(data_name.c_str(), name))
				break;

			domain_assemblies = reinterpret_cast<glist_t*>(domain_assemblies->next());
			if (!domain_assemblies)
				break;
		}

		return reinterpret_cast<mono_assembly_t*>(data);
	}

	struct cached_class_t
	{
		std::string name;
		mono_class_t* klass;
	};

	struct cached_assembly_info_t
	{
		std::string assembly_name;
		std::vector<cached_class_t> classes;
	};

	inline std::vector<cached_assembly_info_t> cached_assemblies{};
	inline bool invalid_classes = false;

	mono_class_t* find_class(const char* assembly_name, const char* class_name) 
	{
#ifndef NO_SECURITY
		VIRTUALIZER_START;
#endif

		bool cache_hit = false;

		for (const auto& cached_assembly : cached_assemblies)
		{

			if (cached_assembly.assembly_name == assembly_name)
			{

				cache_hit = true;

				for (const auto& klass : cached_assembly.classes)
				{

					if (klass.name == class_name)
						return klass.klass;

				}

			}

		}

		mono_class_t* ret = nullptr;

		if (!cache_hit)
		{

			cached_assembly_info_t insertion_cache{};
			insertion_cache.assembly_name = assembly_name;

			static auto root_domain = get_root_domain();
			if (!root_domain)
				return nullptr;

			const auto domain_assembly = domain_assembly_open(root_domain, assembly_name);
			if (!domain_assembly)
				return nullptr;

			const auto mono_image = domain_assembly->mono_image();
			if (!mono_image)
				return nullptr;

			const auto table_info = mono_image->get_table_info(2);
			if (!table_info)
				return nullptr;

			auto rows = table_info->get_rows();

			for (int i = 0; i < rows; i++) 
			{
				
				const auto ptr = static_cast<mono_hash_table_t*>(reinterpret_cast<void*>(mono_image + 0x4D0))->lookup<mono_class_t>(reinterpret_cast<void*>(0x02000000 | i + 1));

				if (!ptr)
					continue;

				auto name = ptr->name();

				if (!ptr->namespace_name().empty())
					name = ptr->namespace_name().append(".").append(ptr->name());

				cached_class_t cached_class{};
				cached_class.name = name;
				cached_class.klass = ptr;
				insertion_cache.classes.push_back(cached_class);

				//std::cout << "[+] found class : " << name << " : " << ptr << "\n";

				if (!strcmp(name.c_str(), class_name))
					ret = ptr;

			}

			cached_assemblies.push_back(insertion_cache);

			if (!ret)
			{

				invalid_classes = true;

				//std::cout << "invalid class: " << class_name << "\n";

			}

		}

#ifndef NO_SECURITY
		VIRTUALIZER_END;
#endif

		return ret;
	}

}