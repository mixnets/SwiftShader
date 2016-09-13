#include "src/IceCfg.h"
#include "src/IceELFStreamer.h"
#include "src/IceGlobalContext.h"
#include "src/IceCfgNode.h"
#include "src/IceTargetLoweringX8664.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_os_ostream.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include <iostream>
#include <vector>

class ELFMemoryStreamer : public Ice::ELFStreamer
{
	ELFMemoryStreamer(const ELFMemoryStreamer &) = delete;
	ELFMemoryStreamer &operator=(const ELFMemoryStreamer &) = delete;

public:
	ELFMemoryStreamer() = default;

	void write8(uint8_t Value) override
	{
		if(position == (uint64_t)buffer.size())
		{
			buffer.push_back(Value);
			position++;
		}
		else if(position < (uint64_t)buffer.size())
		{
			buffer[position] = Value;
			position++;
		}
		else assert(false);
	}

	void writeBytes(llvm::StringRef Bytes) override
	{
		for(char c : Bytes)
		{
			write8(c);
		}
	}

	uint64_t tell() const override { return position; }

	void seek(uint64_t Off) override { position = Off; }

	void *getBuffer() const { return (void*)&buffer[0]; }
	uint64_t getSize() const { return (uint64_t)buffer.size(); }

private:
	std::vector<uint8_t> buffer;
	uint64_t position;
};

void *loadImage32(char *const elfImage, unsigned int size)
{
	Elf32_Ehdr *elfHeader = (Elf32_Ehdr*)elfImage;

	if(!elfHeader->checkMagic())
	{
		return nullptr;
	}

	Elf32_Shdr *sectionHeader = (Elf32_Shdr*)(elfImage + elfHeader->e_shoff);
	void *entry = nullptr;

	for(int i = 0; i < elfHeader->e_shnum; i++)
	{
		if(sectionHeader[i].sh_type == SHT_PROGBITS && sectionHeader[i].sh_flags & SHF_EXECINSTR)
		{
			entry = elfImage + sectionHeader[i].sh_offset;

			DWORD oldProtection;
			VirtualProtect(entry, sectionHeader[i].sh_size, PAGE_EXECUTE_READ, &oldProtection);
		}
	}

	return entry;
}

void *loadImage64(char *const elfImage, unsigned int size)
{
	Elf64_Ehdr *elfHeader = (Elf64_Ehdr*)elfImage;

	if(!elfHeader->checkMagic())
	{
		return nullptr;
	}

	Elf64_Shdr *sectionHeader = (Elf64_Shdr*)(elfImage + elfHeader->e_shoff);
	void *entry = nullptr;

	for(int i = 0; i < elfHeader->e_shnum; i++)
	{
		if(sectionHeader[i].sh_type == SHT_PROGBITS && sectionHeader[i].sh_flags & SHF_EXECINSTR)
		{
			entry = elfImage + sectionHeader[i].sh_offset;

			DWORD oldProtection;
			VirtualProtect(entry, sectionHeader[i].sh_size, PAGE_EXECUTE_READ, &oldProtection);
		}
	}

	return entry;
}

#if !defined(__x86_64__) && (defined(_M_AMD64) || defined (_M_X64))
#define __x86_64__ 1
#endif

#if !defined(__x386__) && defined(_M_IX86)
#define __i386__ 1
#endif

int main()
{
	#if defined(__i386__)
		Ice::ClFlags::Flags.setTargetArch(Ice::Target_X8632);
	#elif defined(__x86_64__)
		Ice::ClFlags::Flags.setTargetArch(Ice::Target_X8664);
	#else
		#error ISA unsupported
	#endif
	
	Ice::ClFlags::Flags.setOutFileType(Ice::FT_Elf);
	Ice::ClFlags::Flags.setOptLevel(Ice::Opt_2);

	std::unique_ptr<Ice::Ostream> cout(new llvm::raw_os_ostream(std::cout));
	std::error_code errorCode;
	std::unique_ptr<Ice::Fdstream> out(new Ice::Fdstream("out.o", errorCode, llvm::sys::fs::F_None));
	std::unique_ptr<Ice::ELFStreamer> elf(new Ice::ELFFileStreamer(*out.get()));
	//std::unique_ptr<ELFMemoryStreamer> elf(new ELFMemoryStreamer());
	Ice::GlobalContext context(cout.get(), cout.get(), cout.get(), elf.get());

	std::unique_ptr<Ice::Cfg> function(Ice::Cfg::create(&context, 0));
	{
		Ice::CfgLocalAllocatorScope _(function.get());

		function->setFunctionName(Ice::GlobalString::createWithString(&context, "HelloWorld"));
		Ice::Variable *arg1 = function->makeVariable(Ice::IceType_i32);
		Ice::Variable *arg2 = function->makeVariable(Ice::IceType_i32);
		function->addArg(arg1);
		function->addArg(arg2);
		function->setReturnType(Ice::IceType_i32);

		Ice::CfgNode *node = function->makeNode();
		function->setEntryNode(node);

		Ice::Variable *sum = function->makeVariable(Ice::IceType_i32);
		Ice::InstArithmetic *add = Ice::InstArithmetic::create(function.get(), Ice::InstArithmetic::Add, sum, arg1, arg2);
		node->appendInst(add);

		Ice::InstRet *ret = Ice::InstRet::create(function.get(), sum);
		node->appendInst(ret);

		function->translate();

		context.emitFileHeader();
		function->emitIAS();
		auto assembler = function->releaseAssembler();
		context.getObjectWriter()->writeFunctionCode(function->getFunctionName(), false, assembler.get());
		context.getObjectWriter()->writeNonUserSections();
	}

	//void *buffer = elf->getBuffer();
	//int (*add)(int, int) = (int(*)(int,int))loadImage64((char*)buffer, elf->getSize());

	//int x = add(1, 2);

	out->close();

	return 0;
}
