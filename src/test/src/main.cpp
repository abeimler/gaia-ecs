#include <gaia.h>

#if GAIA_COMPILER_MSVC
	#if _MSC_VER <= 1916
// warning C4100: 'XYZ': unreferenced formal parameter
GAIA_MSVC_WARNING_DISABLE(4100)
	#endif
#endif

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace gaia;

struct Int3 {
	uint32_t x, y, z;
};
struct Position {
	float x, y, z;
};
struct PositionSoA {
	float x, y, z;
	static constexpr auto Layout = mem::DataLayout::SoA;
};
struct PositionSoA8 {
	float x, y, z;
	static constexpr auto Layout = mem::DataLayout::SoA8;
};
struct PositionSoA16 {
	float x, y, z;
	static constexpr auto Layout = mem::DataLayout::SoA16;
};
struct Acceleration {
	float x, y, z;
};
struct Rotation {
	float x, y, z, w;
};
struct DummySoA {
	float x, y;
	bool b;
	float w;
	static constexpr auto Layout = mem::DataLayout::SoA;
};
struct RotationSoA {
	float x, y, z, w;
	static constexpr auto Layout = mem::DataLayout::SoA;
};
struct RotationSoA8 {
	float x, y, z, w;
	static constexpr auto Layout = mem::DataLayout::SoA8;
};
struct RotationSoA16 {
	float x, y, z, w;
	static constexpr auto Layout = mem::DataLayout::SoA16;
};
struct Scale {
	float x, y, z;
};
struct Something {
	bool value;
};
struct Else {
	bool value;
};
struct Empty {};

struct PositionNonTrivial {
	float x, y, z;
	PositionNonTrivial(): x(1), y(2), z(3) {}
	PositionNonTrivial(float xx, float yy, float zz): x(xx), y(yy), z(zz) {}
};

static constexpr const char* StringComponentDefaultValue =
		"StringComponentDefaultValue_ReasonablyLongSoThatItShouldCauseAHeapAllocationOnAllStdStringImplementations";
static constexpr const char* StringComponentDefaultValue_2 =
		"2_StringComponentDefaultValue_ReasonablyLongSoThatItShouldCauseAHeapAllocationOnAllStdStringImplementations";
static constexpr const char* StringComponent2DefaultValue =
		"StringComponent2DefaultValue_ReasonablyLongSoThatItShouldCauseAHeapAllocationOnAllStdStringImplementations";
static constexpr const char* StringComponent2DefaultValue_2 =
		"2_StringComponent2DefaultValue_ReasonablyLongSoThatItShouldCauseAHeapAllocationOnAllStdStringImplementations";
static constexpr const char* StringComponentEmptyValue =
		"StringComponentEmptyValue_ReasonablyLongSoThatItShouldCauseAHeapAllocationOnAllStdStringImplementations";

struct StringComponent {
	std::string value;
};
struct StringComponent2 {
	std::string value = StringComponent2DefaultValue;

	StringComponent2() = default;
	~StringComponent2() {
		value = StringComponentEmptyValue;
	}

	StringComponent2(const StringComponent2&) = default;
	StringComponent2(StringComponent2&&) noexcept = default;
	StringComponent2& operator=(const StringComponent2&) = default;
	StringComponent2& operator=(StringComponent2&&) noexcept = default;
};
GAIA_DEFINE_HAS_FUNCTION(foo)
struct Dummy0 {
	Dummy0* foo(const Dummy0&) const {
		return nullptr;
	}
};
inline bool operator==(const Dummy0&, const Dummy0&) {
	return true;
}
struct Dummy1 {
	bool operator==(const Dummy1&) const {
		return true;
	}
};

TEST_CASE("StringLookupKey") {
	constexpr uint32_t MaxLen = 32;
	char tmp0[MaxLen];
	char tmp1[MaxLen];
	GAIA_STRFMT(tmp0, MaxLen, "%s", "some string");
	GAIA_STRFMT(tmp1, MaxLen, "%s", "some string");
	core::StringLookupKey<128> l0(tmp0);
	core::StringLookupKey<128> l1(tmp1);
	REQUIRE(l0.len() == l1.len());
	// Two different addresses in memory have to return the same hash if the string is the same
	REQUIRE(l0.hash() == l1.hash());
}

TEST_CASE("has_XYZ_equals_check") {
	{
		constexpr auto hasMember = core::has_member_equals<Dummy0>::value;
		constexpr auto hasGlobal = core::has_global_equals<Dummy0>::value;
		// constexpr auto hasFoo = has_foo<Dummy0>::value;
		REQUIRE_FALSE(hasMember);
		REQUIRE(hasGlobal);
		// REQUIRE(hasFoo);
	}
	{
		constexpr auto hasMember = core::has_member_equals<Dummy1>::value;
		constexpr auto hasGlobal = core::has_global_equals<Dummy1>::value;
		// constexpr auto hasFoo = has_foo<Dummy1>::value;
		REQUIRE(hasMember);
		REQUIRE_FALSE(hasGlobal);
		// REQUIRE_FALSE(hasFoo);
	}
}

TEST_CASE("Intrinsics") {
	SECTION("POPCNT") {
		const uint32_t zero32 = GAIA_POPCNT(0);
		REQUIRE(zero32 == 0);
		const uint64_t zero64 = GAIA_POPCNT64(0);
		REQUIRE(zero64 == 0);

		const uint32_t val32 = GAIA_POPCNT(0x0003002);
		REQUIRE(val32 == 3);
		const uint64_t val64_1 = GAIA_POPCNT64(0x0003002);
		REQUIRE(val64_1 == 3);
		const uint64_t val64_2 = GAIA_POPCNT64(0x00030020000000);
		REQUIRE(val64_2 == 3);
		const uint64_t val64_3 = GAIA_POPCNT64(0x00030020003002);
		REQUIRE(val64_3 == 6);
	}
	SECTION("CLZ") {
		const uint32_t zero32 = GAIA_CLZ(0);
		REQUIRE(zero32 == 32);
		const uint64_t zero64 = GAIA_CLZ64(0);
		REQUIRE(zero64 == 64);

		const uint32_t val32 = GAIA_CLZ(0x0003002);
		REQUIRE(val32 == 1);
		const uint64_t val64_1 = GAIA_CLZ64(0x0003002);
		REQUIRE(val64_1 == 1);
		const uint64_t val64_2 = GAIA_CLZ64(0x00030020000000);
		REQUIRE(val64_2 == 29);
		const uint64_t val64_3 = GAIA_CLZ64(0x00030020003002);
		REQUIRE(val64_3 == 1);
	}
	SECTION("CTZ") {
		const uint32_t zero32 = GAIA_CTZ(0);
		REQUIRE(zero32 == 32);
		const uint64_t zero64 = GAIA_CTZ64(0);
		REQUIRE(zero64 == 64);

		const uint32_t val32 = GAIA_CTZ(0x0003002);
		REQUIRE(val32 == 18);
		const uint64_t val64_1 = GAIA_CTZ64(0x0003002);
		REQUIRE(val64_1 == 50);
		const uint64_t val64_2 = GAIA_CTZ64(0x00030020000000);
		REQUIRE(val64_2 == 22);
		const uint64_t val64_3 = GAIA_CTZ64(0x00030020003002);
		REQUIRE(val64_3 == 22);
	}
	SECTION("FFS") {
		const uint32_t zero32 = GAIA_FFS(0);
		REQUIRE(zero32 == 0);
		const uint64_t zero64 = GAIA_FFS64(0);
		REQUIRE(zero64 == 0);

		const uint32_t val32 = GAIA_FFS(0x0003002);
		REQUIRE(val32 == 2);
		const uint64_t val64_1 = GAIA_FFS64(0x0003002);
		REQUIRE(val64_1 == 2);
		const uint64_t val64_2 = GAIA_FFS64(0x00030020000000);
		REQUIRE(val64_2 == 30);
		const uint64_t val64_3 = GAIA_FFS64(0x00030020003002);
		REQUIRE(val64_3 == 2);
	}
}

TEST_CASE("EntityKinds") {
	REQUIRE(ecs::entity_kind_v<uint32_t> == ecs::EntityKind::EK_Gen);
	REQUIRE(ecs::entity_kind_v<Position> == ecs::EntityKind::EK_Gen);
	REQUIRE(ecs::entity_kind_v<ecs::uni<Position>> == ecs::EntityKind::EK_Uni);
}

TEST_CASE("Memory allocation") {
	SECTION("mem_alloc") {
		void* pMem = mem::mem_alloc(311);
		REQUIRE(pMem != nullptr);
		mem::mem_free(pMem);
	}
	SECTION("mem_alloc_alig A") {
		void* pMem = mem::mem_alloc_alig(1, 16);
		REQUIRE(pMem != nullptr);
		mem::mem_free_alig(pMem);
	}
	SECTION("mem_alloc_alig B") {
		void* pMem = mem::mem_alloc_alig(311, 16);
		REQUIRE(pMem != nullptr);
		mem::mem_free_alig(pMem);
	}
}

TEST_CASE("bit_view") {
	constexpr uint32_t BlockBits = 6;
	using view = core::bit_view<BlockBits>;
	// Number of BlockBits-sized elements
	constexpr uint32_t NBlocks = view::MaxValue + 1;
	// Number of bytes necessary to host "Blocks" number of BlockBits-sized elements
	constexpr uint32_t N = (BlockBits * NBlocks + 7) / 8;

	uint8_t arr[N]{};
	view bv{arr};

	GAIA_FOR(NBlocks) {
		bv.set(i * BlockBits, (uint8_t)i);

		const uint8_t val = bv.get(i * BlockBits);
		REQUIRE(val == i);
	}

	// Make sure nothing was overriden
	GAIA_FOR(NBlocks) {
		const uint8_t val = bv.get(i * BlockBits);
		REQUIRE(val == i);
	}
}

template <typename Container>
void fixed_arr_test() {
	constexpr auto N = Container::extent;
	static_assert(N > 2); // we need at least 2 items to complete this test
	Container arr;

	GAIA_FOR(N) {
		arr[i] = i;
		REQUIRE(arr[i] == i);
	}
	REQUIRE(arr.back() == N - 1);
	// Verify the values remain the same even after the internal buffer is reallocated
	GAIA_FOR(N) REQUIRE(arr[i] == i);
	// Copy assignment
	{
		Container arrCopy = arr;
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
	}
	// Copy constructor
	{
		Container arrCopy(arr);
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
	}
	// Move assignment
	{
		Container arrCopy = GAIA_MOV(arr);
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
		// move back
		arr = GAIA_MOV(arrCopy);
	}
	// Move constructor
	{
		Container arrCopy(GAIA_MOV(arr));
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
		// move back
		arr = GAIA_MOV(arrCopy);
	}

	// Container comparison
	{
		Container arrEmpty;
		REQUIRE_FALSE(arrEmpty == arr);

		Container arr2(arr);
		REQUIRE(arr2 == arr);
	}

	uint32_t cnt = 0;
	for (auto val: arr) {
		REQUIRE(val == cnt);
		++cnt;
	}
	REQUIRE(cnt == N);
	REQUIRE(cnt == arr.size());

	REQUIRE(core::find(arr, 0U) == arr.begin());
	REQUIRE(core::find(arr, N) == arr.end());

	REQUIRE(core::has(arr, 0U));
	REQUIRE_FALSE(core::has(arr, N));
}

TEST_CASE("Containers - sarr") {
	using arr = cnt::sarr<uint32_t, 100>;
	fixed_arr_test<arr>();
}

template <typename Container>
void resizable_arr_test(uint32_t N) {
	GAIA_ASSERT(N > 2); // we need at least 2 items to complete this test
	Container arr;

	GAIA_FOR(N) {
		arr.push_back(i);
		REQUIRE(arr[i] == i);
		REQUIRE(arr.back() == i);
	}
	// Verify the values remain the same even after the internal buffer is reallocated
	GAIA_FOR(N) REQUIRE(arr[i] == i);
	// Copy assignment
	{
		Container arrCopy = arr;
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
	}
	// Copy constructor
	{
		Container arrCopy(arr);
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
	}
	// Move assignment
	{
		Container arrCopy = GAIA_MOV(arr);
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
		// move back
		arr = GAIA_MOV(arrCopy);
	}
	// Move constructor
	{
		Container arrCopy(GAIA_MOV(arr));
		GAIA_FOR(N) REQUIRE(arrCopy[i] == i);
		// move back
		arr = GAIA_MOV(arrCopy);
	}

	// Container comparison
	{
		Container arrEmpty;
		REQUIRE_FALSE(arrEmpty == arr);

		Container arr2(arr);
		REQUIRE(arr2 == arr);
	}

	uint32_t cnt = 0;
	for (auto val: arr) {
		REQUIRE(val == cnt);
		++cnt;
	}
	REQUIRE(cnt == N);
	REQUIRE(cnt == arr.size());

	REQUIRE(core::find(arr, 0U) == arr.begin());
	REQUIRE(core::find(arr, N) == arr.end());

	REQUIRE(core::has(arr, 0U));
	REQUIRE_FALSE(core::has(arr, N));

	arr.erase(arr.begin());
	REQUIRE(arr.size() == (N - 1));
	REQUIRE(core::find(arr, 0U) == arr.end());
	GAIA_EACH(arr)
	REQUIRE(arr[i] == i + 1);

	arr.clear();
	REQUIRE(arr.empty());

	arr.push_back(11);
	arr.erase(arr.begin());
	REQUIRE(arr.empty());

	arr.push_back(11);
	arr.push_back(12);
	arr.push_back(13);
	arr.push_back(14);
	arr.push_back(15);
	arr.erase(arr.begin() + 1, arr.begin() + 4);
	REQUIRE(arr.size() == 2);
	REQUIRE(arr[0] == 11);
	REQUIRE(arr[1] == 15);

	arr.erase(arr.begin(), arr.end());
	REQUIRE(arr.empty());
}

TEST_CASE("Containers - sarr_ext") {
	using arr = cnt::sarr_ext<uint32_t, 100>;
	resizable_arr_test<arr>(100);
}

TEST_CASE("Containers - darr") {
	using arr = cnt::darr<uint32_t>;
	resizable_arr_test<arr>(100);
	resizable_arr_test<arr>(10000);
}

TEST_CASE("Containers - darr_ext") {
	using arr0 = cnt::darr_ext<uint32_t, 50>;
	resizable_arr_test<arr0>(100);
	resizable_arr_test<arr0>(10000);

	using arr1 = cnt::darr_ext<uint32_t, 100>;
	resizable_arr_test<arr1>(100);
	resizable_arr_test<arr1>(10000);
}

TEST_CASE("Containers - sringbuffer") {
	{
		cnt::sringbuffer<uint32_t, 5> arr = {0, 1, 2, 3, 4};
		uint32_t val{};

		REQUIRE_FALSE(arr.empty());
		REQUIRE(arr.front() == 0);
		REQUIRE(arr.back() == 4);

		arr.pop_front(val);
		REQUIRE(val == 0);
		REQUIRE_FALSE(arr.empty());
		REQUIRE(arr.front() == 1);
		REQUIRE(arr.back() == 4);

		arr.pop_front(val);
		REQUIRE(val == 1);
		REQUIRE_FALSE(arr.empty());
		REQUIRE(arr.front() == 2);
		REQUIRE(arr.back() == 4);

		arr.pop_front(val);
		REQUIRE(val == 2);
		REQUIRE_FALSE(arr.empty());
		REQUIRE(arr.front() == 3);
		REQUIRE(arr.back() == 4);

		arr.pop_back(val);
		REQUIRE(val == 4);
		REQUIRE_FALSE(arr.empty());
		REQUIRE(arr.front() == 3);
		REQUIRE(arr.back() == 3);

		arr.pop_back(val);
		REQUIRE(val == 3);
		REQUIRE(arr.empty());
	}

	{
		cnt::sringbuffer<uint32_t, 5> arr;
		uint32_t val{};

		REQUIRE(arr.empty());
		{
			arr.push_back(0);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 0);
			REQUIRE(arr.back() == 0);

			arr.push_back(1);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 0);
			REQUIRE(arr.back() == 1);

			arr.push_back(2);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 0);
			REQUIRE(arr.back() == 2);

			arr.push_back(3);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 0);
			REQUIRE(arr.back() == 3);

			arr.push_back(4);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 0);
			REQUIRE(arr.back() == 4);
		}
		{
			arr.pop_front(val);
			REQUIRE(val == 0);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 1);
			REQUIRE(arr.back() == 4);

			arr.pop_front(val);
			REQUIRE(val == 1);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 2);
			REQUIRE(arr.back() == 4);

			arr.pop_front(val);
			REQUIRE(val == 2);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 3);
			REQUIRE(arr.back() == 4);

			arr.pop_back(val);
			REQUIRE(val == 4);
			REQUIRE_FALSE(arr.empty());
			REQUIRE(arr.front() == 3);
			REQUIRE(arr.back() == 3);

			arr.pop_back(val);
			REQUIRE(val == 3);
			REQUIRE(arr.empty());
		}
	}
}

TEST_CASE("Containers - ilist") {
	struct EntityContainer: cnt::ilist_item {
		int value;

		EntityContainer() = default;
		EntityContainer(uint32_t index, uint32_t generation): cnt::ilist_item(index, generation) {}
	};
	SECTION("0 -> 1 -> 2") {
		cnt::ilist<EntityContainer, ecs::Entity> il;
		ecs::Entity handles[3];

		handles[0] = il.alloc();
		il[handles[0].id()].value = 100;
		REQUIRE(handles[0].id() == 0);
		REQUIRE(il[0].idx == 0);
		REQUIRE(handles[0].gen() == il[0].gen);
		REQUIRE(il[0].gen == 0);
		handles[1] = il.alloc();
		il[handles[1].id()].value = 200;
		REQUIRE(handles[1].id() == 1);
		REQUIRE(il[1].idx == 1);
		REQUIRE(handles[1].gen() == il[1].gen);
		REQUIRE(il[1].gen == 0);
		handles[2] = il.alloc();
		il[handles[2].id()].value = 300;
		REQUIRE(handles[2].id() == 2);
		REQUIRE(il[2].idx == 2);
		REQUIRE(handles[2].gen() == il[2].gen);
		REQUIRE(il[2].gen == 0);

		il.free(handles[2]);
		REQUIRE(il[2].idx == ecs::Entity::IdMask);
		REQUIRE(il[2].gen == 1);
		il.free(handles[1]);
		REQUIRE(il[1].idx == 2);
		REQUIRE(il[1].gen == 1);
		il.free(handles[0]);
		REQUIRE(il[0].idx == 1);
		REQUIRE(il[0].gen == 1);

		handles[0] = il.alloc();
		REQUIRE(handles[0].id() == 0);
		REQUIRE(il[0].value == 100);
		REQUIRE(il[0].idx == 1);
		REQUIRE(handles[0].gen() == il[0].gen);
		REQUIRE(il[0].gen == 1);
		handles[1] = il.alloc();
		REQUIRE(handles[1].id() == 1);
		REQUIRE(il[1].value == 200);
		REQUIRE(il[1].idx == 2);
		REQUIRE(handles[1].gen() == il[1].gen);
		REQUIRE(il[1].gen == 1);
		handles[2] = il.alloc();
		REQUIRE(handles[2].id() == 2);
		REQUIRE(il[2].value == 300);
		REQUIRE(il[2].idx == ecs::Entity::IdMask);
		REQUIRE(handles[2].gen() == il[2].gen);
		REQUIRE(il[2].gen == 1);
	}
	SECTION("2 -> 1 -> 0") {
		cnt::ilist<EntityContainer, ecs::Entity> il;
		ecs::Entity handles[3];

		handles[0] = il.alloc();
		il[handles[0].id()].value = 100;
		REQUIRE(handles[0].id() == 0);
		REQUIRE(il[0].idx == 0);
		REQUIRE(handles[0].gen() == il[0].gen);
		REQUIRE(il[0].gen == 0);
		handles[1] = il.alloc();
		il[handles[1].id()].value = 200;
		REQUIRE(handles[1].id() == 1);
		REQUIRE(il[1].idx == 1);
		REQUIRE(handles[1].gen() == il[1].gen);
		REQUIRE(il[1].gen == 0);
		handles[2] = il.alloc();
		il[handles[2].id()].value = 300;
		REQUIRE(handles[2].id() == 2);
		REQUIRE(il[2].idx == 2);
		REQUIRE(handles[2].gen() == il[2].gen);
		REQUIRE(il[2].gen == 0);

		il.free(handles[0]);
		REQUIRE(il[0].idx == ecs::Entity::IdMask);
		REQUIRE(il[0].gen == 1);
		il.free(handles[1]);
		REQUIRE(il[1].idx == 0);
		REQUIRE(il[1].gen == 1);
		il.free(handles[2]);
		REQUIRE(il[2].idx == 1);
		REQUIRE(il[2].gen == 1);

		handles[0] = il.alloc();
		REQUIRE(handles[0].id() == 2);
		const auto idx0 = handles[0].id();
		REQUIRE(il[idx0].value == 300);
		REQUIRE(il[idx0].idx == 1);
		REQUIRE(handles[0].gen() == il[idx0].gen);
		REQUIRE(il[idx0].gen == 1);
		handles[1] = il.alloc();
		REQUIRE(handles[1].id() == 1);
		const auto idx1 = handles[1].id();
		REQUIRE(il[idx1].value == 200);
		REQUIRE(il[idx1].idx == 0);
		REQUIRE(handles[1].gen() == il[idx1].gen);
		REQUIRE(il[idx1].gen == 1);
		handles[2] = il.alloc();
		REQUIRE(handles[2].id() == 0);
		const auto idx2 = handles[2].id();
		REQUIRE(il[idx2].value == 100);
		REQUIRE(il[idx2].idx == ecs::Entity::IdMask);
		REQUIRE(handles[2].gen() == il[idx2].gen);
		REQUIRE(il[idx2].gen == 1);
	}
}

template <uint32_t NBits>
void test_bitset() {
	// Following tests expect at least 5 bits of space
	static_assert(NBits >= 5);

	SECTION("Bit operations") {
		cnt::bitset<NBits> bs;
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.size() == NBits);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.set(0, true);
		REQUIRE(bs.test(0) == true);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.set(1, true);
		REQUIRE(bs.test(1) == true);
		REQUIRE(bs.count() == 2);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.set(1, false);
		REQUIRE(bs.test(1) == false);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.flip(1);
		REQUIRE(bs.test(1) == true);
		REQUIRE(bs.count() == 2);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.flip(1);
		REQUIRE(bs.test(1) == false);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.reset(0);
		REQUIRE(bs.test(0) == false);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.set();
		REQUIRE(bs.count() == NBits);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.flip();
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.flip();
		REQUIRE(bs.count() == NBits);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.reset();
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);
	}
	SECTION("Iteration") {
		{
			cnt::bitset<NBits> bs;
			uint32_t i = 0;
			for ([[maybe_unused]] auto val: bs)
				++i;
			REQUIRE(i == 0);
		}
		auto fwd_iterator_test = [](std::span<uint32_t> vals) {
			cnt::bitset<NBits> bs;
			for (uint32_t bit: vals)
				bs.set(bit);
			const uint32_t c = bs.count();

			uint32_t i = 0;
			for (auto val: bs) {
				REQUIRE(vals[i] == val);
				++i;
			}
			REQUIRE(i == c);
			REQUIRE(i == (uint32_t)vals.size());
		};
		{
			uint32_t vals[]{1, 2, 3};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 2, 3};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{1, 3, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{1, NBits - 2, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 1, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 3, NBits - 1};
			fwd_iterator_test(vals);
		}
	}
}

TEST_CASE("Containers - bitset") {
	SECTION("5 bits") {
		test_bitset<5>();
	}
	SECTION("11 bits") {
		test_bitset<11>();
	}
	SECTION("32 bits") {
		test_bitset<32>();
	}
	SECTION("33 bits") {
		test_bitset<33>();
	}
	SECTION("64 bits") {
		test_bitset<64>();
	}
	SECTION("90 bits") {
		test_bitset<90>();
	}
	SECTION("128 bits") {
		test_bitset<128>();
	}
	SECTION("150 bits") {
		test_bitset<150>();
	}
	SECTION("512 bits") {
		test_bitset<512>();
	}
	SECTION("Ranges 11 bits") {
		cnt::bitset<11> bs;
		bs.set(1);
		bs.set(10);
		bs.flip(2, 9);
		for (uint32_t i = 1; i <= 10; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 9);
		for (uint32_t i = 2; i < 10; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(10));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(10, 10);
		REQUIRE(bs.test(10));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 10);
		REQUIRE(bs.count() == 11);
		REQUIRE(bs.all() == true);
		bs.flip(0, 10);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
	SECTION("Ranges 64 bits") {
		cnt::bitset<64> bs;
		bs.set(1);
		bs.set(10);
		bs.flip(2, 9);
		for (uint32_t i = 1; i <= 10; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 9);
		for (uint32_t i = 2; i < 10; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(10));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(63, 63);
		REQUIRE(bs.test(63));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 63);
		REQUIRE(bs.count() == 64);
		REQUIRE(bs.all() == true);
		bs.flip(0, 63);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
	SECTION("Ranges 101 bits") {
		cnt::bitset<101> bs;
		bs.set(1);
		bs.set(100);
		bs.flip(2, 99);
		for (uint32_t i = 1; i <= 100; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 99);
		for (uint32_t i = 2; i < 100; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(100));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(100, 100);
		REQUIRE(bs.test(100));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 100);
		REQUIRE(bs.count() == 101);
		REQUIRE(bs.all() == true);
		bs.flip(0, 100);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
}

template <uint32_t NBits>
void test_dbitset() {
	// Following tests expect at least 5 bits of space
	static_assert(NBits >= 5);

	SECTION("Bit operations") {
		cnt::dbitset bs;
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.size() == 1);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.set(0, true);
		REQUIRE(bs.test(0) == true);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.set(1, true);
		REQUIRE(bs.test(1) == true);
		REQUIRE(bs.count() == 2);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.set(1, false);
		REQUIRE(bs.test(1) == false);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.flip(1);
		REQUIRE(bs.test(1) == true);
		REQUIRE(bs.count() == 2);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.flip(1);
		REQUIRE(bs.test(1) == false);
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == false);

		bs.reset(0);
		REQUIRE(bs.test(0) == false);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.set();
		REQUIRE(bs.count() == bs.size());
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.flip();
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);

		bs.flip();
		REQUIRE(bs.count() == bs.size());
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == true);
		REQUIRE(bs.none() == false);

		bs.reset();
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.any() == false);
		REQUIRE(bs.all() == false);
		REQUIRE(bs.none() == true);
	}
	SECTION("Iteration") {
		{
			cnt::dbitset bs;
			uint32_t i = 0;
			for ([[maybe_unused]] auto val: bs)
				++i;
			REQUIRE(i == 0);
		}
		auto fwd_iterator_test = [](std::span<uint32_t> vals) {
			cnt::dbitset bs;
			for (uint32_t bit: vals)
				bs.set(bit);
			const uint32_t c = bs.count();

			uint32_t i = 0;
			for (auto val: bs) {
				REQUIRE(vals[i] == val);
				++i;
			}
			REQUIRE(i == c);
			REQUIRE(i == (uint32_t)vals.size());
		};
		{
			uint32_t vals[]{1, 2, 3};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 2, 3};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{1, 3, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{1, NBits - 2, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 1, NBits - 1};
			fwd_iterator_test(vals);
		}
		{
			uint32_t vals[]{0, 3, NBits - 1};
			fwd_iterator_test(vals);
		}
	}
}

TEST_CASE("Containers - dbitset") {
	SECTION("5 bits") {
		test_dbitset<5>();
	}
	SECTION("11 bits") {
		test_dbitset<11>();
	}
	SECTION("32 bits") {
		test_dbitset<32>();
	}
	SECTION("33 bits") {
		test_dbitset<33>();
	}
	SECTION("64 bits") {
		test_dbitset<64>();
	}
	SECTION("90 bits") {
		test_dbitset<90>();
	}
	SECTION("128 bits") {
		test_dbitset<128>();
	}
	SECTION("150 bits") {
		test_dbitset<150>();
	}
	SECTION("512 bits") {
		test_dbitset<512>();
	}
	SECTION("Ranges 11 bits") {
		cnt::dbitset bs;
		bs.resize(11);

		bs.set(1);
		bs.set(10);
		bs.flip(2, 9);
		for (uint32_t i = 1; i <= 10; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 9);
		for (uint32_t i = 2; i < 10; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(10));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(10, 10);
		REQUIRE(bs.test(10));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 10);
		REQUIRE(bs.count() == 11);
		REQUIRE(bs.all() == true);
		bs.flip(0, 10);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
	SECTION("Ranges 64 bits") {
		cnt::dbitset bs;
		bs.resize(64);

		bs.set(1);
		bs.set(10);
		bs.flip(2, 9);
		for (uint32_t i = 1; i <= 10; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 9);
		for (uint32_t i = 2; i < 10; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(10));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(63, 63);
		REQUIRE(bs.test(63));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 63);
		REQUIRE(bs.count() == 64);
		REQUIRE(bs.all() == true);
		bs.flip(0, 63);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
	SECTION("Ranges 101 bits") {
		cnt::dbitset bs;
		bs.resize(101);

		bs.set(1);
		bs.set(100);
		bs.flip(2, 99);
		for (uint32_t i = 1; i <= 100; ++i)
			REQUIRE(bs.test(i) == true);
		bs.flip(2, 99);
		for (uint32_t i = 2; i < 100; ++i)
			REQUIRE(bs.test(i) == false);
		REQUIRE(bs.test(1));
		REQUIRE(bs.test(100));

		bs.reset();
		bs.flip(0, 0);
		REQUIRE(bs.test(0));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(100, 100);
		REQUIRE(bs.test(100));
		REQUIRE(bs.count() == 1);
		REQUIRE(bs.any() == true);
		REQUIRE(bs.all() == false);

		bs.reset();
		bs.flip(0, 100);
		REQUIRE(bs.count() == 101);
		REQUIRE(bs.all() == true);
		bs.flip(0, 100);
		REQUIRE(bs.count() == 0);
		REQUIRE(bs.none() == true);
	}
}

TEST_CASE("each") {
	constexpr uint32_t N = 10;
	SECTION("index agument") {
		uint32_t cnt = 0;
		core::each<N>([&cnt](auto i) {
			cnt += i;
		});
		REQUIRE(cnt == 0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9);
	}
	SECTION("no index agument") {
		uint32_t cnt = 0;
		core::each<N>([&cnt]() {
			++cnt;
		});
		REQUIRE(cnt == N);
	}
}

TEST_CASE("each_ext") {
	constexpr uint32_t N = 10;
	SECTION("index agument") {
		uint32_t cnt = 0;
		core::each_ext<2, N - 1, 2>([&cnt](auto i) {
			cnt += i;
		});
		REQUIRE(cnt == 2 + 4 + 6 + 8);
	}
	SECTION("no index agument") {
		uint32_t cnt = 0;
		core::each_ext<2, N - 1, 2>([&cnt]() {
			++cnt;
		});
		REQUIRE(cnt == 4);
	}
}

TEST_CASE("each_tuple") {
	SECTION("func(Args)") {
		uint32_t val = 0;
		core::each_tuple(std::make_tuple(69, 10, 20), [&val](const auto& value) {
			val += value;
		});
		REQUIRE(val == 99);
	}
	SECTION("func(Args, iter)") {
		uint32_t val = 0;
		uint32_t iter = 0;
		core::each_tuple(std::make_tuple(69, 10, 20), [&](const auto& value, uint32_t i) {
			val += value;
			REQUIRE(i == iter);
			++iter;
		});
		REQUIRE(val == 99);
	}
}

TEST_CASE("each_tuple_ext") {
	SECTION("func(Args)") {
		uint32_t val = 0;
		core::each_tuple_ext<1, 3>(std::make_tuple(69, 10, 20), [&val](const auto& value) {
			val += value;
		});
		REQUIRE(val == 30);
	}
	SECTION("func(Args, iter)") {
		uint32_t val = 0;
		uint32_t iter = 1;
		core::each_tuple_ext<1, 3>(std::make_tuple(69, 10, 20), [&](const auto& value, uint32_t i) {
			val += value;
			REQUIRE(i == iter);
			++iter;
		});
		REQUIRE(val == 30);
	}
}

TEST_CASE("each_tuple2") {
	SECTION("func(Args)") {
		uint32_t val = 0;
		using TTuple = std::tuple<int, int, double>;
		core::each_tuple<TTuple>([&val](auto&& item) {
			val += sizeof(item);
		});
		REQUIRE(val == 16);
	}
	SECTION("func(Args, iter)") {
		uint32_t val = 0;
		uint32_t iter = 0;
		using TTuple = std::tuple<int, int, double>;
		core::each_tuple<TTuple>([&](auto&& item, uint32_t i) {
			val += sizeof(item);
			REQUIRE(i == iter);
			++iter;
		});
		REQUIRE(val == 16);
	}
}

TEST_CASE("each_tuple_ext2") {
	SECTION("func(Args)") {
		uint32_t val = 0;
		using TTuple = std::tuple<int, int, double>;
		core::each_tuple_ext<1, 3, TTuple>([&val](auto&& item) {
			val += sizeof(item);
		});
		REQUIRE(val == 12);
	}
	SECTION("func(Args, iter)") {
		uint32_t val = 0;
		uint32_t iter = 1;
		using TTuple = std::tuple<int, int, double>;
		core::each_tuple_ext<1, 3, TTuple>([&](auto&& item, uint32_t i) {
			val += sizeof(item);
			REQUIRE(i == iter);
			++iter;
		});
		REQUIRE(val == 12);
	}
}

TEST_CASE("each_pack") {
	uint32_t val = 0;
	core::each_pack(
			[&val](const auto& value) {
				val += value;
			},
			69, 10, 20);
	REQUIRE(val == 99);
}

template <bool IsRuntime, typename C>
void sort_descending(C arr) {
	using TValue = typename C::value_type;

	for (TValue i = 0; i < (TValue)arr.size(); ++i)
		arr[i] = i;
	if constexpr (IsRuntime)
		core::sort(arr, core::is_greater<TValue>());
	else
		core::sort_ct(arr, core::is_greater<TValue>());
	for (uint32_t i = 1; i < arr.size(); ++i)
		REQUIRE(arr[i - 1] > arr[i]);
}

template <bool IsRuntime, typename C>
void sort_ascending(C arr) {
	using TValue = typename C::value_type;

	for (TValue i = 0; i < (TValue)arr.size(); ++i)
		arr[i] = i;
	if constexpr (IsRuntime)
		core::sort(arr, core::is_smaller<TValue>());
	else
		core::sort_ct(arr, core::is_smaller<TValue>());
	for (uint32_t i = 1; i < arr.size(); ++i)
		REQUIRE(arr[i - 1] < arr[i]);
}

TEST_CASE("Compile-time sort descending") {
	sort_descending<false>(cnt::sarray<uint32_t, 2>{});
	sort_descending<false>(cnt::sarray<uint32_t, 3>{});
	sort_descending<false>(cnt::sarray<uint32_t, 4>{});
	sort_descending<false>(cnt::sarray<uint32_t, 5>{});
	sort_descending<false>(cnt::sarray<uint32_t, 6>{});
	sort_descending<false>(cnt::sarray<uint32_t, 7>{});
	sort_descending<false>(cnt::sarray<uint32_t, 8>{});
	sort_descending<false>(cnt::sarray<uint32_t, 9>{});
	sort_descending<false>(cnt::sarray<uint32_t, 10>{});
	sort_descending<false>(cnt::sarray<uint32_t, 11>{});
	sort_descending<false>(cnt::sarray<uint32_t, 12>{});
	sort_descending<false>(cnt::sarray<uint32_t, 13>{});
	sort_descending<false>(cnt::sarray<uint32_t, 14>{});
	sort_descending<false>(cnt::sarray<uint32_t, 15>{});
	sort_descending<false>(cnt::sarray<uint32_t, 16>{});
	sort_descending<false>(cnt::sarray<uint32_t, 17>{});
	sort_descending<false>(cnt::sarray<uint32_t, 18>{});
}

TEST_CASE("Compile-time sort ascending") {
	sort_ascending<false>(cnt::sarray<uint32_t, 2>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 3>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 4>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 5>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 6>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 7>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 8>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 9>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 10>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 11>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 12>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 13>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 14>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 15>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 16>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 17>{});
	sort_ascending<false>(cnt::sarray<uint32_t, 18>{});
}

TEST_CASE("Run-time sort - sorting network") {
	sort_descending<true>(cnt::sarray<uint32_t, 5>{});
	sort_ascending<true>(cnt::sarray<uint32_t, 5>{});
}

TEST_CASE("Run-time sort - bubble sort") {
	sort_descending<true>(cnt::sarray<uint32_t, 15>{});
	sort_ascending<true>(cnt::sarray<uint32_t, 15>{});
}

TEST_CASE("Run-time sort - quick sort") {
	sort_descending<true>(cnt::sarray<uint32_t, 45>{});
	sort_ascending<true>(cnt::sarray<uint32_t, 45>{});
}

template <typename T>
void TestDataLayoutAoS() {
	constexpr uint32_t N = 100;
	cnt::sarray<T, N> data;

	GAIA_FOR(N) {
		const auto f = (float)(i + 1);
		data[i] = {f, f, f};

		auto val = data[i];
		REQUIRE(val.x == f);
		REQUIRE(val.y == f);
		REQUIRE(val.z == f);
	}

	SECTION("Make sure that all values are correct (e.g. that they were not overriden by one of the loop iteration)") {
		GAIA_FOR(N) {
			const auto f = (float)(i + 1);

			auto val = data[i];
			REQUIRE(val.x == f);
			REQUIRE(val.y == f);
			REQUIRE(val.z == f);
		}
	}
}

TEST_CASE("DataLayout AoS") {
	TestDataLayoutAoS<Position>();
	TestDataLayoutAoS<Rotation>();
}

template <typename T>
void TestDataLayoutSoA() {
	constexpr uint32_t N = 100;
	cnt::sarray<T, N> data;

	GAIA_FOR(N) {
		const float f[] = {(float)(i + 1), (float)(i + 2), (float)(i + 3)};
		data[i] = {f[0], f[1], f[2]};

		T val = data[i];
		REQUIRE(val.x == f[0]);
		REQUIRE(val.y == f[1]);
		REQUIRE(val.z == f[2]);

		const float ff[] = {data.template soa_view<0>()[i], data.template soa_view<1>()[i], data.template soa_view<2>()[i]};
		REQUIRE(ff[0] == f[0]);
		REQUIRE(ff[1] == f[1]);
		REQUIRE(ff[2] == f[2]);
	}

	// Make sure that all values are correct (e.g. that they were not overriden by one of the loop iteration)
	GAIA_FOR(N) {
		const float f[] = {(float)(i + 1), (float)(i + 2), (float)(i + 3)};
		T val = data[i];
		REQUIRE(val.x == f[0]);
		REQUIRE(val.y == f[1]);
		REQUIRE(val.z == f[2]);

		const float ff[] = {data.template soa_view<0>()[i], data.template soa_view<1>()[i], data.template soa_view<2>()[i]};
		REQUIRE(ff[0] == f[0]);
		REQUIRE(ff[1] == f[1]);
		REQUIRE(ff[2] == f[2]);
	}
}

template <>
void TestDataLayoutSoA<DummySoA>() {
	constexpr uint32_t N = 100;
	cnt::sarray<DummySoA, N> data{};

	GAIA_FOR(N) {
		float f[] = {(float)(i + 1), (float)(i + 2), (float)(i + 3)};
		data[i] = {f[0], f[1], true, f[2]};

		DummySoA val = data[i];
		REQUIRE(val.x == f[0]);
		REQUIRE(val.y == f[1]);
		REQUIRE(val.b == true);
		REQUIRE(val.w == f[2]);

		const float ff[] = {data.template soa_view<0>()[i], data.template soa_view<1>()[i], data.template soa_view<3>()[i]};
		const bool b = data.template soa_view<2>()[i];
		REQUIRE(ff[0] == f[0]);
		REQUIRE(ff[1] == f[1]);
		REQUIRE(b == true);
		REQUIRE(ff[2] == f[2]);
	}

	// Make sure that all values are correct (e.g. that they were not overriden by one of the loop iteration)
	GAIA_FOR(N) {
		const float f[] = {(float)(i + 1), (float)(i + 2), (float)(i + 3)};

		DummySoA val = data[i];
		REQUIRE(val.x == f[0]);
		REQUIRE(val.y == f[1]);
		REQUIRE(val.b == true);
		REQUIRE(val.w == f[2]);

		const float ff[] = {data.template soa_view<0>()[i], data.template soa_view<1>()[i], data.template soa_view<3>()[i]};
		const bool b = data.template soa_view<2>()[i];
		REQUIRE(ff[0] == f[0]);
		REQUIRE(ff[1] == f[1]);
		REQUIRE(b == true);
		REQUIRE(ff[2] == f[2]);
	}
}

TEST_CASE("DataLayout SoA") {
	TestDataLayoutSoA<PositionSoA>();
	TestDataLayoutSoA<RotationSoA>();
	TestDataLayoutSoA<DummySoA>();
}

TEST_CASE("DataLayout SoA8") {
	TestDataLayoutSoA<PositionSoA8>();
	TestDataLayoutSoA<RotationSoA8>();
}

TEST_CASE("DataLayout SoA16") {
	TestDataLayoutSoA<PositionSoA16>();
	TestDataLayoutSoA<RotationSoA16>();
}

TEST_CASE("Entity - valid") {
	ecs::World w;
	GAIA_FOR(2) {
		auto e = w.add();
		REQUIRE(w.valid(e));
		w.del(e);
		REQUIRE_FALSE(w.valid(e));
	}
}

TEST_CASE("Entity - has") {
	ecs::World w;
	GAIA_FOR(2) {
		auto e = w.add();
		REQUIRE(w.has(e));
		w.del(e);
		REQUIRE_FALSE(w.has(e));
	}
}

TEST_CASE("Entity - EntityBad") {
	REQUIRE(ecs::Entity{} == ecs::EntityBad);

	ecs::World w;
	REQUIRE_FALSE(w.valid(ecs::EntityBad));

	auto e = w.add();
	REQUIRE(e != ecs::EntityBad);
	REQUIRE_FALSE(e == ecs::EntityBad);
	REQUIRE(e.entity());
}

TEST_CASE("Add - no components") {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> ents, arr;
	ents.reserve(N);
	arr.reserve(N);

	auto create = [&]() {
		auto e = w.add();
		ents.push_back(e);
	};
	auto verify = [&](uint32_t i) {
		REQUIRE(arr[i + 3] == ents[i]);
	};

	GAIA_FOR(N) create();

	auto q = w.query().all<ecs::EntityDesc>().no<ecs::Component>();
	q.arr(arr);
	REQUIRE(arr.size() - 3 == ents.size()); // 3 for core component

	// GAIA_FOR(N) verify(i);
}

TEST_CASE("Add - 1 component") {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> ents;
	ents.reserve(N);

	auto create = [&](uint32_t i) {
		auto e = w.add();
		ents.push_back(e);
		w.add<Int3>(e, {i, i, i});
	};
	auto verify = [&](uint32_t i) {
		auto e = ents[i];

		REQUIRE(w.has<Int3>(e));
		auto val = w.get<Int3>(e);
		REQUIRE(val.x == i);
		REQUIRE(val.y == i);
		REQUIRE(val.z == i);
	};

	GAIA_FOR(N) create(i);
	GAIA_FOR(N) verify(i);

	{
		const auto& p = w.add<Int3>();
		auto e = w.add();
		w.add(e, p.entity, Int3{1, 2, 3});

		REQUIRE(w.has(e, p.entity));
		REQUIRE(w.has<Int3>(e));
		auto val0 = w.get<Int3>(e);
		REQUIRE(val0.x == 1);
		REQUIRE(val0.y == 2);
		REQUIRE(val0.z == 3);
	}
}

namespace dummy {
	struct Position {
		float x;
		float y;
		float z;
	};
} // namespace dummy

TEST_CASE("Add - namespaces") {
	ecs::World w;
	auto e = w.add();
	w.add<Position>(e, {1, 1, 1});
	w.add<dummy::Position>(e, {2, 2, 2});
	auto e2 = w.add();
	w.add<Position>(e2);
	auto a3 = w.add();
	w.add<dummy::Position>(a3);
	auto a4 = w.copy(a3);

	REQUIRE(w.has<Position>(e));
	REQUIRE(w.has<dummy::Position>(e));

	{
		auto p1 = w.get<Position>(e);
		REQUIRE(p1.x == 1.f);
		REQUIRE(p1.y == 1.f);
		REQUIRE(p1.z == 1.f);
	}
	{
		auto p2 = w.get<dummy::Position>(e);
		REQUIRE(p2.x == 2.f);
		REQUIRE(p2.y == 2.f);
		REQUIRE(p2.z == 2.f);
	}
	{
		auto q1 = w.query().all<Position>();
		const auto c1 = q1.count();
		REQUIRE(c1 == 2);

		auto q2 = w.query().all<dummy::Position>();
		const auto c2 = q2.count();
		REQUIRE(c2 == 3);

		auto q3 = w.query().no<Position>();
		const auto c3 = q3.count();
		REQUIRE(c3 > 0); // It's going to be a bunch

		auto q4 = w.query().all<dummy::Position>().no<Position>();
		const auto c4 = q4.count();
		REQUIRE(c4 == 2);
	}
}

TEST_CASE("Add - many components") {
	ecs::World w;

	auto create = [&]() {
		auto e = w.add();

		w.add<Int3>(e, {3, 3, 3});
		w.add<Position>(e, {1, 1, 1});
		w.add<Empty>(e);
		w.add<Else>(e, {true});
		w.add<Rotation>(e, {2, 2, 2, 2});
		w.add<Scale>(e, {4, 4, 4});

		REQUIRE(w.has<Int3>(e));
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Empty>(e));
		REQUIRE(w.has<Rotation>(e));
		REQUIRE(w.has<Scale>(e));

		{
			auto val = w.get<Int3>(e);
			REQUIRE(val.x == 3);
			REQUIRE(val.y == 3);
			REQUIRE(val.z == 3);
		}
		{
			auto val = w.get<Position>(e);
			REQUIRE(val.x == 1.f);
			REQUIRE(val.y == 1.f);
			REQUIRE(val.z == 1.f);
		}
		{
			auto val = w.get<Rotation>(e);
			REQUIRE(val.x == 2.f);
			REQUIRE(val.y == 2.f);
			REQUIRE(val.z == 2.f);
			REQUIRE(val.w == 2.f);
		}
		{
			auto val = w.get<Scale>(e);
			REQUIRE(val.x == 4.f);
			REQUIRE(val.y == 4.f);
			REQUIRE(val.z == 4.f);
		}
	};

	const uint32_t N = 1'500;
	GAIA_FOR(N) create();
}

TEST_CASE("Add - many components, bulk") {
	ecs::World w;

	auto create = [&]() {
		auto e = w.add();

		w.bulk(e).add<Int3, Position, Empty>().add<Else>().add<Rotation>().add<Scale>();

		w.set(e)
				.set<Int3>({3, 3, 3})
				.set<Position>({1, 1, 1})
				.set<Else>({true})
				.set<Rotation>({2, 2, 2, 2})
				.set<Scale>({4, 4, 4});

		REQUIRE(w.has<Int3>(e));
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Empty>(e));
		REQUIRE(w.has<Rotation>(e));
		REQUIRE(w.has<Scale>(e));

		{
			auto val = w.get<Int3>(e);
			REQUIRE(val.x == 3);
			REQUIRE(val.y == 3);
			REQUIRE(val.z == 3);
		}
		{
			auto val = w.get<Position>(e);
			REQUIRE(val.x == 1.f);
			REQUIRE(val.y == 1.f);
			REQUIRE(val.z == 1.f);
		}
		{
			auto val = w.get<Rotation>(e);
			REQUIRE(val.x == 2.f);
			REQUIRE(val.y == 2.f);
			REQUIRE(val.z == 2.f);
			REQUIRE(val.w == 2.f);
		}
		{
			auto val = w.get<Scale>(e);
			REQUIRE(val.x == 4.f);
			REQUIRE(val.y == 4.f);
			REQUIRE(val.z == 4.f);
		}
	};

	const uint32_t N = 1'500;
	GAIA_FOR(N) create();
}

TEST_CASE("Pair") {
	{
		ecs::World w;
		auto a = w.add();
		auto b = w.add();
		auto p = ecs::Pair(a, b);
		REQUIRE(p.first() == a);
		REQUIRE(p.second() == b);
		auto pe = (ecs::Entity)p;
		REQUIRE(w.get(pe.id()) == a);
		REQUIRE(w.get(pe.gen()) == b);
	}
	{
		ecs::World w;
		auto a = w.add<Position>().entity;
		auto b = w.add<ecs::DependsOn_>().entity;
		auto p = ecs::Pair(a, b);
		REQUIRE(ecs::is_pair<decltype(p)>::value);
		REQUIRE(p.first() == a);
		REQUIRE(p.second() == b);
		auto pe = (ecs::Entity)p;
		REQUIRE_FALSE(ecs::is_pair<decltype(pe)>::value);
		REQUIRE(w.get(pe.id()) == a);
		REQUIRE(w.get(pe.gen()) == b);
	}
	struct Start {};
	struct Stop {};
	{
		REQUIRE(ecs::is_pair<ecs::pair<Start, Position>>::value);
		REQUIRE(ecs::is_pair<ecs::pair<Position, Start>>::value);

		using Pair1 = ecs::pair<Start, Position>;
		static_assert(std::is_same_v<Pair1::rel, Start>);
		static_assert(std::is_same_v<Pair1::tgt, Position>);
		using Pair1Actual = ecs::actual_type_t<Pair1>;
		static_assert(std::is_same_v<Pair1Actual::Type, Position>);

		using Pair2 = ecs::pair<Start, ecs::uni<Position>>;
		static_assert(std::is_same_v<Pair2::rel, Start>);
		static_assert(std::is_same_v<Pair2::tgt, ecs::uni<Position>>);
		using Pair2Actual = ecs::actual_type_t<Pair2>;
		static_assert(std::is_same_v<Pair2Actual::Type, Position>);
		static_assert(std::is_same_v<Pair2Actual::TypeFull, ecs::uni<Position>>);
	}
	{
		ecs::World w;
		auto eStart = w.add<Start>().entity;
		auto eStop = w.add<Stop>().entity;
		auto ePos = w.add<Position>().entity;
		auto e = w.add();

		w.add<Position>(e, {5, 5, 5});
		w.add(e, ecs::Pair(eStart, ePos));
		w.add(e, ecs::Pair(eStop, ePos));
		auto p = w.get<Position>(e);
		REQUIRE(p.x == 5);
		REQUIRE(p.y == 5);
		REQUIRE(p.z == 5);

		w.add<ecs::pair<Start, ecs::uni<Position>>>(e, {50, 50, 50});
		auto spu = w.get<ecs::pair<Start, ecs::uni<Position>>>(e);
		REQUIRE(spu.x == 50);
		REQUIRE(spu.y == 50);
		REQUIRE(spu.z == 50);
		REQUIRE(p.x == 5);
		REQUIRE(p.y == 5);
		REQUIRE(p.z == 5);

		w.add<ecs::pair<Start, Position>>(e, {100, 100, 100});
		auto sp = w.get<ecs::pair<Start, Position>>(e);
		REQUIRE(sp.x == 100);
		REQUIRE(sp.y == 100);
		REQUIRE(sp.z == 100);

		p = w.get<Position>(e);
		spu = w.get<ecs::pair<Start, ecs::uni<Position>>>(e);
		REQUIRE(p.x == 5);
		REQUIRE(p.y == 5);
		REQUIRE(p.z == 5);
		REQUIRE(spu.x == 50);
		REQUIRE(spu.y == 50);
		REQUIRE(spu.z == 50);

		{
			uint32_t i = 0;
			auto q = w.query().all<ecs::pair<Start, Position>>();
			q.each([&]() {
				++i;
			});
			REQUIRE(i == 1);
		}
	}
}

TEST_CASE("CantCombine") {
	ecs::World w;
	auto weak = w.add();
	auto strong = w.add();
	w.add(weak, ecs::Pair(ecs::CantCombine, strong));

	auto dummy = w.add();
	w.add(dummy, strong);
#if !GAIA_ASSERT_ENABLED
	// Can be tested only with asserts disabled because the situation is assert-protected.
	w.add(dummy, weak);
	REQUIRE(w.has(dummy, strong));
	REQUIRE(!w.has(dummy, weak));
#endif
}

TEST_CASE("DependsOn") {
	ecs::World w;
	auto rabbit = w.add();
	auto animal = w.add();
	auto herbivore = w.add();
	auto carrot = w.add();
	w.add(carrot, ecs::Pair(ecs::DependsOn, herbivore));
	w.add(herbivore, ecs::Pair(ecs::DependsOn, animal));

	w.add(rabbit, carrot);
	REQUIRE(w.has(rabbit, herbivore));
	REQUIRE(w.has(rabbit, animal));

	w.del(rabbit, animal);
	REQUIRE(w.has(rabbit, animal));

	w.del(rabbit, herbivore);
	REQUIRE(w.has(rabbit, herbivore));

	w.del(rabbit, carrot);
	REQUIRE(!w.has(rabbit, carrot));
}

TEST_CASE("Inheritance (Is)") {
	ecs::World w;
	ecs::Entity animal = w.add();
	ecs::Entity herbivore = w.add();
	w.add<Position>(herbivore, {});
	w.add<Rotation>(herbivore, {});
	ecs::Entity rabbit = w.add();
	ecs::Entity hare = w.add();
	ecs::Entity wolf = w.add();

	w.as(herbivore, animal); // w.add(herbivore, ecs::Pair(ecs::Is, animal));
	w.as(rabbit, herbivore); // w.add(rabbit, ecs::Pair(ecs::Is, herbivore));
	w.as(hare, herbivore); // w.add(hare, ecs::Pair(ecs::Is, herbivore));
	w.as(wolf, animal); // w.add(wolf, ecs::Pair(ecs::Is, animal))

	REQUIRE(w.is(herbivore, animal));
	REQUIRE(w.is(rabbit, herbivore));
	REQUIRE(w.is(hare, herbivore));
	REQUIRE(w.is(rabbit, animal));
	REQUIRE(w.is(hare, animal));
	REQUIRE(w.is(wolf, animal));

	REQUIRE_FALSE(w.is(animal, herbivore));
	REQUIRE_FALSE(w.is(wolf, herbivore));

	{
		uint32_t i = 0;
		ecs::Query q = w.query().all(ecs::Pair(ecs::Is, animal));
		q.each([&](ecs::Entity entity) {
			// runs for herbivore, rabbit, hare, wolf
			const bool isOK = entity == hare || entity == rabbit || entity == herbivore || entity == wolf;
			REQUIRE(isOK);

			++i;
		});
		REQUIRE(i == 4);
	}
	{
		uint32_t i = 0;
		ecs::Query q = w.query().all(ecs::Pair(ecs::Is, herbivore));
		q.each([&](ecs::Entity entity) {
			// runs for rabbit and hare
			const bool isOK = entity == hare || entity == rabbit;
			REQUIRE(isOK);

			++i;
		});
		REQUIRE(i == 2);
	}
}

TEST_CASE("AddAndDel_entity - no components") {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	auto create = [&]() {
		auto e = w.add();
		arr.push_back(e);
	};
	auto remove = [&](ecs::Entity e) {
		w.del(e);
		const bool isEntityValid = w.valid(e);
		REQUIRE_FALSE(isEntityValid);
	};

	// Create entities
	GAIA_FOR(N) create();
	// Remove entities
	GAIA_FOR(N) remove(arr[i]);
}

TEST_CASE("AddAndDel_entity - 1 component") {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	auto create = [&](uint32_t id) {
		auto e = w.add();
		arr.push_back(e);

		w.add<Int3>(e, {id, id, id});
		auto pos = w.get<Int3>(e);
		REQUIRE(pos.x == id);
		REQUIRE(pos.y == id);
		REQUIRE(pos.z == id);
		return e;
	};
	auto remove = [&](ecs::Entity e) {
		w.del(e);
		const bool isEntityValid = w.valid(e);
		REQUIRE_FALSE(isEntityValid);
	};

	GAIA_FOR(N) create(i);
	GAIA_FOR(N) remove(arr[i]);
}

void verify_entity_has(const ecs::ComponentCache& cc, ecs::Entity entity) {
	const auto* res = cc.find(entity);
	REQUIRE(res != nullptr);
}

template <typename T>
void verify_name_has(const ecs::ComponentCache& cc, const char* str) {
	auto name = cc.get<T>().name;
	REQUIRE(name.str() != nullptr);
	REQUIRE(name.len() > 1);
	const auto* res = cc.find(str);
	REQUIRE(res != nullptr);
}

void verify_name_has_not(const ecs::ComponentCache& cc, const char* str) {
	const auto* item = cc.find(str);
	REQUIRE(item == nullptr);
}
template <typename T>
void verify_name_has_not(const ecs::ComponentCache& cc) {
	const auto* item = cc.find<T>();
	REQUIRE(item == nullptr);
}

#define verify_name_has(name) verify_name_has<name>(cc, #name);
#define verify_name_has_not(name)                                                                                      \
	{                                                                                                                    \
		verify_name_has_not<name>(cc);                                                                                     \
		verify_name_has_not(cc, #name);                                                                                    \
	}

TEST_CASE("Component names") {
	SECTION("direct registration") {
		ecs::World w;
		const auto& cc = w.comp_cache();

		verify_name_has_not(Int3);
		verify_name_has_not(Position);
		verify_name_has_not(dummy::Position);

		auto e_pos = w.add<Position>().entity;
		verify_entity_has(cc, e_pos);
		verify_name_has_not(Int3);
		verify_name_has(Position);
		verify_name_has_not(dummy::Position);

		auto e_int = w.add<Int3>().entity;
		verify_entity_has(cc, e_pos);
		verify_entity_has(cc, e_int);
		verify_name_has(Int3);
		verify_name_has(Position);
		verify_name_has_not(dummy::Position);

		auto e_dpos = w.add<dummy::Position>().entity;
		verify_entity_has(cc, e_pos);
		verify_entity_has(cc, e_int);
		verify_entity_has(cc, e_dpos);
		verify_name_has(Int3);
		verify_name_has(Position);
		verify_name_has(dummy::Position);
	}
	SECTION("entity+component") {
		ecs::World w;
		const auto& cc = w.comp_cache();
		auto e = w.add();

		verify_name_has_not(Int3);
		verify_name_has_not(Position);
		verify_name_has_not(dummy::Position);

		w.add<Position>(e);
		verify_name_has_not(Int3);
		verify_name_has(Position);
		verify_name_has_not(dummy::Position);

		w.add<Int3>(e);
		verify_name_has(Int3);
		verify_name_has(Position);
		verify_name_has_not(dummy::Position);

		w.del<Position>(e);
		verify_name_has(Int3);
		verify_name_has(Position);
		verify_name_has_not(dummy::Position);

		w.add<dummy::Position>(e);
		verify_name_has(Int3);
		verify_name_has(Position);
		verify_name_has(dummy::Position);
	}
}

template <typename TQuery>
void Test_Query_QueryResult() {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> ents;
	ents.reserve(N);

	auto create = [&](uint32_t i) {
		auto e = w.add();
		ents.push_back(e);
		w.add<Position>(e, {(float)i, (float)i, (float)i});
	};

	GAIA_FOR(N) create(i);

	constexpr bool UseCachedQuery = std::is_same_v<TQuery, ecs::Query>;
	auto q1 = w.query<UseCachedQuery>().template all<Position>();
	auto q2 = w.query<UseCachedQuery>().template all<Rotation>();
	auto q3 = w.query<UseCachedQuery>().template all<Position, Rotation>();

	{
		const auto cnt = q1.count();
		GAIA_ASSERT(cnt == N);
	}
	{
		cnt::darr<ecs::Entity> arr;
		q1.arr(arr);
		GAIA_ASSERT(arr.size() == N);
		GAIA_EACH(arr) REQUIRE(arr[i] == ents[i]);
	}
	{
		cnt::darr<Position> arr;
		q1.arr(arr);
		GAIA_ASSERT(arr.size() == N);
		GAIA_EACH(arr) {
			const auto& pos = arr[i];
			REQUIRE(pos.x == (float)i);
			REQUIRE(pos.y == (float)i);
			REQUIRE(pos.z == (float)i);
		}
	}
	{
		const auto cnt = q1.count();
		REQUIRE(cnt > 0);

		const auto empty = q1.empty();
		REQUIRE(empty == false);

		uint32_t cnt2 = 0;
		q1.each([&]() {
			++cnt2;
		});
		REQUIRE(cnt == cnt2);
	}

	{
		const auto cnt = q2.count();
		REQUIRE(cnt == 0);

		const auto empty = q2.empty();
		REQUIRE(empty == true);

		uint32_t cnt2 = 0;
		q2.each([&]() {
			++cnt2;
		});
		REQUIRE(cnt == cnt2);
	}

	{
		const auto cnt = q3.count();
		REQUIRE(cnt == 0);

		const auto empty = q3.empty();
		REQUIRE(empty == true);

		uint32_t cnt3 = 0;
		q3.each([&]() {
			++cnt3;
		});
		REQUIRE(cnt == cnt3);
	}

	// Verify querying at a different source entity works, too
	auto game = w.add();
	struct Level {
		uint32_t value;
	};
	w.add<Level>(game, {2});
	auto q4 = w.query<UseCachedQuery>().template all<Position>().all(w.add<Level>().entity, game);

	{
		const auto cnt = q4.count();
		GAIA_ASSERT(cnt == N);
	}
	{
		cnt::darr<ecs::Entity> arr;
		q4.arr(arr);
		GAIA_ASSERT(arr.size() == N);
		GAIA_EACH(arr) REQUIRE(arr[i] == ents[i]);
	}
	{
		cnt::darr<Position> arr;
		q4.arr(arr);
		GAIA_ASSERT(arr.size() == N);
		GAIA_EACH(arr) {
			const auto& pos = arr[i];
			REQUIRE(pos.x == (float)i);
			REQUIRE(pos.y == (float)i);
			REQUIRE(pos.z == (float)i);
		}
	}
	{
		const auto cnt = q4.count();
		REQUIRE(cnt > 0);

		const auto empty = q4.empty();
		REQUIRE(empty == false);

		uint32_t cnt2 = 0;
		q4.each([&]() {
			++cnt2;
		});
		REQUIRE(cnt == cnt2);
	}
	{
		const auto cnt = q4.count();

		uint32_t cnt2 = 0;
		q4.each([&](ecs::Iter it /*, ecs::Src src*/) {
			auto pos_view = it.view<Position>();
			// auto lvl_view = src.view<Level>();
			GAIA_EACH(it) {
				++cnt2;
			}
		});
		REQUIRE(cnt == cnt2);
	}
}

TEST_CASE("Query - QueryResult") {
	SECTION("Cached query") {
		Test_Query_QueryResult<ecs::Query>();
	}
	SECTION("Non-cached query") {
		Test_Query_QueryResult<ecs::QueryUncached>();
	}
}

template <typename TQuery>
void Test_Query_QueryResult_Complex() {
	const uint32_t N = 1'500;

	ecs::World w;
	struct Data {
		Position p;
		Scale s;
	};
	cnt::map<ecs::Entity, Data> cmp;
	cnt::darr<ecs::Entity> ents;
	ents.reserve(N);

	auto create = [&](uint32_t i) {
		auto e = w.add();

		auto b = w.bulk(e);
		b.add<Position, Scale>();
		if (i % 2 == 0)
			b.add<Something>();
		b.commit();

		auto p1 = Position{(float)i, (float)i, (float)i};
		auto s1 = Scale{(float)i * 2, (float)i * 2, (float)i * 2};

		auto s = w.set(e);
		s.set<Position>(p1).set<Scale>(s1);
		if (i % 2 == 0)
			s.set<Something>({true});

		auto p0 = w.get<Position>(e);
		REQUIRE(memcmp((void*)&p0, (void*)&p1, sizeof(p0)) == 0);
		cmp.try_emplace(e, Data{p1, s1});
	};

	GAIA_FOR(N) create(i);

	constexpr bool UseCachedQuery = std::is_same_v<TQuery, ecs::Query>;
	auto q1 = w.query<UseCachedQuery>().template all<Position>();
	auto q2 = w.query<UseCachedQuery>().template all<Rotation>();
	auto q3 = w.query<UseCachedQuery>().template all<Position, Rotation>();
	auto q4 = w.query<UseCachedQuery>().template all<Position, Scale>();
	auto q5 = w.query<UseCachedQuery>().template all<Position, Scale, Something>();

	{
		ents.clear();
		q1.arr(ents);
		REQUIRE(ents.size() == N);
	}
	{
		cnt::darr<Position> arr;
		q1.arr(arr);
		REQUIRE(arr.size() == N);
		GAIA_EACH(arr) {
			const auto e = ents[i];
			const auto& v0 = arr[i];
			const auto& v1 = w.get<Position>(e);
			REQUIRE(memcmp((const void*)&v0, (const void*)&v1, sizeof(v0)) == 0);
			REQUIRE(memcmp((const void*)&v0, (const void*)&cmp[e].p, sizeof(v0)) == 0);
		}
	}
	{
		const auto cnt = q1.count();
		REQUIRE(cnt > 0);

		const auto empty = q1.empty();
		REQUIRE(empty == false);

		uint32_t cnt2 = 0;
		q1.each([&]() {
			++cnt2;
		});
		REQUIRE(cnt2 == cnt);
	}

	{
		const auto cnt = q2.count();
		REQUIRE(cnt == 0);

		const auto empty = q2.empty();
		REQUIRE(empty == true);

		uint32_t cnt2 = 0;
		q2.each([&]() {
			++cnt2;
		});
		REQUIRE(cnt2 == cnt);
	}

	{
		const auto cnt = q3.count();
		REQUIRE(cnt == 0);

		const auto empty = q3.empty();
		REQUIRE(empty == true);

		uint32_t cnt3 = 0;
		q3.each([&]() {
			++cnt3;
		});
		REQUIRE(cnt3 == cnt);
	}

	{
		ents.clear();
		q4.arr(ents);
		REQUIRE(ents.size() == N);
	}
	{
		cnt::darr<Position> arr;
		q4.arr(arr);
		REQUIRE(arr.size() == N);
		GAIA_EACH(arr) {
			const auto e = ents[i];
			const auto& v0 = arr[i];
			const auto& v1 = w.get<Position>(e);
			REQUIRE(memcmp((const void*)&v0, (const void*)&v1, sizeof(v0)) == 0);
			REQUIRE(memcmp((const void*)&v0, (const void*)&cmp[e].p, sizeof(v0)) == 0);
		}
	}
	{
		cnt::darr<Scale> arr;
		q4.arr(arr);
		REQUIRE(arr.size() == N);
		GAIA_EACH(arr) {
			const auto e = ents[i];
			const auto& v0 = arr[i];
			const auto& v1 = w.get<Scale>(e);
			REQUIRE(memcmp((const void*)&v0, (const void*)&v1, sizeof(v0)) == 0);
			REQUIRE(memcmp((const void*)&v0, (const void*)&cmp[e].s, sizeof(v0)) == 0);
		}
	}
	{
		const auto cnt = q4.count();
		REQUIRE(cnt > 0);

		const auto empty = q4.empty();
		REQUIRE(empty == false);

		uint32_t cnt4 = 0;
		q4.each([&]() {
			++cnt4;
		});
		REQUIRE(cnt4 == cnt);
	}

	{
		ents.clear();
		q5.arr(ents);
		REQUIRE(ents.size() == N / 2);
	}
	{
		cnt::darr<Position> arr;
		q5.arr(arr);
		REQUIRE(arr.size() == ents.size());
		REQUIRE(arr.size() == N / 2);

		GAIA_EACH(arr) {
			const auto e = ents[i];
			const auto& v0 = arr[i];
			const auto& v1 = w.get<Position>(e);
			REQUIRE(memcmp((const void*)&v0, (const void*)&v1, sizeof(v0)) == 0);
			REQUIRE(memcmp((const void*)&v0, (const void*)&cmp[e].p, sizeof(v0)) == 0);
		}
	}
	{
		cnt::darr<Scale> arr;
		q5.arr(arr);
		REQUIRE(arr.size() == N / 2);
		GAIA_EACH(arr) {
			const auto e = ents[i];
			const auto& v0 = arr[i];
			const auto& v1 = w.get<Scale>(e);
			REQUIRE(memcmp((const void*)&v0, (const void*)&v1, sizeof(v0)) == 0);
			REQUIRE(memcmp((const void*)&v0, (const void*)&cmp[e].s, sizeof(v0)) == 0);
		}
	}
	{
		const auto cnt = q5.count();
		REQUIRE(cnt > 0);

		const auto empty = q5.empty();
		REQUIRE(empty == false);

		uint32_t cnt5 = 0;
		q5.each([&]() {
			++cnt5;
		});
		REQUIRE(cnt5 == cnt);
	}
}

TEST_CASE("Query - QueryResult complex") {
	SECTION("Cached query") {
		Test_Query_QueryResult_Complex<ecs::Query>();
	}
	SECTION("Non-cached query") {
		Test_Query_QueryResult_Complex<ecs::QueryUncached>();
	}
}

TEST_CASE("Relationship") {
	ecs::World w;

	SECTION("Simple") {
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();

		w.add(rabbit, ecs::Pair(eats, carrot));
		w.add(wolf, ecs::Pair(eats, rabbit));

		REQUIRE(w.has(rabbit, ecs::Pair(eats, carrot)));
		REQUIRE(w.has(wolf, ecs::Pair(eats, rabbit)));
		REQUIRE_FALSE(w.has(wolf, ecs::Pair(eats, carrot)));
		REQUIRE_FALSE(w.has(rabbit, ecs::Pair(eats, wolf)));

		{
			auto q = w.query().add({ecs::Pair(eats, carrot), ecs::QueryOp::All, ecs::QueryAccess::None});
			// auto q = w.query().all<ecs::Pair(eats, carrot)>();
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == rabbit);
				++i;
			});
			REQUIRE(i == cnt);
		}
		{
			auto q = w.query().add("(%e, %e)", eats.value(), carrot.value());
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == rabbit);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(eats, rabbit), ecs::QueryOp::All, ecs::QueryAccess::None});
			// auto q = w.query().all<ecs::Pair(eats, rabbit)>();
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == wolf);
				++i;
			});
			REQUIRE(i == cnt);
		}
	}

	SECTION("Simple - bulk") {
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();

		w.bulk(rabbit).add(ecs::Pair(eats, carrot));
		w.bulk(wolf).add(ecs::Pair(eats, rabbit));

		REQUIRE(w.has(rabbit, ecs::Pair(eats, carrot)));
		REQUIRE(w.has(wolf, ecs::Pair(eats, rabbit)));
		REQUIRE_FALSE(w.has(wolf, ecs::Pair(eats, carrot)));
		REQUIRE_FALSE(w.has(rabbit, ecs::Pair(eats, wolf)));

		{
			auto q = w.query().add({ecs::Pair(eats, carrot), ecs::QueryOp::All, ecs::QueryAccess::None});
			// auto q = w.query().all<ecs::Pair(eats, carrot)>();
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == rabbit);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(eats, rabbit), ecs::QueryOp::All, ecs::QueryAccess::None});
			// auto q = w.query().all<ecs::Pair(eats, rabbit)>();
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == wolf);
				++i;
			});
			REQUIRE(i == cnt);
		}
	}

	SECTION("Intermediate") {
		auto wolf = w.add();
		auto hare = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();

		w.add(rabbit, ecs::Pair(eats, carrot));
		w.add(hare, ecs::Pair(eats, carrot));
		w.add(wolf, ecs::Pair(eats, rabbit));

		{
			auto q = w.query().add({ecs::Pair(eats, carrot), ecs::QueryOp::All, ecs::QueryAccess::None});
			const auto cnt = q.count();
			REQUIRE(cnt == 2);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				const bool isRabbit = entity == rabbit;
				const bool isHare = entity == hare;
				const bool is = isRabbit || isHare;
				REQUIRE(is);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(eats, rabbit), ecs::QueryOp::All, ecs::QueryAccess::None});
			const auto cnt = q.count();
			REQUIRE(cnt == 1);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				REQUIRE(entity == wolf);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(eats, ecs::All), ecs::QueryOp::All, ecs::QueryAccess::None});
			const auto cnt = q.count();
			REQUIRE(cnt == 3);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				const bool isRabbit = entity == rabbit;
				const bool isHare = entity == hare;
				const bool isWolf = entity == wolf;
				const bool is = isRabbit || isHare || isWolf;
				REQUIRE(is);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(ecs::All, carrot), ecs::QueryOp::All, ecs::QueryAccess::None});
			const auto cnt = q.count();
			REQUIRE(cnt == 2);

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				const bool isRabbit = entity == rabbit;
				const bool isHare = entity == hare;
				const bool is = isRabbit || isHare;
				REQUIRE(is);
				++i;
			});
			REQUIRE(i == cnt);
		}

		{
			auto q = w.query().add({ecs::Pair(ecs::All, ecs::All), ecs::QueryOp::All, ecs::QueryAccess::None});
			const auto cnt = q.count();
			REQUIRE(cnt == 5); // 3 +2 for internal relationhsip

			uint32_t i = 0;
			q.each([&](ecs::Entity entity) {
				if (entity <= ecs::GAIA_ID(LastCoreComponent))
					return;
				const bool isRabbit = entity == rabbit;
				const bool isHare = entity == hare;
				const bool isWolf = entity == wolf;
				const bool is = isRabbit || isHare || isWolf;
				REQUIRE(is);
				++i;
			});
			REQUIRE(i == 3);
		}
	}
}

TEST_CASE("Relationship target/relation") {
	ecs::World w;

	auto wolf = w.add();
	auto hates = w.add();
	auto rabbit = w.add();
	auto carrot = w.add();
	auto salad = w.add();
	auto eats = w.add();

	w.add(rabbit, ecs::Pair(eats, carrot));
	w.add(rabbit, ecs::Pair(eats, salad));
	w.add(rabbit, ecs::Pair(hates, wolf));

	auto e = w.target(rabbit, eats);
	const bool ret_e = e == carrot || e == salad;
	REQUIRE(ret_e);

	cnt::sarr_ext<ecs::Entity, 32> out;
	w.targets(rabbit, eats, [&out](ecs::Entity target) {
		out.push_back(target);
	});
	REQUIRE(out.size() == 2);
	REQUIRE(core::has(out, carrot));
	REQUIRE(core::has(out, salad));
	REQUIRE(w.target(rabbit, eats) == carrot);

	out.clear();
	w.relations(rabbit, salad, [&out](ecs::Entity relation) {
		out.push_back(relation);
	});
	REQUIRE(out.size() == 1);
	REQUIRE(core::has(out, eats));
	REQUIRE(w.relation(rabbit, salad) == eats);
}

template <typename TQuery>
void Test_Query_Equality() {
	constexpr bool UseCachedQuery = std::is_same_v<TQuery, ecs::Query>;
	constexpr uint32_t N = 100;

	auto verify = [&](TQuery& q1, TQuery& q2, TQuery& q3, TQuery& q4) {
		REQUIRE(q1.count() == q2.count());
		REQUIRE(q1.count() == q3.count());
		REQUIRE(q1.count() == q4.count());

		cnt::darr<ecs::Entity> ents1, ents2, ents3, ents4;
		q1.arr(ents1);
		q2.arr(ents2);
		q3.arr(ents3);
		q4.arr(ents4);
		REQUIRE(ents1.size() == ents2.size());
		REQUIRE(ents1.size() == ents3.size());
		REQUIRE(ents1.size() == ents4.size());

		uint32_t i = 0;
		for (auto e: ents1) {
			REQUIRE(e == ents2[i]);
			++i;
		}
		i = 0;
		for (auto e: ents1) {
			REQUIRE(e == ents3[i]);
			++i;
		}
		i = 0;
		for (auto e: ents1) {
			REQUIRE(e == ents4[i]);
			++i;
		}
	};

	SECTION("2 components") {
		ecs::World w;
		GAIA_FOR(N) {
			auto e = w.add();
			w.add<Position>(e);
			w.add<Rotation>(e);
		}

		auto p = w.add<Position>().entity;
		auto r = w.add<Rotation>().entity;

		auto qq1 = w.query<UseCachedQuery>().template all<Position, Rotation>();
		auto qq2 = w.query<UseCachedQuery>().template all<Rotation, Position>();
		auto qq3 = w.query<UseCachedQuery>().all(p).all(r);
		auto qq4 = w.query<UseCachedQuery>().all(r).all(p);
		verify(qq1, qq2, qq3, qq4);

		auto qq1_ = w.query<UseCachedQuery>().add("Position; Rotation");
		auto qq2_ = w.query<UseCachedQuery>().add("Rotation; Position");
		auto qq3_ = w.query<UseCachedQuery>().add("Position").add("Rotation");
		auto qq4_ = w.query<UseCachedQuery>().add("Rotation").add("Position");
		verify(qq1_, qq2_, qq3_, qq4_);
	}
	SECTION("4 components") {
		ecs::World w;
		GAIA_FOR(N) {
			auto e = w.add();
			w.add<Position>(e);
			w.add<Rotation>(e);
			w.add<Acceleration>(e);
			w.add<Something>(e);
		}

		auto p = w.add<Position>().entity;
		auto r = w.add<Rotation>().entity;
		auto a = w.add<Acceleration>().entity;
		auto s = w.add<Something>().entity;

		auto qq1 = w.query<UseCachedQuery>().template all<Position, Rotation, Acceleration, Something>();
		auto qq2 = w.query<UseCachedQuery>().template all<Rotation, Something, Position, Acceleration>();
		auto qq3 = w.query<UseCachedQuery>().all(p).all(r).all(a).all(s);
		auto qq4 = w.query<UseCachedQuery>().all(r).all(p).all(s).all(a);
		verify(qq1, qq2, qq3, qq4);

		auto qq1_ = w.query<UseCachedQuery>().add("Position; Rotation; Acceleration; Something");
		auto qq2_ = w.query<UseCachedQuery>().add("Rotation; Something; Position; Acceleration");
		auto qq3_ = w.query<UseCachedQuery>().add("Position").add("Rotation").add("Acceleration").add("Something");
		auto qq4_ = w.query<UseCachedQuery>().add("Rotation").add("Position").add("Something").add("Acceleration");
		verify(qq1_, qq2_, qq3_, qq4_);
	}
}

TEST_CASE("Query - equality") {
	SECTION("Cached query") {
		Test_Query_Equality<ecs::Query>();
	}
	SECTION("Non-cached query") {
		Test_Query_Equality<ecs::QueryUncached>();
	}
}

TEST_CASE("Enable") {
	// 1,500 picked so we create enough entites that they overflow into another chunk
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	auto create = [&]() {
		auto e = w.add();
		w.add<Position>(e);
		arr.push_back(e);
	};

	GAIA_FOR(N) create();

	SECTION("State validity") {
		w.enable(arr[0], false);
		REQUIRE_FALSE(w.enabled(arr[0]));

		w.enable(arr[0], true);
		REQUIRE(w.enabled(arr[0]));

		w.enable(arr[1], false);
		REQUIRE(w.enabled(arr[0]));
		REQUIRE_FALSE(w.enabled(arr[1]));

		w.enable(arr[1], true);
		REQUIRE(w.enabled(arr[0]));
		REQUIRE(w.enabled(arr[1]));
	}

	SECTION("State persistance") {
		w.enable(arr[0], false);
		REQUIRE_FALSE(w.enabled(arr[0]));
		auto e = arr[0];
		w.del<Position>(e);
		REQUIRE_FALSE(w.enabled(e));

		w.enable(arr[0], true);
		w.add<Position>(arr[0]);
		REQUIRE(w.enabled(arr[0]));
	}

	ecs::Query q = w.query().all<Position>();

	auto checkQuery = [&q](uint32_t expectedCountAll, uint32_t expectedCountEnabled, uint32_t expectedCountDisabled) {
		{
			uint32_t cnt = 0;
			q.each([&]([[maybe_unused]] ecs::IterAll iter) {
				const uint32_t cExpected = iter.size();
				uint32_t c = 0;
				GAIA_EACH(iter)++ c;
				REQUIRE(c == cExpected);
				cnt += c;
			});
			REQUIRE(cnt == expectedCountAll);

			cnt = q.count(ecs::Constraints::AcceptAll);
			REQUIRE(cnt == expectedCountAll);
		}
		{
			uint32_t cnt = 0;
			q.each([&]([[maybe_unused]] ecs::Iter iter) {
				const uint32_t cExpected = iter.size();
				uint32_t c = 0;
				GAIA_EACH(iter) {
					REQUIRE(iter.enabled(i));
					++c;
				}
				REQUIRE(c == cExpected);
				cnt += c;
			});
			REQUIRE(cnt == expectedCountEnabled);

			cnt = q.count();
			REQUIRE(cnt == expectedCountEnabled);
		}
		{
			uint32_t cnt = 0;
			q.each([&]([[maybe_unused]] ecs::IterDisabled iter) {
				const uint32_t cExpected = iter.size();
				uint32_t c = 0;
				GAIA_EACH(iter) {
					REQUIRE(!iter.enabled(i));
					++c;
				}
				REQUIRE(c == cExpected);
				cnt += c;
			});
			REQUIRE(cnt == expectedCountDisabled);

			cnt = q.count(ecs::Constraints::DisabledOnly);
			REQUIRE(cnt == expectedCountDisabled);
		}
	};

	checkQuery(N, N, 0);

	SECTION("Disable vs query") {
		w.enable(arr[1000], false);
		checkQuery(N, N - 1, 1);
	}

	SECTION("Enable vs query") {
		w.enable(arr[1000], true);
		checkQuery(N, N, 0);
	}

	SECTION("Disable vs query") {
		w.enable(arr[1], false);
		w.enable(arr[100], false);
		w.enable(arr[999], false);
		w.enable(arr[1400], false);
		checkQuery(N, N - 4, 4);
	}

	SECTION("Enable vs query") {
		w.enable(arr[1], true);
		w.enable(arr[100], true);
		w.enable(arr[999], true);
		w.enable(arr[1400], true);
		checkQuery(N, N, 0);
	}
}

TEST_CASE("Add - generic") {
	{
		ecs::World w;
		auto e = w.add();

		auto f = w.add();
		w.add(e, f);
		REQUIRE(w.has(e, f));
	}

	{
		ecs::World w;
		auto e = w.add();

		w.add<Position>(e);
		w.add<Acceleration>(e);

		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e));

		auto f = w.add();
		w.add(e, f);
		REQUIRE(w.has(e, f));

		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e));
	}

	{
		ecs::World w;
		auto e = w.add();

		w.add<Position>(e, {1, 2, 3});
		w.add<Acceleration>(e, {4, 5, 6});

		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);

		auto a = w.get<Acceleration>(e);
		REQUIRE(a.x == 4.f);
		REQUIRE(a.y == 5.f);
		REQUIRE(a.z == 6.f);

		auto f = w.add();
		w.add(e, f);
		REQUIRE(w.has(e, f));

		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e));

		p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);

		a = w.get<Acceleration>(e);
		REQUIRE(a.x == 4.f);
		REQUIRE(a.y == 5.f);
		REQUIRE(a.z == 6.f);
	}
}

// TEST_CASE("Add - from query") {
// 	ecs::World w;
//
// 	cnt::sarray<ecs::Entity, 5> ents;
// 	for (auto& e: ents)
// 		e = w.add();
//
// 	for (uint32_t i = 0; i < ents.size() - 1; ++i)
// 		w.add<Position>(ents[i], {(float)i, (float)i, (float)i});
//
// 	ecs::Query q;
// 	q.all<Position>();
// 	w.add<Acceleration>(q, {1.f, 2.f, 3.f});
//
// 	for (uint32_t i = 0; i < ents.size() - 1; ++i) {
// 		REQUIRE(w.has<Position>(ents[i]));
// 		REQUIRE(w.has<Acceleration>(ents[i]));
//
// 		Position p;
// 		w.get<Position>(ents[i], p);
// 		REQUIRE(p.x == (float)i);
// 		REQUIRE(p.y == (float)i);
// 		REQUIRE(p.z == (float)i);
//
// 		Acceleration a;
// 		w.get<Acceleration>(ents[i], a);
// 		REQUIRE(a.x == 1.f);
// 		REQUIRE(a.y == 2.f);
// 		REQUIRE(a.z == 3.f);
// 	}
//
// 	{
// 		REQUIRE_FALSE(w.has<Position>(ents[4]));
// 		REQUIRE_FALSE(w.has<Acceleration>(ents[4]));
// 	}
// }

TEST_CASE("Add - unique") {
	{
		ecs::World w;
		auto e = w.add();

		auto f = w.add(ecs::EntityKind::EK_Uni);
		w.add(e, f);
		REQUIRE(w.has(e, f));
	}

	{
		ecs::World w;
		auto e = w.add();

		w.add<ecs::uni<Position>>(e);
		w.add<ecs::uni<Acceleration>>(e);

		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
	}

	{
		ecs::World w;
		auto e = w.add();

		// Add Position unique component
		w.add<ecs::uni<Position>>(e, {1, 2, 3});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		{
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);
		}
		// Add Acceleration unique component.
		// This moves "e" to a new archetype.
		w.add<ecs::uni<Acceleration>>(e, {4, 5, 6});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		{
			auto a = w.get<ecs::uni<Acceleration>>(e);
			REQUIRE(a.x == 4.f);
			REQUIRE(a.y == 5.f);
			REQUIRE(a.z == 6.f);
		}
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE_FALSE(p.x == 1.f);
			REQUIRE_FALSE(p.y == 2.f);
			REQUIRE_FALSE(p.z == 3.f);
		}
	}

	{
		ecs::World w;
		auto e = w.add();

		// Add Position unique component
		w.add<ecs::uni<Position>>(e, {1, 2, 3});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		{
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);
		}
		// Add Acceleration unique component.
		// This moves "e" to a new archetype.
		w.add<ecs::uni<Acceleration>>(e, {4, 5, 6});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		{
			auto a = w.get<ecs::uni<Acceleration>>(e);
			REQUIRE(a.x == 4.f);
			REQUIRE(a.y == 5.f);
			REQUIRE(a.z == 6.f);
		}
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE_FALSE(p.x == 1.f);
			REQUIRE_FALSE(p.y == 2.f);
			REQUIRE_FALSE(p.z == 3.f);
		}

		// Add a generic entity. Archetype changes.
		auto f = w.add();
		w.add(e, f);
		REQUIRE(w.has(e, f));

		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		{
			auto a = w.get<ecs::uni<Acceleration>>(e);
			REQUIRE_FALSE(a.x == 4.f);
			REQUIRE_FALSE(a.y == 5.f);
			REQUIRE_FALSE(a.z == 6.f);
		}
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE_FALSE(p.x == 1.f);
			REQUIRE_FALSE(p.y == 2.f);
			REQUIRE_FALSE(p.z == 3.f);
		}
	}

	{
		ecs::World w;
		auto e = w.add();

		// Add Position unique component
		w.add<ecs::uni<Position>>(e, {1, 2, 3});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		{
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);
		}
		// Add Acceleration unique component.
		// This moves "e" to a new archetype.
		w.add<ecs::uni<Acceleration>>(e, {4, 5, 6});
		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		{
			auto a = w.get<ecs::uni<Acceleration>>(e);
			REQUIRE(a.x == 4.f);
			REQUIRE(a.y == 5.f);
			REQUIRE(a.z == 6.f);
		}
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE_FALSE(p.x == 1.f);
			REQUIRE_FALSE(p.y == 2.f);
			REQUIRE_FALSE(p.z == 3.f);
		}

		// Add a unique entity. Archetype changes.
		auto f = w.add(ecs::EntityKind::EK_Uni);
		w.add(e, f);
		REQUIRE(w.has(e, f));

		REQUIRE(w.has<ecs::uni<Position>>(e));
		REQUIRE(w.has<ecs::uni<Acceleration>>(e));
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto a = w.get<ecs::uni<Acceleration>>(e);
			REQUIRE_FALSE(a.x == 4.f);
			REQUIRE_FALSE(a.y == 5.f);
			REQUIRE_FALSE(a.z == 6.f);
		}
		{
			// Because "e" was moved to a new archetype nobody ever set the value.
			// Therefore, it is garbage and won't match the original chunk.
			auto p = w.get<ecs::uni<Position>>(e);
			REQUIRE_FALSE(p.x == 1.f);
			REQUIRE_FALSE(p.y == 2.f);
			REQUIRE_FALSE(p.z == 3.f);
		}
	}
}

TEST_CASE("del - generic") {
	{
		ecs::World w;
		auto e1 = w.add();

		{
			w.add<Position>(e1);
			w.del<Position>(e1);
			REQUIRE_FALSE(w.has<Position>(e1));
		}
		{
			w.add<Rotation>(e1);
			w.del<Rotation>(e1);
			REQUIRE_FALSE(w.has<Rotation>(e1));
		}
	}
	{
		ecs::World w;
		auto e1 = w.add();
		{
			w.add<Position>(e1);
			w.add<Rotation>(e1);

			{
				w.del<Position>(e1);
				REQUIRE_FALSE(w.has<Position>(e1));
				REQUIRE(w.has<Rotation>(e1));
			}
			{
				w.del<Rotation>(e1);
				REQUIRE_FALSE(w.has<Position>(e1));
				REQUIRE_FALSE(w.has<Rotation>(e1));
			}
		}
		{
			w.add<Rotation>(e1);
			w.add<Position>(e1);
			{
				w.del<Position>(e1);
				REQUIRE_FALSE(w.has<Position>(e1));
				REQUIRE(w.has<Rotation>(e1));
			}
			{
				w.del<Rotation>(e1);
				REQUIRE_FALSE(w.has<Position>(e1));
				REQUIRE_FALSE(w.has<Rotation>(e1));
			}
		}
	}
}

TEST_CASE("del - unique") {
	ecs::World w;
	auto e1 = w.add();

	{
		w.add<ecs::uni<Position>>(e1);
		w.add<ecs::uni<Acceleration>>(e1);
		{
			w.del<ecs::uni<Position>>(e1);
			REQUIRE_FALSE(w.has<ecs::uni<Position>>(e1));
			REQUIRE(w.has<ecs::uni<Acceleration>>(e1));
		}
		{
			w.del<ecs::uni<Acceleration>>(e1);
			REQUIRE_FALSE(w.has<ecs::uni<Position>>(e1));
			REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e1));
		}
	}

	{
		w.add<ecs::uni<Acceleration>>(e1);
		w.add<ecs::uni<Position>>(e1);
		{
			w.del<ecs::uni<Position>>(e1);
			REQUIRE_FALSE(w.has<ecs::uni<Position>>(e1));
			REQUIRE(w.has<ecs::uni<Acceleration>>(e1));
		}
		{
			w.del<ecs::uni<Acceleration>>(e1);
			REQUIRE_FALSE(w.has<ecs::uni<Position>>(e1));
			REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e1));
		}
	}
}

TEST_CASE("del - generic, unique") {
	ecs::World w;
	auto e1 = w.add();

	{
		w.add<Position>(e1);
		w.add<Acceleration>(e1);
		w.add<ecs::uni<Position>>(e1);
		w.add<ecs::uni<Acceleration>>(e1);
		{
			w.del<Position>(e1);
			REQUIRE_FALSE(w.has<Position>(e1));
			REQUIRE(w.has<Acceleration>(e1));
			REQUIRE(w.has<ecs::uni<Position>>(e1));
			REQUIRE(w.has<ecs::uni<Acceleration>>(e1));
		}
		{
			w.del<Acceleration>(e1);
			REQUIRE_FALSE(w.has<Position>(e1));
			REQUIRE_FALSE(w.has<Acceleration>(e1));
			REQUIRE(w.has<ecs::uni<Position>>(e1));
			REQUIRE(w.has<ecs::uni<Acceleration>>(e1));
		}
		{
			w.del<ecs::uni<Acceleration>>(e1);
			REQUIRE_FALSE(w.has<Position>(e1));
			REQUIRE_FALSE(w.has<Acceleration>(e1));
			REQUIRE(w.has<ecs::uni<Position>>(e1));
			REQUIRE_FALSE(w.has<ecs::uni<Acceleration>>(e1));
		}
	}
}

TEST_CASE("del - cleanup rules") {
	SECTION("default") {
		ecs::World w;
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();
		auto hungry = w.add();
		w.add(wolf, hungry);
		w.add(wolf, ecs::Pair(eats, rabbit));
		w.add(rabbit, ecs::Pair(eats, carrot));

		w.del(wolf);
		REQUIRE(!w.has(wolf));
		REQUIRE(w.has(rabbit));
		REQUIRE(w.has(eats));
		REQUIRE(w.has(carrot));
		REQUIRE(w.has(hungry));
		// global relationships
		REQUIRE(w.has(ecs::Pair(eats, rabbit)));
		REQUIRE(w.has(ecs::Pair(eats, carrot)));
		REQUIRE(w.has(ecs::Pair(eats, ecs::All)));
		REQUIRE(w.has(ecs::Pair(ecs::All, carrot)));
		REQUIRE(w.has(ecs::Pair(ecs::All, rabbit)));
		REQUIRE(w.has(ecs::Pair(ecs::All, ecs::All)));
		// rabbit relationships
		REQUIRE(w.has(rabbit, ecs::Pair(eats, carrot)));
		REQUIRE(w.has(rabbit, ecs::Pair(eats, ecs::All)));
		REQUIRE(w.has(rabbit, ecs::Pair(ecs::All, carrot)));
		REQUIRE(w.has(rabbit, ecs::Pair(ecs::All, ecs::All)));
	}
	SECTION("default, relationship source") {
		ecs::World w;
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();
		auto hungry = w.add();
		w.add(wolf, hungry);
		w.add(wolf, ecs::Pair(eats, rabbit));
		w.add(rabbit, ecs::Pair(eats, carrot));

		w.del(eats);
		REQUIRE(w.has(wolf));
		REQUIRE(w.has(rabbit));
		REQUIRE(!w.has(eats));
		REQUIRE(w.has(carrot));
		REQUIRE(w.has(hungry));
		REQUIRE(!w.has(wolf, ecs::Pair(eats, rabbit)));
		REQUIRE(!w.has(rabbit, ecs::Pair(eats, carrot)));
	}
	SECTION("(OnDelete,Remove)") {
		ecs::World w;
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();
		auto hungry = w.add();
		w.add(wolf, hungry);
		w.add(wolf, ecs::Pair(ecs::OnDelete, ecs::Remove));
		w.add(wolf, ecs::Pair(eats, rabbit));
		w.add(rabbit, ecs::Pair(eats, carrot));

		w.del(wolf);
		REQUIRE(!w.has(wolf));
		REQUIRE(w.has(rabbit));
		REQUIRE(w.has(eats));
		REQUIRE(w.has(carrot));
		REQUIRE(w.has(hungry));
		REQUIRE(w.has(ecs::Pair(eats, rabbit)));
		REQUIRE(w.has(ecs::Pair(eats, carrot)));
		REQUIRE(w.has(rabbit, ecs::Pair(eats, carrot)));
	}
	SECTION("(OnDelete,Delete)") {
		ecs::World w;
		auto wolf = w.add();
		auto rabbit = w.add();
		auto carrot = w.add();
		auto eats = w.add();
		auto hungry = w.add();
		w.add(wolf, hungry);
		w.add(hungry, ecs::Pair(ecs::OnDelete, ecs::Delete));
		w.add(wolf, ecs::Pair(eats, rabbit));
		w.add(rabbit, ecs::Pair(eats, carrot));

		w.del(hungry);
		REQUIRE(!w.has(wolf));
		REQUIRE(w.has(rabbit));
		REQUIRE(w.has(eats));
		REQUIRE(w.has(carrot));
		REQUIRE(!w.has(hungry));
		REQUIRE(w.has(ecs::Pair(eats, rabbit)));
		REQUIRE(w.has(ecs::Pair(eats, carrot)));
	}
	SECTION("(OnDeleteTarget,Delete)") {
		ecs::World w;
		auto parent = w.add();
		auto child = w.add();
		auto child_of = w.add();
		w.add(child_of, ecs::Pair(ecs::OnDeleteTarget, ecs::Delete));
		w.add(child, ecs::Pair(child_of, parent));

		w.del(parent);
		REQUIRE(!w.has(child));
		REQUIRE(!w.has(parent));
	}
}

TEST_CASE("entity name") {
	constexpr uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> ents;
	ents.reserve(N);

	constexpr auto MaxLen = 32;
	char tmp[MaxLen];

	auto create = [&]() {
		auto e = w.add();
		GAIA_STRFMT(tmp, MaxLen, "name_%u", e.id());
		w.name(e, tmp);
		ents.push_back(e);
	};
	auto create_raw = [&]() {
		auto e = w.add();
		GAIA_STRFMT(tmp, MaxLen, "name_%u", e.id());
		w.name_raw(e, tmp);
		ents.push_back(e);
	};
	auto verify = [&](uint32_t i) {
		auto e = ents[i];
		GAIA_STRFMT(tmp, MaxLen, "name_%u", e.id());
		const auto* ename = w.name(e);

		const auto l0 = strlen(tmp);
		const auto l1 = strlen(ename);
		REQUIRE(l0 == l1);
		REQUIRE(strcmp(tmp, ename) == 0);
	};

	SECTION("basic") {
		ents.clear();
		create();
		verify(0);
		auto e = ents[0];

		char original[MaxLen];
		GAIA_STRFMT(original, MaxLen, "%s", w.name(e));

		// If we change the original string we still must have a match
		{
			GAIA_STRCPY(tmp, MaxLen, "some_random_string");
			REQUIRE(strcmp(w.name(e), original) == 0);
			REQUIRE(w.get(original) == e);
			REQUIRE(w.get(tmp) == ecs::EntityBad);

			// Change the name back
			GAIA_STRCPY(tmp, MaxLen, original);
			verify(0);
		}

#if !GAIA_ASSERT_ENABLED
		// Renaming has to fail. Can be tested only with asserts disabled
		// because the situation is assert-protected.
		{
			auto e1 = w.add();
			w.name(e1, original);
			REQUIRE(w.name(e1) == nullptr);
			REQUIRE(w.get(original) == e);
		}
#endif

		w.name(e, nullptr);
		REQUIRE(w.get(original) == ecs::EntityBad);
		REQUIRE(w.name(e) == nullptr);

		w.name(e, original);
		w.del(e);
		REQUIRE(w.get(original) == ecs::EntityBad);
	}

	SECTION("basic - non-owned") {
		ents.clear();
		create_raw();
		verify(0);
		auto e = ents[0];

		char original[MaxLen];
		GAIA_STRFMT(original, MaxLen, "%s", w.name(e));

		// If we change the original string we can't have a match
		{
			GAIA_STRCPY(tmp, MaxLen, "some_random_string");
			const auto* str = w.name(e);
			REQUIRE(strcmp(str, "some_random_string") == 0);
			REQUIRE(w.get(original) == ecs::EntityBad);
			// Hash was calculated for [original] but we changed the string to "some_random_string".
			// Hash won't match so we shouldn't be able to find the entity still.
			REQUIRE(w.get("some_random_string") == ecs::EntityBad);
		}

		{
			// Change the name back
			GAIA_STRCPY(tmp, MaxLen, original);
			verify(0);
		}

		w.name(e, nullptr);
		REQUIRE(w.get(original) == ecs::EntityBad);
		REQUIRE(w.name(e) == nullptr);

		w.name_raw(e, original);
		w.del(e);
		REQUIRE(w.get(original) == ecs::EntityBad);
	}

	SECTION("two") {
		ents.clear();
		GAIA_FOR(2) create();
		GAIA_FOR(2) verify(i);
		w.del(ents[0]);
		verify(1);
	}

	SECTION("many") {
		ents.clear();
		GAIA_FOR(N) create();
		GAIA_FOR(N) verify(i);
		w.del(ents[900]);
		GAIA_FOR(900) verify(i);
		GAIA_FOR2(901, N) verify(i);

		{
			auto e = ents[1000];

			char original[MaxLen];
			GAIA_STRFMT(original, MaxLen, "%s", w.name(e));

			{
				w.enable(e, false);
				const auto* str = w.name(e);
				REQUIRE(strcmp(str, original) == 0);
				REQUIRE(e == w.get(original));
			}
			{
				w.enable(e, true);
				const auto* str = w.name(e);
				REQUIRE(strcmp(str, original) == 0);
				REQUIRE(e == w.get(original));
			}
		}
	}
}

TEST_CASE("Usage 1 - simple query, 0 component") {
	ecs::World w;

	auto e = w.add();

	auto qa = w.query().all<Acceleration>();
	auto qp = w.query().all<Position>();

	{
		uint32_t cnt = 0;
		qa.each([&]([[maybe_unused]] const Acceleration&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}

	auto e1 = w.copy(e);
	auto e2 = w.copy(e);
	auto e3 = w.copy(e);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}

	w.del(e1);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}

	w.del(e2);
	w.del(e3);
	w.del(e);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
}

TEST_CASE("entity copy") {
	ecs::World w;

	auto e1 = w.add();
	auto e2 = w.add();
	w.add(e1, e2);
	auto e3 = w.copy(e1);

	REQUIRE(w.has(e3, e2));
}

TEST_CASE("Usage 1 - simple query, 1 component") {
	ecs::World w;

	auto e = w.add();
	w.add<Position>(e);

	auto qa = w.query().all<Acceleration>();
	auto qp = w.query().all<Position>();

	{
		uint32_t cnt = 0;
		qa.each([&]([[maybe_unused]] const Acceleration&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}

	auto e1 = w.copy(e);
	auto e2 = w.copy(e);
	auto e3 = w.copy(e);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 4);
	}

	w.del(e1);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 3);
	}

	w.del(e2);
	w.del(e3);
	w.del(e);

	{
		uint32_t cnt = 0;
		qp.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
}

TEST_CASE("Usage 1 - simple query, 1 unique component") {
	ecs::World w;

	auto e = w.add();
	w.add<ecs::uni<Position>>(e);

	auto q = w.query().all<ecs::uni<Position>>();
	auto qq = w.query().all<Position>();

	{
		uint32_t cnt = 0;
		qq.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{
		uint32_t cnt = 0;
		qq.each([&]() {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}

	auto e1 = w.copy(e);
	auto e2 = w.copy(e);
	auto e3 = w.copy(e);

	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}

	w.del(e1);

	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}

	w.del(e2);
	w.del(e3);
	w.del(e);

	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
}

TEST_CASE("Usage 2 - simple query, many components") {
	ecs::World w;

	auto e1 = w.add();
	w.add<Position>(e1, {});
	w.add<Acceleration>(e1, {});
	w.add<Else>(e1, {});
	auto e2 = w.add();
	w.add<Rotation>(e2, {});
	w.add<Scale>(e2, {});
	w.add<Else>(e2, {});
	auto e3 = w.add();
	w.add<Position>(e3, {});
	w.add<Acceleration>(e3, {});
	w.add<Scale>(e3, {});

	{
		uint32_t cnt = 0;
		auto q = w.query().all<Position>();
		q.each([&]([[maybe_unused]] const Position&) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<Acceleration>();
		q.each([&]([[maybe_unused]] const Acceleration&) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<Rotation>();
		q.each([&]([[maybe_unused]] const Rotation&) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<Scale>();
		q.each([&]([[maybe_unused]] const Scale&) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<Position, Acceleration>();
		q.each([&]([[maybe_unused]] const Position&, [[maybe_unused]] const Acceleration&) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<Position, Scale>();
		q.each([&]([[maybe_unused]] const Position&, [[maybe_unused]] const Scale&) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}
	{
		ecs::Query q = w.query().any<Position, Acceleration>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			const bool ok1 = iter.has<Position>() || iter.has<Acceleration>();
			REQUIRE(ok1);
			const bool ok2 = iter.has<Acceleration>() || iter.has<Position>();
			REQUIRE(ok2);
		});
		REQUIRE(cnt == 2);
	}
	{
		ecs::Query q = w.query().any<Position, Acceleration>().all<Scale>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			REQUIRE(iter.size() == 1);

			const bool ok1 = iter.has<Position>() || iter.has<Acceleration>();
			REQUIRE(ok1);
			const bool ok2 = iter.has<Acceleration>() || iter.has<Position>();
			REQUIRE(ok2);
		});
		REQUIRE(cnt == 1);
	}
	{
		ecs::Query q = w.query().any<Position, Acceleration>().no<Scale>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			REQUIRE(iter.size() == 1);
		});
		REQUIRE(cnt == 1);
	}
}

TEST_CASE("Usage 2 - simple query, many unique components") {
	ecs::World w;

	auto e1 = w.add();
	w.add<ecs::uni<Position>>(e1, {});
	w.add<ecs::uni<Acceleration>>(e1, {});
	w.add<ecs::uni<Else>>(e1, {});
	auto e2 = w.add();
	w.add<ecs::uni<Rotation>>(e2, {});
	w.add<ecs::uni<Scale>>(e2, {});
	w.add<ecs::uni<Else>>(e2, {});
	auto e3 = w.add();
	w.add<ecs::uni<Position>>(e3, {});
	w.add<ecs::uni<Acceleration>>(e3, {});
	w.add<ecs::uni<Scale>>(e3, {});

	{
		uint32_t cnt = 0;
		auto q = w.query().all<ecs::uni<Position>>();
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<ecs::uni<Acceleration>>();
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<ecs::uni<Rotation>>();
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<ecs::uni<Else>>();
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		uint32_t cnt = 0;
		auto q = w.query().all<ecs::uni<Scale>>();
		q.each([&]([[maybe_unused]] ecs::Iter iter) {
			++cnt;
		});
		REQUIRE(cnt == 2);
	}
	{
		auto q = w.query().any<ecs::uni<Position>, ecs::uni<Acceleration>>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			const bool ok1 = iter.has<ecs::uni<Position>>() || iter.has<ecs::uni<Acceleration>>();
			REQUIRE(ok1);
			const bool ok2 = iter.has<ecs::uni<Acceleration>>() || iter.has<ecs::uni<Position>>();
			REQUIRE(ok2);
		});
		REQUIRE(cnt == 2);
	}
	{
		auto q = w.query().any<ecs::uni<Position>, ecs::uni<Acceleration>>().all<ecs::uni<Scale>>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			REQUIRE(iter.size() == 1);

			const bool ok1 = iter.has<ecs::uni<Position>>() || iter.has<ecs::uni<Acceleration>>();
			REQUIRE(ok1);
			const bool ok2 = iter.has<ecs::uni<Acceleration>>() || iter.has<ecs::uni<Position>>();
			REQUIRE(ok2);
		});
		REQUIRE(cnt == 1);
	}
	{
		auto q = w.query().any<ecs::uni<Position>, ecs::uni<Acceleration>>().no<ecs::uni<Scale>>();

		uint32_t cnt = 0;
		q.each([&](ecs::Iter iter) {
			++cnt;

			REQUIRE(iter.size() == 1);
		});
		REQUIRE(cnt == 1);
	}
}

TEST_CASE("Set - generic") {
	ecs::World w;

	constexpr uint32_t N = 100;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	GAIA_FOR(N) {
		const auto ent = w.add();
		arr.push_back(ent);
		w.add<Rotation>(ent, {});
		w.add<Scale>(ent, {});
		w.add<Else>(ent, {});
	}

	// Default values
	for (const auto ent: arr) {
		auto r = w.get<Rotation>(ent);
		REQUIRE(r.x == 0.f);
		REQUIRE(r.y == 0.f);
		REQUIRE(r.z == 0.f);
		REQUIRE(r.w == 0.f);

		auto s = w.get<Scale>(ent);
		REQUIRE(s.x == 0.f);
		REQUIRE(s.y == 0.f);
		REQUIRE(s.z == 0.f);

		auto e = w.get<Else>(ent);
		REQUIRE(e.value == false);
	}

	// Modify values
	{
		ecs::Query q = w.query().all<Rotation&, Scale&, Else&>();

		q.each([&](ecs::Iter iter) {
			auto rotationView = iter.view_mut<Rotation>();
			auto scaleView = iter.view_mut<Scale>();
			auto elseView = iter.view_mut<Else>();

			GAIA_EACH(iter) {
				rotationView[i] = {1, 2, 3, 4};
				scaleView[i] = {11, 22, 33};
				elseView[i] = {true};
			}
		});

		for (const auto ent: arr) {
			auto r = w.get<Rotation>(ent);
			REQUIRE(r.x == 1.f);
			REQUIRE(r.y == 2.f);
			REQUIRE(r.z == 3.f);
			REQUIRE(r.w == 4.f);

			auto s = w.get<Scale>(ent);
			REQUIRE(s.x == 11.f);
			REQUIRE(s.y == 22.f);
			REQUIRE(s.z == 33.f);

			auto e = w.get<Else>(ent);
			REQUIRE(e.value == true);
		}
	}

	// Add one more component and check if the values are still fine after creating a new archetype
	{
		auto ent = w.copy(arr[0]);
		w.add<Position>(ent, {5, 6, 7});

		auto r = w.get<Rotation>(ent);
		REQUIRE(r.x == 1.f);
		REQUIRE(r.y == 2.f);
		REQUIRE(r.z == 3.f);
		REQUIRE(r.w == 4.f);

		auto s = w.get<Scale>(ent);
		REQUIRE(s.x == 11.f);
		REQUIRE(s.y == 22.f);
		REQUIRE(s.z == 33.f);

		auto e = w.get<Else>(ent);
		REQUIRE(e.value == true);
	}
}

TEST_CASE("Set - generic & unique") {
	ecs::World w;

	constexpr uint32_t N = 100;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	GAIA_FOR(N) {
		arr.push_back(w.add());
		w.add<Rotation>(arr.back(), {});
		w.add<Scale>(arr.back(), {});
		w.add<Else>(arr.back(), {});
		w.add<ecs::uni<Position>>(arr.back(), {});
	}

	// Default values
	for (const auto ent: arr) {
		auto r = w.get<Rotation>(ent);
		REQUIRE(r.x == 0.f);
		REQUIRE(r.y == 0.f);
		REQUIRE(r.z == 0.f);
		REQUIRE(r.w == 0.f);

		auto s = w.get<Scale>(ent);
		REQUIRE(s.x == 0.f);
		REQUIRE(s.y == 0.f);
		REQUIRE(s.z == 0.f);

		auto e = w.get<Else>(ent);
		REQUIRE(e.value == false);

		auto p = w.get<ecs::uni<Position>>(ent);
		REQUIRE(p.x == 0.f);
		REQUIRE(p.y == 0.f);
		REQUIRE(p.z == 0.f);
	}

	// Modify values
	{
		ecs::Query q = w.query().all<Rotation&, Scale&, Else&>();

		q.each([&](ecs::Iter iter) {
			auto rotationView = iter.view_mut<Rotation>();
			auto scaleView = iter.view_mut<Scale>();
			auto elseView = iter.view_mut<Else>();

			GAIA_EACH(iter) {
				rotationView[i] = {1, 2, 3, 4};
				scaleView[i] = {11, 22, 33};
				elseView[i] = {true};
			}
		});

		w.set<ecs::uni<Position>>(arr[0], {111, 222, 333});

		{
			Position p = w.get<ecs::uni<Position>>(arr[0]);
			REQUIRE(p.x == 111.f);
			REQUIRE(p.y == 222.f);
			REQUIRE(p.z == 333.f);
		}
		{
			for (const auto ent: arr) {
				auto r = w.get<Rotation>(ent);
				REQUIRE(r.x == 1.f);
				REQUIRE(r.y == 2.f);
				REQUIRE(r.z == 3.f);
				REQUIRE(r.w == 4.f);

				auto s = w.get<Scale>(ent);
				REQUIRE(s.x == 11.f);
				REQUIRE(s.y == 22.f);
				REQUIRE(s.z == 33.f);

				auto e = w.get<Else>(ent);
				REQUIRE(e.value == true);
			}
		}
		{
			auto p = w.get<ecs::uni<Position>>(arr[0]);
			REQUIRE(p.x == 111.f);
			REQUIRE(p.y == 222.f);
			REQUIRE(p.z == 333.f);
		}
	}
}

TEST_CASE("Components - non trivial") {
	ecs::World w;

	constexpr uint32_t N = 100;
	cnt::darr<ecs::Entity> arr;
	arr.reserve(N);

	GAIA_FOR(N) {
		arr.push_back(w.add());
		w.add<StringComponent>(arr.back(), {});
		w.add<StringComponent2>(arr.back(), {});
		w.add<PositionNonTrivial>(arr.back(), {});
	}

	// Default values
	for (const auto ent: arr) {
		const auto& s1 = w.get<StringComponent>(ent);
		REQUIRE(s1.value.empty());

		{
			auto s2 = w.get<StringComponent2>(ent);
			REQUIRE(s2.value == StringComponent2DefaultValue);
		}
		{
			const auto& s2 = w.get<StringComponent2>(ent);
			REQUIRE(s2.value == StringComponent2DefaultValue);
		}

		const auto& p = w.get<PositionNonTrivial>(ent);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);
	}

	// Modify values
	{
		ecs::Query q = w.query().all<StringComponent&, StringComponent2&, PositionNonTrivial&>();

		q.each([&](ecs::Iter iter) {
			auto strView = iter.view_mut<StringComponent>();
			auto str2View = iter.view_mut<StringComponent2>();
			auto posView = iter.view_mut<PositionNonTrivial>();

			GAIA_EACH(iter) {
				strView[i] = {StringComponentDefaultValue};
				str2View[i].value = StringComponent2DefaultValue_2;
				posView[i] = {111, 222, 333};
			}
		});

		for (const auto ent: arr) {
			const auto& s1 = w.get<StringComponent>(ent);
			REQUIRE(s1.value == StringComponentDefaultValue);

			const auto& s2 = w.get<StringComponent2>(ent);
			REQUIRE(s2.value == StringComponent2DefaultValue_2);

			const auto& p = w.get<PositionNonTrivial>(ent);
			REQUIRE(p.x == 111.f);
			REQUIRE(p.y == 222.f);
			REQUIRE(p.z == 333.f);
		}
	}

	// Add one more component and check if the values are still fine after creating a new archetype
	{
		auto ent = w.copy(arr[0]);
		w.add<Position>(ent, {5, 6, 7});

		const auto& s1 = w.get<StringComponent>(ent);
		REQUIRE(s1.value == StringComponentDefaultValue);

		const auto& s2 = w.get<StringComponent2>(ent);
		REQUIRE(s2.value == StringComponent2DefaultValue_2);

		const auto& p = w.get<PositionNonTrivial>(ent);
		REQUIRE(p.x == 111.f);
		REQUIRE(p.y == 222.f);
		REQUIRE(p.z == 333.f);
	}
}

TEST_CASE("CommandBuffer") {
	SECTION("Entity creation") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		const uint32_t N = 100;
		GAIA_FOR(N) {
			[[maybe_unused]] auto tmp = cb.add();
		}

		cb.commit();

		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1 + N);
	}

	SECTION("Entity creation from another entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto mainEntity = w.add();

		const uint32_t N = 100;
		GAIA_FOR(N) {
			[[maybe_unused]] auto tmp = cb.copy(mainEntity);
		}

		cb.commit();

		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1 + 1 + N); // core + mainEntity + N others
	}

	SECTION("Entity creation from another entity with a component") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto mainEntity = w.add();
		w.add<Position>(mainEntity, {1, 2, 3});

		[[maybe_unused]] auto tmp = cb.copy(mainEntity);
		cb.commit();

		auto q = w.query().all<Position>();
		REQUIRE(q.count() == 2);
		uint32_t i = 0;
		q.each([&](const Position& p) {
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);
			++i;
		});
		REQUIRE(i == 2);
	}

	SECTION("Delayed component addition to an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();
		cb.add<Position>(e);
		REQUIRE_FALSE(w.has<Position>(e));
		cb.commit();
		REQUIRE(w.has<Position>(e));
	}

	SECTION("Delayed component addition to a to-be-created entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto tmp = cb.add(); // core + 0 (no new entity created yet)
		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1);
		cb.add<Position>(tmp);
		cb.commit();

		auto e = w.get(ecs::GAIA_ID(LastCoreComponent).id() + 1 + 1); // core + position + new entity
		REQUIRE(w.has<Position>(e));
	}

	SECTION("Delayed component setting of an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();

		cb.add<Position>(e);
		cb.set<Position>(e, {1, 2, 3});
		REQUIRE_FALSE(w.has<Position>(e));

		cb.commit();
		REQUIRE(w.has<Position>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1);
		REQUIRE(p.y == 2);
		REQUIRE(p.z == 3);
	}

	SECTION("Delayed 2 components setting of an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();

		cb.add<Position>(e);
		cb.set<Position>(e, {1, 2, 3});
		cb.add<Acceleration>(e);
		cb.set<Acceleration>(e, {4, 5, 6});
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));

		cb.commit();
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);

		auto a = w.get<Acceleration>(e);
		REQUIRE(a.x == 4.f);
		REQUIRE(a.y == 5.f);
		REQUIRE(a.z == 6.f);
	}

	SECTION("Delayed component setting of a to-be-created entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto tmp = cb.add();
		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1); // core + 0 (no new entity created yet)

		cb.add<Position>(tmp);
		cb.set<Position>(tmp, {1, 2, 3});
		cb.commit();

		auto e = w.get(ecs::GAIA_ID(LastCoreComponent).id() + 1 + 1); // core + position + new entity
		REQUIRE(w.has<Position>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);
	}

	SECTION("Delayed 2 components setting of a to-be-created entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto tmp = cb.add();
		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1); // core + 0 (no new entity created yet)

		cb.add<Position>(tmp);
		cb.add<Acceleration>(tmp);
		cb.set<Position>(tmp, {1, 2, 3});
		cb.set<Acceleration>(tmp, {4, 5, 6});
		cb.commit();

		auto e = w.get(ecs::GAIA_ID(LastCoreComponent).id() + 1 + 2); // core + 2 new components + new entity
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);

		auto a = w.get<Acceleration>(e);
		REQUIRE(a.x == 4.f);
		REQUIRE(a.y == 5.f);
		REQUIRE(a.z == 6.f);
	}

	SECTION("Delayed component add with setting of a to-be-created entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto tmp = cb.add();
		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1); // core + 0 (no new entity created yet)

		cb.add<Position>(tmp, {1, 2, 3});
		cb.commit();

		auto e = w.get(ecs::GAIA_ID(LastCoreComponent).id() + 1 + 1); // core + position + new entity
		REQUIRE(w.has<Position>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);
	}

	SECTION("Delayed 2 components add with setting of a to-be-created entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto tmp = cb.add();
		REQUIRE(w.size() == ecs::GAIA_ID(LastCoreComponent).id() + 1); // core + 0 (no new entity created yet)

		cb.add<Position>(tmp, {1, 2, 3});
		cb.add<Acceleration>(tmp, {4, 5, 6});
		cb.commit();

		auto e = w.get(ecs::GAIA_ID(LastCoreComponent).id() + 1 + 2); // core + 2 new components + new entity
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));

		auto p = w.get<Position>(e);
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 3.f);

		auto a = w.get<Acceleration>(e);
		REQUIRE(a.x == 4.f);
		REQUIRE(a.y == 5.f);
		REQUIRE(a.z == 6.f);
	}

	SECTION("Delayed component removal from an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();
		w.add<Position>(e, {1, 2, 3});

		cb.del<Position>(e);
		REQUIRE(w.has<Position>(e));
		{
			auto p = w.get<Position>(e);
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);
		}

		cb.commit();
		REQUIRE_FALSE(w.has<Position>(e));
	}

	SECTION("Delayed 2 component removal from an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();
		w.add<Position>(e, {1, 2, 3});
		w.add<Acceleration>(e, {4, 5, 6});

		cb.del<Position>(e);
		cb.del<Acceleration>(e);
		REQUIRE(w.has<Position>(e));
		REQUIRE(w.has<Acceleration>(e));
		{
			auto p = w.get<Position>(e);
			REQUIRE(p.x == 1.f);
			REQUIRE(p.y == 2.f);
			REQUIRE(p.z == 3.f);

			auto a = w.get<Acceleration>(e);
			REQUIRE(a.x == 4.f);
			REQUIRE(a.y == 5.f);
			REQUIRE(a.z == 6.f);
		}

		cb.commit();
		REQUIRE_FALSE(w.has<Position>(e));
		REQUIRE_FALSE(w.has<Acceleration>(e));
	}

	SECTION("Delayed non-trivial component setting of an existing entity") {
		ecs::World w;
		ecs::CommandBuffer cb(w);

		auto e = w.add();

		cb.add<StringComponent>(e);
		cb.set<StringComponent>(e, {StringComponentDefaultValue});
		cb.add<StringComponent2>(e);
		REQUIRE_FALSE(w.has<StringComponent>(e));
		REQUIRE_FALSE(w.has<StringComponent2>(e));

		cb.commit();
		REQUIRE(w.has<StringComponent>(e));
		REQUIRE(w.has<StringComponent2>(e));

		auto s1 = w.get<StringComponent>(e);
		REQUIRE(s1.value == StringComponentDefaultValue);
		auto s2 = w.get<StringComponent2>(e);
		REQUIRE(s2.value == StringComponent2DefaultValue);
	}
}

TEST_CASE("Query Filter - no systems") {
	ecs::World w;
	ecs::Query q = w.query().all<Position>().changed<Position>();

	auto e = w.add();
	w.add<Position>(e);

	// System-less filters
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 1); // first run always happens
	}
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 0); // no change of position so this shouldn't run
	}
	{ w.set<Position>(e, {}); }
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 1);
	}
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{ w.sset<Position>(e, {}); }
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
	{
		auto* ch = w.get_chunk(e);
		auto p = ch->sview_mut<Position>();
		p[0] = {};
	}
	{
		uint32_t cnt = 0;
		q.each([&]([[maybe_unused]] const Position& a) {
			++cnt;
		});
		REQUIRE(cnt == 0);
	}
}

TEST_CASE("Query Filter - systems") {
	ecs::World w;

	auto e = w.add();
	w.add<Position>(e);

	class WriterSystem final: public ecs::System {
		ecs::Query m_q;

	public:
		void OnCreated() override {
			m_q = world().query().all<Position&>();
		}
		void OnUpdate() override {
			m_q.each([]([[maybe_unused]] Position&) {});
		}
	};
	class WriterSystemSilent final: public ecs::System {
		ecs::Query m_q;

	public:
		void OnCreated() override {
			m_q = world().query().all<Position&>();
		}
		void OnUpdate() override {
			m_q.each([&](ecs::Iter iter) {
				auto posRWView = iter.sview_mut<Position>();
				(void)posRWView;
			});
		}
	};
	class ReaderSystem final: public ecs::System {
		uint32_t m_expectedCnt = 0;

		ecs::Query m_q;

	public:
		void SetExpectedCount(uint32_t cnt) {
			m_expectedCnt = cnt;
		}
		void OnCreated() override {
			m_q = world().query().all<Position>().changed<Position>();
		}
		void OnUpdate() override {
			uint32_t cnt = 0;
			m_q.each([&]([[maybe_unused]] const Position& a) {
				++cnt;
			});
			REQUIRE(cnt == m_expectedCnt);
		}
	};
	ecs::SystemManager sm(w);
	auto* ws = sm.add<WriterSystem>();
	auto* wss = sm.add<WriterSystemSilent>();
	auto* rs = sm.add<ReaderSystem>();

	// first run always happens
	ws->enable(false);
	wss->enable(false);
	rs->SetExpectedCount(1);
	sm.update();
	// no change of position so ReaderSystem should't see changes
	rs->SetExpectedCount(0);
	sm.update();
	// update position so ReaderSystem should detect a change
	ws->enable(true);
	rs->SetExpectedCount(1);
	sm.update();
	// no change of position so ReaderSystem shouldn't see changes
	ws->enable(false);
	rs->SetExpectedCount(0);
	sm.update();
	// silent writer enabled again. If should not cause an update
	ws->enable(false);
	wss->enable(true);
	rs->SetExpectedCount(0);
	sm.update();
}

template <typename T>
void TestDataLayoutSoA_ECS() {
	const uint32_t N = 1'500;

	ecs::World w;
	cnt::darr<ecs::Entity> ents;
	ents.reserve(N);

	auto create = [&]() {
		auto e = w.add();
		w.add<T>(e, {});
		ents.push_back(e);
	};

	GAIA_FOR(N) create();

	SECTION("each - iterator") {
		ecs::Query q = w.query().all<T&>();

		{
			const auto cnt = q.count();
			REQUIRE(cnt == N);

			uint32_t j = 0;
			q.each([&](ecs::Iter iter) {
				auto t = iter.view_mut<T>();
				auto tx = t.template set<0>();
				auto ty = t.template set<1>();
				auto tz = t.template set<2>();
				GAIA_EACH(iter) {
					auto f = (float)j;
					tx[i] = f;
					ty[i] = f;
					tz[i] = f;

					REQUIRE(tx[i] == f);
					REQUIRE(ty[i] == f);
					REQUIRE(tz[i] == f);

					++j;
				}
			});
			REQUIRE(j == cnt);
		}

		// Make sure disabling works
		{
			auto e = ents[0];
			w.enable(e, false);
			const auto cnt = q.count();
			REQUIRE(cnt == N - 1);
			uint32_t cntCalculated = 0;
			q.each([&](ecs::Iter iter) {
				cntCalculated += iter.size();
			});
			REQUIRE(cnt == cntCalculated);
		}
	}

	// TODO: Finish this part
	// SECTION("each") {
	// 	ecs::Query q = w.query().all<T>();

	// 	{
	// 		const auto cnt = q.count();
	// 		REQUIRE(cnt == N);

	// 		uint32_t j = 0;
	// 		// TODO: Add SoA support for q.each([](T& t){})
	// 		q.each([&](const T& t) {
	// 			auto f = (float)j;
	// 			REQUIRE(t.x == f);
	// 			REQUIRE(t.y == f);
	// 			REQUIRE(t.z == f);

	// 			++j;
	// 		});
	// 		REQUIRE(j == cnt);
	// 	}

	// 	// Make sure disabling works
	// 	{
	// 		auto e = ents[0];
	// 		w.enable(e, false);
	// 		const auto cnt = q.count();
	// 		REQUIRE(cnt == N - 1);
	// 		uint32_t cntCalculated = 0;
	// 		q.each([&]([[maybe_unused]] const T& t) {
	// 			++cntCalculated;
	// 		});
	// 		REQUIRE(cnt == cntCalculated);
	// 	}
	// }
}

TEST_CASE("DataLayout SoA - ECS") {
	TestDataLayoutSoA_ECS<PositionSoA>();
	TestDataLayoutSoA_ECS<RotationSoA>();
}

TEST_CASE("DataLayout SoA8 - ECS") {
	TestDataLayoutSoA_ECS<PositionSoA8>();
	TestDataLayoutSoA_ECS<RotationSoA8>();
}

TEST_CASE("DataLayout SoA16 - ECS") {
	TestDataLayoutSoA_ECS<PositionSoA16>();
	TestDataLayoutSoA_ECS<RotationSoA16>();
}

//------------------------------------------------------------------------------
// Component cache
//------------------------------------------------------------------------------

TEST_CASE("Component cache") {
	ecs::World w;
	auto& cc = w.comp_cache_mut();
	{
		cc.clear();
		const auto& desc = w.add<Position>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<const Position>().comp);
		REQUIRE(comp == w.add<Position&>().comp);
		REQUIRE(comp == w.add<const Position&>().comp);
		REQUIRE(comp == w.add<Position*>().comp);
		REQUIRE(comp == w.add<const Position*>().comp);
	}
	{
		cc.clear();
		const auto& desc = w.add<const Position>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<Position>().comp);
		REQUIRE(comp == w.add<Position&>().comp);
		REQUIRE(comp == w.add<const Position&>().comp);
		REQUIRE(comp == w.add<Position*>().comp);
		REQUIRE(comp == w.add<const Position*>().comp);
	}
	{
		cc.clear();
		const auto& desc = w.add<Position&>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<Position>().comp);
		REQUIRE(comp == w.add<const Position>().comp);
		REQUIRE(comp == w.add<const Position&>().comp);
		REQUIRE(comp == w.add<Position*>().comp);
		REQUIRE(comp == w.add<const Position*>().comp);
	}
	{
		cc.clear();
		const auto& desc = w.add<const Position&>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<Position>().comp);
		REQUIRE(comp == w.add<const Position>().comp);
		REQUIRE(comp == w.add<Position&>().comp);
		REQUIRE(comp == w.add<Position*>().comp);
		REQUIRE(comp == w.add<const Position*>().comp);
	}
	{
		cc.clear();
		const auto& desc = w.add<Position*>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<Position>().comp);
		REQUIRE(comp == w.add<const Position>().comp);
		REQUIRE(comp == w.add<Position&>().comp);
		REQUIRE(comp == w.add<const Position&>().comp);
		REQUIRE(comp == w.add<const Position*>().comp);
	}
	{
		cc.clear();
		const auto& desc = w.add<const Position*>();
		auto ent = desc.entity;
		REQUIRE_FALSE(ent.entity());
		auto comp = desc.comp;
		REQUIRE(comp == w.add<Position>().comp);
		REQUIRE(comp == w.add<const Position>().comp);
		REQUIRE(comp == w.add<Position&>().comp);
		REQUIRE(comp == w.add<const Position&>().comp);
		REQUIRE(comp == w.add<Position*>().comp);
	}
}

//------------------------------------------------------------------------------
// Multiple worlds
//------------------------------------------------------------------------------

TEST_CASE("Multiple worlds") {
	ecs::World w1, w2;
	{
		auto e = w1.add();
		w1.add<Position>(e, {1, 1, 1});
		(void)w1.copy(e);
		(void)w1.copy(e);
	}
	{
		auto e = w2.add();
		w2.add<Scale>(e, {2, 0, 0});
		w2.add<Position>(e, {2, 2, 2});
	}

	uint32_t c = 0;
	auto q1 = w1.query().all<Position>();
	q1.each([&c](const Position& p) {
		REQUIRE(p.x == 1.f);
		REQUIRE(p.y == 1.f);
		REQUIRE(p.z == 1.f);
		++c;
	});
	REQUIRE(c == 3);

	c = 0;
	auto q2 = w2.query().all<Position>();
	q2.each([&c](const Position& p) {
		REQUIRE(p.x == 2.f);
		REQUIRE(p.y == 2.f);
		REQUIRE(p.z == 2.f);
		++c;
	});
	REQUIRE(c == 1);
}

//------------------------------------------------------------------------------
// Serialization
//------------------------------------------------------------------------------

template <typename T>
bool CompareSerializableType(const T& a, const T& b) {
	if constexpr (std::is_trivially_copyable_v<T>)
		return !std::memcmp((const void*)&a, (const void*)&b, sizeof(b));
	else
		return a == b;
}

template <typename T>
bool CompareSerializableTypes(const T& a, const T& b) {
	// Convert inputs into tuples where each struct member is an element of the tuple
	auto ta = meta::struct_to_tuple(a);
	auto tb = meta::struct_to_tuple(b);

	// Do element-wise comparison
	bool ret = true;
	core::each<std::tuple_size<decltype(ta)>::value>([&](auto i) {
		const auto& aa = std::get<i>(ta);
		const auto& bb = std::get<i>(ta);
		ret = ret && CompareSerializableType(aa, bb);
	});
	return ret;
}

struct FooNonTrivial {
	uint32_t a = 0;

	explicit FooNonTrivial(uint32_t value): a(value){};
	FooNonTrivial() noexcept = default;
	~FooNonTrivial() = default;
	FooNonTrivial(const FooNonTrivial&) = default;
	FooNonTrivial(FooNonTrivial&&) noexcept = default;
	FooNonTrivial& operator=(const FooNonTrivial&) = default;
	FooNonTrivial& operator=(FooNonTrivial&&) noexcept = default;

	bool operator==(const FooNonTrivial& other) const {
		return a == other.a;
	}
};

struct SerializeStruct1 {
	int a1;
	int a2;
	bool b;
	float c;
};

struct SerializeStruct2 {
	FooNonTrivial d;
	int a1;
	int a2;
	bool b;
	float c;
};

struct CustomStruct {
	char* ptr;
	uint32_t size;
};

namespace gaia::ser {
	template <>
	uint32_t bytes(const CustomStruct& data) {
		return data.size + sizeof(data.size);
	}
	template <typename Serializer>
	void save(Serializer& s, const CustomStruct& data) {
		s.save(data.size);
		s.save(data.ptr, data.size);
	}

	template <typename Serializer>
	void load(Serializer& s, CustomStruct& data) {
		s.load(data.size);
		data.ptr = new char[data.size];
		s.load(data.ptr, data.size);
	}
} // namespace gaia::ser

struct CustomStructInternal {
	char* ptr;
	uint32_t size;

	bool operator==(const CustomStructInternal& other) const {
		return ptr == other.ptr && size == other.size;
	}

	constexpr uint32_t bytes() const noexcept {
		return size + sizeof(size);
	}

	template <typename Serializer>
	void save(Serializer& s) const {
		s.save(size);
		s.save(ptr, size);
	}

	template <typename Serializer>
	void load(Serializer& s) {
		s.load(size);
		ptr = new char[size];
		s.load(ptr, size);
	}
};

TEST_CASE("Serialization - custom") {
	SECTION("external") {
		CustomStruct in, out;
		in.ptr = new char[5];
		in.ptr[0] = 'g';
		in.ptr[1] = 'a';
		in.ptr[2] = 'i';
		in.ptr[3] = 'a';
		in.ptr[4] = '\0';
		in.size = 5;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
		delete in.ptr;
		delete out.ptr;
	}
	SECTION("internal") {
		CustomStructInternal in, out;
		in.ptr = new char[5];
		in.ptr[0] = 'g';
		in.ptr[1] = 'a';
		in.ptr[2] = 'i';
		in.ptr[3] = 'a';
		in.ptr[4] = '\0';
		in.size = 5;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
		delete in.ptr;
		delete out.ptr;
	}
}

TEST_CASE("Serialization - simple") {
	{
		Int3 in{1, 2, 3}, out{};

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		Position in{1, 2, 3}, out{};

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		SerializeStruct1 in{1, 2, true, 3.12345f}, out{};

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		SerializeStruct2 in{FooNonTrivial(111), 1, 2, true, 3.12345f}, out{};

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}
}

struct SerializeStructSArray {
	cnt::sarray<uint32_t, 8> arr;
	float f;
};

struct SerializeStructSArrayNonTrivial {
	cnt::sarray<FooNonTrivial, 8> arr;
	float f;

	bool operator==(const SerializeStructSArrayNonTrivial& other) const {
		return f == other.f && arr == other.arr;
	}
};

struct SerializeStructDArray {
	cnt::darr<uint32_t> arr;
	float f;
};

struct SerializeStructDArrayNonTrivial {
	cnt::darr<uint32_t> arr;
	float f;

	bool operator==(const SerializeStructDArrayNonTrivial& other) const {
		return f == other.f && arr == other.arr;
	}
};

struct SerializeCustomStructInternalDArray {
	cnt::darr<CustomStructInternal> arr;
	float f;

	bool operator==(const SerializeCustomStructInternalDArray& other) const {
		return f == other.f && arr == other.arr;
	}
};

TEST_CASE("Serialization - arrays") {
	{
		SerializeStructSArray in{}, out{};
		GAIA_EACH(in.arr) in.arr[i] = i;
		in.f = 3.12345f;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		SerializeStructSArrayNonTrivial in{}, out{};
		GAIA_EACH(in.arr) in.arr[i] = FooNonTrivial(i);
		in.f = 3.12345f;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		SerializeStructDArray in{}, out{};
		in.arr.resize(10);
		GAIA_EACH(in.arr) in.arr[i] = i;
		in.f = 3.12345f;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));
	}

	{
		SerializeCustomStructInternalDArray in{}, out{};
		in.arr.resize(10);
		for (auto& a: in.arr) {
			a.ptr = new char[5];
			a.ptr[0] = 'g';
			a.ptr[1] = 'a';
			a.ptr[2] = 'i';
			a.ptr[3] = 'a';
			a.ptr[4] = '\0';
			a.size = 5;
		}
		in.f = 3.12345f;

		ecs::SerializationBuffer s;
		s.reserve(ser::bytes(in));

		ser::save(s, in);
		s.seek(0);
		ser::load(s, out);

		REQUIRE(CompareSerializableTypes(in, out));

		for (auto& a: in.arr)
			delete a.ptr;
		for (auto& a: out.arr)
			delete a.ptr;
	}
}

//------------------------------------------------------------------------------
// Mutlithreading
//------------------------------------------------------------------------------

static uint32_t JobSystemFunc(std::span<const uint32_t> arr) {
	uint32_t sum = 0;
	GAIA_EACH(arr) sum += arr[i];
	return sum;
}

template <typename Func>
void Run_Schedule_Simple(const uint32_t* pArr, uint32_t* pRes, uint32_t Jobs, uint32_t ItemsPerJob, Func func) {
	auto& tp = mt::ThreadPool::get();

	std::atomic_uint32_t sum = 0;

	GAIA_FOR(Jobs) {
		mt::Job job;
		job.func = [&pArr, &pRes, i, ItemsPerJob, func]() {
			const auto idxStart = i * ItemsPerJob;
			const auto idxEnd = (i + 1) * ItemsPerJob;
			pRes[i] += func({pArr + idxStart, idxEnd - idxStart});
		};
		tp.sched(job);
	}
	tp.wait_all();

	GAIA_FOR(Jobs) REQUIRE(pRes[i] == ItemsPerJob);
}

TEST_CASE("Multithreading - Schedule") {
	constexpr uint32_t JobCount = 64;
	constexpr uint32_t ItemsPerJob = 5000;
	constexpr uint32_t N = JobCount * ItemsPerJob;

	cnt::sarray<uint32_t, JobCount> res;
	GAIA_FOR(res.max_size()) res[i] = 0;

	cnt::darr<uint32_t> arr;
	arr.resize(N);
	GAIA_EACH(arr) arr[i] = 1;

	Run_Schedule_Simple(arr.data(), res.data(), JobCount, ItemsPerJob, JobSystemFunc);
}

TEST_CASE("Multithreading - ScheduleParallel") {
	auto& tp = mt::ThreadPool::get();

	constexpr uint32_t JobCount = 64;
	constexpr uint32_t ItemsPerJob = 5000;
	constexpr uint32_t N = JobCount * ItemsPerJob;

	cnt::darr<uint32_t> arr;
	arr.resize(N);
	GAIA_EACH(arr) arr[i] = 1;

	std::atomic_uint32_t sum1 = 0;
	mt::JobParallel j1;
	j1.func = [&arr, &sum1](const mt::JobArgs& args) {
		sum1 += JobSystemFunc({arr.data() + args.idxStart, args.idxEnd - args.idxStart});
	};

	auto jobHandle = tp.sched_par(j1, N, ItemsPerJob);
	tp.wait(jobHandle);

	REQUIRE(sum1 == N);

	tp.wait_all();
}

TEST_CASE("Multithreading - complete") {
	auto& tp = mt::ThreadPool::get();

	constexpr uint32_t Jobs = 15000;

	cnt::darr<mt::JobHandle> handles;
	cnt::darr<uint32_t> res;

	handles.resize(Jobs);
	res.resize(Jobs);

	GAIA_EACH(res) res[i] = (uint32_t)-1;

	GAIA_FOR(Jobs) {
		mt::Job job;
		job.func = [&res, i]() {
			res[i] = i;
		};
		handles[i] = tp.sched(job);
	}

	GAIA_FOR(Jobs) {
		tp.wait(handles[i]);
		REQUIRE(res[i] == i);
	}
}

TEST_CASE("Multithreading - CompleteMany") {
	auto& tp = mt::ThreadPool::get();

	srand(0);

	constexpr uint32_t Iters = 15000;
	uint32_t res = (uint32_t)-1;

	GAIA_FOR(Iters) {
		mt::Job job0{[&res, i]() {
			res = (i + 1);
		}};
		mt::Job job1{[&res, i]() {
			res *= (i + 1);
		}};
		mt::Job job2{[&res, i]() {
			res /= (i + 1); // we add +1 everywhere to avoid division by zero at i==0
		}};

		const mt::JobHandle jobHandle[] = {tp.add(job0), tp.add(job1), tp.add(job2)};

		tp.dep(jobHandle[1], jobHandle[0]);
		tp.dep(jobHandle[2], jobHandle[1]);

		// 2, 0, 1 -> wrong sum
		// Submit jobs in random order to make sure this doesn't work just by accident
		const uint32_t startIdx0 = rand() % 3;
		const uint32_t startIdx1 = (startIdx0 + 1) % 3;
		const uint32_t startIdx2 = (startIdx0 + 2) % 3;
		tp.submit(jobHandle[startIdx0]);
		tp.submit(jobHandle[startIdx1]);
		tp.submit(jobHandle[startIdx2]);

		tp.wait(jobHandle[2]);

		REQUIRE(res == (i + 1));
	}
}

//------------------------------------------------------------------------------
