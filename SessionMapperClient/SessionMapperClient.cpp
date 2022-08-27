// SessionMapperClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include"Includes.h"
#include"Mapper.h"

int main(int agrc,char* agrv[])
{
	if (agrc != 2)
	{
		fmt::print("[Usage] {} driver.sys\n", agrv[0]);
		system("pause");
		return 0;
	}

	if (!std::filesystem::exists(agrv[1]))
	{
		fmt::print("File {} does not exist\n", agrv[1]);
		system("pause");
		return 0;
	}

	auto [status, service_name] = driver::load(MapperDriver, sizeof(MapperDriver));

	if (!status)
	{
		fmt::print(fmt::fg(fmt::color::red),"Failed to load signed driver\n");
		system("pause");
		return 0;
	}
	fmt::print(fmt::bg(fmt::color::gray), "Loaded signed mapper\n");

	fmt::print(fmt::bg(fmt::color::gray), "Service Name : {}\n", service_name);

	std::vector<uint8_t> DriverImage = { 0 };
	if (!Mapper::readFile(agrv[1], &DriverImage))
	{
		fmt::print(fmt::fg(fmt::color::red), "Failed to read file {}\n", agrv[1]);
		fmt::print(fmt::bg(fmt::color::gray), "Unloading signed driver\n");
		fmt::print(fmt::bg(fmt::color::gray), "Status : {}\n", !driver::unload(service_name));
		system("pause");
		return 0;
	}
	
	MapperStruct MapperData;
	MapperData.rawDataAddress = reinterpret_cast<std::uintptr_t>(DriverImage.data());
	MapperData.rawDataSize = DriverImage.size();
	const auto Mapperstatus =  Mapper::mapDriver(&MapperData);

	fmt::print(fmt::bg(fmt::color::gray),"Mapped driver status {}\n", Mapperstatus);
	fmt::print(fmt::bg(fmt::color::gray), "Unloading signed driver\n");
	fmt::print(fmt::bg(fmt::color::gray), "Status : {}\n", driver::unload(service_name));	system("pause");
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
